#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include "RotaryEncoder.h"
#include "PlantLogic.h"
#include "pins.h"

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
RotaryEncoder encoder;
Preferences prefs;
PlantController plants[NUM_PLANTS];

static const uint16_t SENSOR_WARMUP_MS = 500;
static const uint8_t  SENSOR_SAMPLES = 15;       // median of N reads per measurement
static const uint32_t MAX_PUMP_RUN_MS = 120000UL; // hard cap on any single pump run
static const uint32_t PUMP_CAL_MS = 30000UL;      // pump calibration run length
static const uint32_t DISPLAY_SLEEP_MS = 120000UL;

// ---------------------------------------------------------------------------
// Button debounce
// ---------------------------------------------------------------------------
static const uint32_t DEBOUNCE_MS = 50;

struct Button {
    uint8_t pin;
    bool lastRaw;
    bool stableState;
    uint32_t lastChangeMs;
    bool pressed; // single-shot flag; caller clears after handling
};

void initButton(Button &btn, uint8_t pin) {
    btn.pin = pin;
    btn.lastRaw = HIGH;
    btn.stableState = HIGH;
    btn.lastChangeMs = 0;
    btn.pressed = false;
    pinMode(pin, INPUT_PULLUP);
}

void updateButton(Button &btn) {
    bool raw = digitalRead(btn.pin);
    if (raw != btn.lastRaw) {
        btn.lastChangeMs = millis();
        btn.lastRaw = raw;
    }
    if ((millis() - btn.lastChangeMs) >= DEBOUNCE_MS) {
        if (btn.stableState == HIGH && raw == LOW) btn.pressed = true;
        btn.stableState = raw;
    }
}

Button btnPsh, btnBak, btnCon;

// ---------------------------------------------------------------------------
// Persistent config (NVS via Preferences)
// ---------------------------------------------------------------------------
static void keyFor(char *out, size_t n, uint8_t plant, const char *field) {
    snprintf(out, n, "p%u_%s", (unsigned)plant, field);
}

static void sanitize(PlantConfig &c) {
    c.targetPct = constrain(c.targetPct, 1, 100);
    if (c.minPct >= c.targetPct) c.minPct = c.targetPct - 1;
    c.waterMl = constrain(c.waterMl, 1, 500);
    c.soakMin = constrain(c.soakMin, 1, 30);
    c.checkHours = constrain(c.checkHours, 1, 24);
    c.pumpMlPerSec = constrain(c.pumpMlPerSec, 0.1f, 20.0f);
    if (c.sensorDry == c.sensorWet) { c.sensorDry = 3100; c.sensorWet = 1500; }
}

void loadConfig() {
    prefs.begin("plants", /*readOnly=*/false); // rw so the namespace is created on first boot
    char k[16];
    for (uint8_t i = 0; i < NUM_PLANTS; i++) {
        PlantConfig &c = plants[i].cfg;
        keyFor(k, sizeof(k), i, "en");   c.enabled = prefs.getBool(k, false); // new plants start disabled
        keyFor(k, sizeof(k), i, "tgt");  c.targetPct = prefs.getUChar(k, c.targetPct);
        keyFor(k, sizeof(k), i, "min");  c.minPct = prefs.getUChar(k, c.minPct);
        keyFor(k, sizeof(k), i, "ml");   c.waterMl = prefs.getUShort(k, c.waterMl);
        keyFor(k, sizeof(k), i, "soak"); c.soakMin = prefs.getUChar(k, c.soakMin);
        keyFor(k, sizeof(k), i, "chk");  c.checkHours = prefs.getUChar(k, c.checkHours);
        keyFor(k, sizeof(k), i, "dry");  c.sensorDry = prefs.getUShort(k, c.sensorDry);
        keyFor(k, sizeof(k), i, "wet");  c.sensorWet = prefs.getUShort(k, c.sensorWet);
        keyFor(k, sizeof(k), i, "rate"); c.pumpMlPerSec = prefs.getFloat(k, c.pumpMlPerSec);
        sanitize(c);
    }
    prefs.end();
}

void saveConfig(uint8_t i) {
    PlantConfig &c = plants[i].cfg;
    sanitize(c);
    prefs.begin("plants", false);
    char k[16];
    keyFor(k, sizeof(k), i, "en");   prefs.putBool(k, c.enabled);
    keyFor(k, sizeof(k), i, "tgt");  prefs.putUChar(k, c.targetPct);
    keyFor(k, sizeof(k), i, "min");  prefs.putUChar(k, c.minPct);
    keyFor(k, sizeof(k), i, "ml");   prefs.putUShort(k, c.waterMl);
    keyFor(k, sizeof(k), i, "soak"); prefs.putUChar(k, c.soakMin);
    keyFor(k, sizeof(k), i, "chk");  prefs.putUChar(k, c.checkHours);
    keyFor(k, sizeof(k), i, "dry");  prefs.putUShort(k, c.sensorDry);
    keyFor(k, sizeof(k), i, "wet");  prefs.putUShort(k, c.sensorWet);
    keyFor(k, sizeof(k), i, "rate"); prefs.putFloat(k, c.pumpMlPerSec);
    prefs.end();
}

// ---------------------------------------------------------------------------
// Sensors (shared DIS line: LOW = all sensors on, HIGH = asleep)
// ---------------------------------------------------------------------------
static void sensorsOn()  { digitalWrite(PIN_SENSOR_DIS, LOW); }
static void sensorsOff() { digitalWrite(PIN_SENSOR_DIS, HIGH); }

static int medianRead(uint8_t pin, uint8_t n) {
    int s[SENSOR_SAMPLES];
    if (n > SENSOR_SAMPLES) n = SENSOR_SAMPLES;
    for (uint8_t i = 0; i < n; i++) {
        s[i] = analogRead(pin);
        delay(5);
    }
    for (uint8_t i = 1; i < n; i++) { // insertion sort
        int v = s[i];
        int j = i - 1;
        while (j >= 0 && s[j] > v) { s[j + 1] = s[j]; j--; }
        s[j + 1] = v;
    }
    return s[n / 2];
}

// Powers the sensors only for the duration of the reading (extends sensor life).
void measureAll() {
    sensorsOn();
    delay(SENSOR_WARMUP_MS);
    int raw[NUM_PLANTS];
    for (uint8_t i = 0; i < NUM_PLANTS; i++) raw[i] = medianRead(SENSOR_PINS[i], SENSOR_SAMPLES);
    sensorsOff();

    const uint32_t now = millis();
    for (uint8_t i = 0; i < NUM_PLANTS; i++) {
        plants[i].onReading(now, raw[i]);
        Serial.printf("plant %u: raw=%d moisture=%d%% phase=%u\n", i + 1, raw[i],
                      plants[i].moisture(), (unsigned)plants[i].phase());
    }
}

// ---------------------------------------------------------------------------
// Pumps (one at a time) and reservoir float switch
// ---------------------------------------------------------------------------
static int8_t   activePump = -1;
static uint32_t pumpEndMs = 0;
static bool     pumpIsCal = false;

static bool     tankLow = false;
static bool     tankRawLast = false;
static uint32_t tankChangeMs = 0;

enum class Screen : uint8_t { Home, MainMenu, PlantMenu, Edit, CalSensor, CalPumpRun, CalPumpEnter };
static Screen screen = Screen::Home;
static int editValue = 0;
static uint8_t selPlant = 0;

static char toastMsg[22] = "";
static uint32_t toastUntil = 0;
static void showToast(const char *msg) {
    snprintf(toastMsg, sizeof(toastMsg), "%s", msg);
    toastUntil = millis() + 1800;
}

static void startPump(uint8_t i, uint32_t durMs, bool cal) {
    if (durMs > MAX_PUMP_RUN_MS) durMs = MAX_PUMP_RUN_MS;
    activePump = i;
    pumpIsCal = cal;
    pumpEndMs = millis() + durMs;
    if (!cal) plants[i].onPumpStarted();
    digitalWrite(PUMP_PINS[i], HIGH);
    Serial.printf("pump %u ON for %lu ms%s\n", i + 1, (unsigned long)durMs, cal ? " (cal)" : "");
}

static void stopPump(bool aborted) {
    if (activePump < 0) return;
    const uint8_t i = activePump;
    digitalWrite(PUMP_PINS[i], LOW);
    activePump = -1;
    Serial.printf("pump %u OFF%s\n", i + 1, aborted ? " (aborted)" : "");
    if (pumpIsCal) {
        if (!aborted) {
            editValue = (int)(plants[i].cfg.pumpMlPerSec * (PUMP_CAL_MS / 1000.0f) + 0.5f);
            screen = Screen::CalPumpEnter;
        }
    } else if (aborted) {
        plants[i].onPumpAborted();
    } else {
        plants[i].onPumpFinished(millis());
    }
}

static void updateTank(uint32_t now) {
    bool raw = digitalRead(PIN_FLOAT_SW) == LOW; // LOW = switch closed = reservoir low
    if (raw != tankRawLast) {
        tankRawLast = raw;
        tankChangeMs = now;
    }
    if (now - tankChangeMs >= 500) tankLow = raw;
}

static void updatePumps(uint32_t now) {
    if (activePump >= 0) {
        if (tankLow) stopPump(true);
        else if ((int32_t)(now - pumpEndMs) >= 0) stopPump(false);
        return;
    }
    if (tankLow) return;
    for (uint8_t i = 0; i < NUM_PLANTS; i++) {
        if (plants[i].wantsPump()) {
            uint32_t dur = pumpDurationMs(plants[i].cfg.waterMl, plants[i].cfg.pumpMlPerSec);
            startPump(i, dur, false);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Menu model
// ---------------------------------------------------------------------------
enum PlantItem : uint8_t {
    PM_ENABLED, PM_TARGET, PM_MIN, PM_WATER_ML, PM_SOAK, PM_CHECK,
    PM_WATER_NOW, PM_CAL_SENSOR, PM_CAL_PUMP, PM_CLEAR_FAULT, PM_COUNT
};
static const uint8_t MAIN_MENU_COUNT = NUM_PLANTS + 1; // plants + "Read sensors now"

static uint8_t menuSel = 0;
static uint8_t pmSel = 0;
static PlantItem editField = PM_TARGET;
static uint8_t calStage = 0; // 0 = air (dry), 1 = water (wet)
static int calRaw = 0;
static uint32_t calLastReadMs = 0;

static const char *itemLabel(PlantItem f) {
    switch (f) {
    case PM_ENABLED:     return "Enabled";
    case PM_TARGET:      return "Target %";
    case PM_MIN:         return "Min %";
    case PM_WATER_ML:    return "Water mL";
    case PM_SOAK:        return "Soak min";
    case PM_CHECK:       return "Check h";
    case PM_WATER_NOW:   return "Water now";
    case PM_CAL_SENSOR:  return "Cal sensor";
    case PM_CAL_PUMP:    return "Cal pump";
    case PM_CLEAR_FAULT: return "Clear fault";
    default:             return "";
    }
}

static int getField(const PlantConfig &c, PlantItem f) {
    switch (f) {
    case PM_TARGET:   return c.targetPct;
    case PM_MIN:      return c.minPct;
    case PM_WATER_ML: return c.waterMl;
    case PM_SOAK:     return c.soakMin;
    case PM_CHECK:    return c.checkHours;
    default:          return 0;
    }
}

static void setField(PlantConfig &c, PlantItem f, int v) {
    switch (f) {
    case PM_TARGET:   c.targetPct = v; break;
    case PM_MIN:      c.minPct = v; break;
    case PM_WATER_ML: c.waterMl = v; break;
    case PM_SOAK:     c.soakMin = v; break;
    case PM_CHECK:    c.checkHours = v; break;
    default: break;
    }
}

static void fieldRange(PlantItem f, int &lo, int &hi, int &step) {
    switch (f) {
    case PM_TARGET:   lo = 1; hi = 100; step = 1; break;
    case PM_MIN:      lo = 0; hi = 99;  step = 1; break;
    case PM_WATER_ML: lo = 5; hi = 500; step = 5; break;
    case PM_SOAK:     lo = 1; hi = 30;  step = 1; break;
    case PM_CHECK:    lo = 1; hi = 24;  step = 1; break;
    default:          lo = 0; hi = 0;   step = 1; break;
    }
}

static const char *faultName(FaultCode f) {
    switch (f) {
    case FaultCode::NoMoistureRise: return "no rise";
    case FaultCode::MaxDoses:       return "max dose";
    case FaultCode::SensorFault:    return "sensor";
    default:                        return "";
    }
}

// ---------------------------------------------------------------------------
// Input handling
// ---------------------------------------------------------------------------
static int wrap(int v, int n) { return ((v % n) + n) % n; }

static void enterCalSensor() {
    calStage = 0;
    sensorsOn();
    delay(SENSOR_WARMUP_MS);
    calLastReadMs = 0;
    screen = Screen::CalSensor;
}

static void leaveCalSensor() {
    sensorsOff();
    screen = Screen::PlantMenu;
}

static void handlePlantMenuSelect() {
    PlantController &p = plants[selPlant];
    switch ((PlantItem)pmSel) {
    case PM_ENABLED:
        p.cfg.enabled = !p.cfg.enabled;
        saveConfig(selPlant);
        break;
    case PM_TARGET: case PM_MIN: case PM_WATER_ML: case PM_SOAK: case PM_CHECK:
        editField = (PlantItem)pmSel;
        editValue = getField(p.cfg, editField);
        screen = Screen::Edit;
        break;
    case PM_WATER_NOW:
        if (!p.cfg.enabled) showToast("Plant disabled");
        else if (p.phase() != Phase::Dry) showToast("Plant busy");
        else { p.startWatering(); showToast("Watering queued"); screen = Screen::Home; }
        break;
    case PM_CAL_SENSOR:
        enterCalSensor();
        break;
    case PM_CAL_PUMP:
        if (p.phase() != Phase::Dry && p.phase() != Phase::Fault) showToast("Plant busy");
        else screen = Screen::CalPumpRun;
        break;
    case PM_CLEAR_FAULT:
        p.clearFault(millis());
        showToast(p.phase() == Phase::Dry ? "Fault cleared" : "No fault");
        break;
    default: break;
    }
}

static void handleInput(int8_t delta, bool select, bool back) {
    PlantController &p = plants[selPlant];
    switch (screen) {
    case Screen::Home:
        if (select) { menuSel = 0; screen = Screen::MainMenu; }
        break;

    case Screen::MainMenu:
        if (delta) menuSel = wrap(menuSel + (delta > 0 ? 1 : -1), MAIN_MENU_COUNT);
        if (back) screen = Screen::Home;
        if (select) {
            if (menuSel < NUM_PLANTS) { selPlant = menuSel; pmSel = 0; screen = Screen::PlantMenu; }
            else if (activePump >= 0) showToast("Pump running");
            else { measureAll(); showToast("Sensors read"); screen = Screen::Home; }
        }
        break;

    case Screen::PlantMenu:
        if (delta) pmSel = wrap(pmSel + (delta > 0 ? 1 : -1), PM_COUNT);
        if (back) screen = Screen::MainMenu;
        if (select) handlePlantMenuSelect();
        break;

    case Screen::Edit: {
        int lo, hi, step;
        fieldRange(editField, lo, hi, step);
        if (delta) editValue = constrain(editValue + delta * step, lo, hi);
        if (back) screen = Screen::PlantMenu;
        if (select) {
            setField(p.cfg, editField, editValue);
            saveConfig(selPlant);
            screen = Screen::PlantMenu;
        }
        break;
    }

    case Screen::CalSensor:
        if (back) { leaveCalSensor(); break; }
        if (select) {
            int v = medianRead(SENSOR_PINS[selPlant], SENSOR_SAMPLES);
            if (calStage == 0) {
                p.cfg.sensorDry = v;
                calStage = 1;
            } else if (abs(v - (int)p.cfg.sensorDry) < 200) {
                showToast("Dry/wet too close");
            } else {
                p.cfg.sensorWet = v;
                saveConfig(selPlant);
                showToast("Calibrated");
                leaveCalSensor();
            }
        }
        break;

    case Screen::CalPumpRun:
        if (back) {
            if (activePump >= 0 && pumpIsCal) stopPump(true);
            screen = Screen::PlantMenu;
        } else if (select && activePump < 0) {
            if (tankLow) showToast("Reservoir low");
            else startPump(selPlant, PUMP_CAL_MS, true);
        }
        break;

    case Screen::CalPumpEnter:
        if (delta) editValue = constrain(editValue + delta, 1, 500);
        if (back) screen = Screen::PlantMenu;
        if (select) {
            p.cfg.pumpMlPerSec = editValue / (PUMP_CAL_MS / 1000.0f);
            saveConfig(selPlant);
            showToast("Pump saved");
            screen = Screen::PlantMenu;
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Display (128x64, 5x8 font = 8 rows of 25 chars)
// ---------------------------------------------------------------------------
static void drawRow(uint8_t row, const char *text) {
    u8g2.drawStr(0, 8 * (row + 1) - 1, text);
}

static void fmtRemaining(char *out, size_t n, uint32_t ms) {
    uint32_t s = ms / 1000;
    if (s >= 3600) snprintf(out, n, "%luh%02lum", (unsigned long)(s / 3600), (unsigned long)((s % 3600) / 60));
    else snprintf(out, n, "%lum%02lus", (unsigned long)(s / 60), (unsigned long)(s % 60));
}

static void drawHome(uint32_t now) {
    char line[28];
    snprintf(line, sizeof(line), "PLANTS         Tank:%s", tankLow ? "LOW" : "OK");
    drawRow(0, line);
    for (uint8_t i = 0; i < NUM_PLANTS; i++) {
        const PlantController &p = plants[i];
        char moist[6], state[16], rem[10];
        if (p.moisture() < 0) snprintf(moist, sizeof(moist), " --");
        else snprintf(moist, sizeof(moist), "%3d%%", p.moisture());

        if (!p.cfg.enabled) snprintf(state, sizeof(state), "off");
        else switch (p.phase()) {
            case Phase::Dry:
                if (p.moisture() < 0) snprintf(state, sizeof(state), "wait");
                else { fmtRemaining(rem, sizeof(rem), p.checkRemainingMs(now)); snprintf(state, sizeof(state), "chk %s", rem); }
                break;
            case Phase::PumpRequested: snprintf(state, sizeof(state), "queued"); break;
            case Phase::Pumping:       snprintf(state, sizeof(state), "PUMPING"); break;
            case Phase::Soaking:
                fmtRemaining(rem, sizeof(rem), p.soakRemainingMs(now));
                snprintf(state, sizeof(state), "soak %s", rem);
                break;
            case Phase::Fault: snprintf(state, sizeof(state), "!%s", faultName(p.fault())); break;
        }
        snprintf(line, sizeof(line), "%u %s %s", i + 1, moist, state);
        drawRow(i + 1, line);
    }
    drawRow(7, "press = menu");
}

static void drawList(const char *title, uint8_t count, uint8_t sel,
                     void (*itemText)(uint8_t, char *, size_t)) {
    drawRow(0, title);
    const uint8_t visible = 7;
    uint8_t top = sel >= visible ? sel - visible + 1 : 0;
    for (uint8_t r = 0; r < visible && top + r < count; r++) {
        char text[26], line[28];
        itemText(top + r, text, sizeof(text));
        snprintf(line, sizeof(line), "%c%s", (top + r == sel) ? '>' : ' ', text);
        drawRow(r + 1, line);
    }
}

static void mainItemText(uint8_t i, char *out, size_t n) {
    if (i < NUM_PLANTS) {
        const PlantController &p = plants[i];
        snprintf(out, n, "Plant %u  %s", i + 1, p.cfg.enabled ? "on" : "off");
    } else {
        snprintf(out, n, "Read sensors now");
    }
}

static void plantItemText(uint8_t i, char *out, size_t n) {
    const PlantConfig &c = plants[selPlant].cfg;
    const PlantItem f = (PlantItem)i;
    switch (f) {
    case PM_ENABLED: snprintf(out, n, "%-11s %s", itemLabel(f), c.enabled ? "ON" : "OFF"); break;
    case PM_TARGET: case PM_MIN: case PM_WATER_ML: case PM_SOAK: case PM_CHECK:
        snprintf(out, n, "%-11s %d", itemLabel(f), getField(c, f)); break;
    default: snprintf(out, n, "%s", itemLabel(f)); break;
    }
}

static void drawScreen(uint32_t now) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tf);
    char line[28];

    switch (screen) {
    case Screen::Home:
        drawHome(now);
        break;
    case Screen::MainMenu:
        drawList("MENU", MAIN_MENU_COUNT, menuSel, mainItemText);
        break;
    case Screen::PlantMenu:
        snprintf(line, sizeof(line), "PLANT %u", selPlant + 1);
        drawList(line, PM_COUNT, pmSel, plantItemText);
        break;
    case Screen::Edit:
        snprintf(line, sizeof(line), "PLANT %u", selPlant + 1);
        drawRow(0, line);
        drawRow(2, itemLabel(editField));
        snprintf(line, sizeof(line), "  [ %d ]", editValue);
        drawRow(4, line);
        drawRow(7, "CON=save BAK=cancel");
        break;
    case Screen::CalSensor:
        if (now - calLastReadMs >= 250) { calRaw = analogRead(SENSOR_PINS[selPlant]); calLastReadMs = now; }
        snprintf(line, sizeof(line), "CAL SENSOR %u", selPlant + 1);
        drawRow(0, line);
        snprintf(line, sizeof(line), "Raw: %d", calRaw);
        drawRow(2, line);
        drawRow(4, calStage == 0 ? "1/2 hold in AIR" : "2/2 put in WATER");
        drawRow(7, "CON=set BAK=cancel");
        break;
    case Screen::CalPumpRun:
        snprintf(line, sizeof(line), "CAL PUMP %u", selPlant + 1);
        drawRow(0, line);
        if (activePump >= 0 && pumpIsCal) {
            drawRow(2, "Pumping...");
            snprintf(line, sizeof(line), "%lus left", (unsigned long)((pumpEndMs - now) / 1000));
            drawRow(3, line);
        } else {
            drawRow(2, "Hold tube in a cup");
            snprintf(line, sizeof(line), "CON = run %lus", (unsigned long)(PUMP_CAL_MS / 1000));
            drawRow(3, line);
        }
        drawRow(7, "BAK=cancel");
        break;
    case Screen::CalPumpEnter:
        snprintf(line, sizeof(line), "CAL PUMP %u", selPlant + 1);
        drawRow(0, line);
        drawRow(2, "mL in the cup:");
        snprintf(line, sizeof(line), "  [ %d ]", editValue);
        drawRow(4, line);
        drawRow(7, "CON=save BAK=cancel");
        break;
    }

    if (now < toastUntil) {
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 26, 128, 12);
        u8g2.setDrawColor(1);
        u8g2.drawFrame(0, 26, 128, 12);
        u8g2.drawStr(4, 35, toastMsg);
    }
    u8g2.sendBuffer();
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------
void setup() {
    // Outputs first: pumps must be off before anything else happens.
    for (uint8_t i = 0; i < NUM_PLANTS; i++) {
        pinMode(PUMP_PINS[i], OUTPUT);
        digitalWrite(PUMP_PINS[i], LOW);
    }
    pinMode(PIN_SENSOR_DIS, OUTPUT);
    sensorsOff();
    pinMode(PIN_FLOAT_SW, INPUT_PULLUP);

    Serial.begin(115200);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); // sensors are powered from 3V3: full 0-3.3 V range
    for (uint8_t i = 0; i < NUM_PLANTS; i++) pinMode(SENSOR_PINS[i], INPUT);

    Wire.begin(PIN_SDA, PIN_SCL);
    u8g2.begin();

    loadConfig();

    encoder.begin(PIN_TRA, PIN_TRB);
    initButton(btnPsh, PIN_PSH);
    initButton(btnBak, PIN_BAK);
    initButton(btnCon, PIN_CON);

    Serial.println("Plant watering system ready.");
}

void loop() {
    const uint32_t now = millis();
    static uint32_t lastInputMs = 0;
    static uint32_t lastDrawMs = 0;
    static bool displayOn = true;

    // --- Input ---
    updateButton(btnPsh);
    updateButton(btnBak);
    updateButton(btnCon);
    int8_t delta = encoder.read();
    bool select = btnPsh.pressed || btnCon.pressed;
    bool back = btnBak.pressed;
    btnPsh.pressed = btnCon.pressed = btnBak.pressed = false;

    if (delta || select || back) {
        lastInputMs = now;
        if (!displayOn) { // first touch only wakes the display
            displayOn = true;
            u8g2.setPowerSave(0);
            delta = 0; select = false; back = false;
        }
        handleInput(delta, select, back);
    }

    // --- Safety / control ---
    updateTank(now);
    updatePumps(now);

    if (activePump < 0 && screen != Screen::CalSensor) {
        for (uint8_t i = 0; i < NUM_PLANTS; i++) {
            if (plants[i].wantsReading(now)) { measureAll(); break; }
        }
    }

    // --- Display ---
    if (displayOn && screen == Screen::Home && activePump < 0 && now - lastInputMs > DISPLAY_SLEEP_MS) {
        displayOn = false;
        u8g2.setPowerSave(1);
    }
    if (displayOn && now - lastDrawMs >= 200) {
        lastDrawMs = now;
        drawScreen(now);
    }
}
