/*
 * Plant Watering System - Phase 2 (Auto-Watering Firmware)
 * Version: 1.1
 *
 * Hardware: Arduino UNO R4 WiFi
 * Display: 16x2 I2C LCD
 *
 * Features:
 * - Automatic plant watering with soil moisture monitoring
 * - Dry cycle logic with configurable thresholds
 * - Incremental watering with soak time for capillary absorption
 * - Automatic failsafe (stops if moisture not increasing)
 * - 14-bit ADC sensor reading with configurable median sampling
 * - DIS pin sensor control for low-power operation
 * - EEPROM circular buffer logging (~2.7 days @ 15-min intervals)
 * - Menu-driven configuration (Settings, Calibration, Manual Water)
 * - Sensor calibration wizard (air → water or depth lookup)
 * - Pump calibration wizard (measure actual mL output)
 * - CSV data download via Serial (115200 baud)
 * - LCD auto-sleep (configurable backlight timeout)
 *
 * Pin Assignments:
 * - A0: Soil Moisture Sensor (ADC)
 * - D2: Sensor Disable Control (DIS pin, HIGH=disabled, LOW=enabled)
 * - D3: Pump MOSFET Control
 * - D4: Button UP
 * - D5: Button DOWN
 * - D6: Button SELECT
 * - D7: Button BACK
 * - SDA (A4): LCD I2C Data
 * - SCL (A5): LCD I2C Clock
 *
 * Default Configuration:
 * - minThreshold: 55% (start watering below this)
 * - targetHumidity: 75% (stop watering at this)
 * - waterAmountInitial: 50 mL (initial pump)
 * - waterAmountIncrement: 10 mL (subsequent increments)
 * - wateringEnabled: 1 (auto-watering on by default)
 * - soakTime: 15 minutes after initial pump
 * - soakTimeIncrement: 1 minute between increments
 * - pumpRate: 1.67 mL/s (typical S-3Z pump)
 * - sensorDry: 11850 (14-bit ADC in air)
 * - sensorWet: 7700 (14-bit ADC in water @ 60cm)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// ============================================================================
// VERSION INFORMATION
// ============================================================================

#define VERSION_MAJOR       1
#define VERSION_MINOR       1
#define VERSION_STRING      "1.1"
#define FIRMWARE_VERSION    ((VERSION_MAJOR << 8) | VERSION_MINOR)  // 0x0100

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void logMoistureReading(uint8_t flags);
void downloadLog();
void displayMenu();

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

#define SENSOR_ADC_PIN      A0    // Soil moisture sensor analog input (14-bit)
#define SENSOR_DISABLE_PIN  2     // DIS pin control (HIGH=disabled, LOW=enabled)
#define PUMP_PIN            3     // MOSFET gate control for pump
#define BTN_UP              4     // Navigation button: UP
#define BTN_DOWN            5     // Navigation button: DOWN
#define BTN_SELECT          6     // Navigation button: SELECT
#define BTN_BACK            7     // Navigation button: BACK

// ============================================================================
// LCD CONFIGURATION
// ============================================================================

// Try 0x27 first, if that doesn't work try 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================================
// EEPROM MEMORY MAP
// ============================================================================

// Header section (40 bytes: 0-39)
#define EEPROM_MAGIC_ADDR           0    // 2 bytes: Magic number (0xA5C3)
#define EEPROM_VERSION_ADDR         2    // 2 bytes: Firmware version
#define EEPROM_TARGET_ADDR          4    // 1 byte: Target humidity (0-100)
#define EEPROM_MIN_ADDR             5    // 1 byte: Min threshold (0-100)
#define EEPROM_AMOUNT_INITIAL_ADDR  6    // 2 bytes: Initial water amount (mL × 10)
#define EEPROM_AMOUNT_INCREMENT_ADDR 8   // 2 bytes: Increment water amount (mL × 10)
#define EEPROM_INTERVAL_ADDR        10   // 1 byte: Check interval (hours)
#define EEPROM_SOAK_TIME_ADDR       11   // 1 byte: Soak time after initial (minutes)
#define EEPROM_SOAK_TIME_INCREMENT_ADDR 12 // 1 byte: Soak time between increments (minutes)
#define EEPROM_CAL_DRY_ADDR         13   // 2 bytes: Sensor dry ADC value
#define EEPROM_CAL_WET_ADDR         15   // 2 bytes: Sensor wet ADC value
#define EEPROM_PUMP_RATE_ADDR       17   // 4 bytes: Pump rate (mL/s as float)
#define EEPROM_BACKLIGHT_TIMEOUT_ADDR   21  // 2 bytes: Backlight timeout (minutes)
#define EEPROM_SENSOR_WARMUP_ADDR      23  // 2 bytes: Sensor warmup (milliseconds)
#define EEPROM_NUM_MEASUREMENTS_ADDR   25  // 1 byte: Number of measurements (5-50)
#define EEPROM_MEASUREMENT_DELAY_ADDR  26  // 2 bytes: Delay between measurements (ms)
#define EEPROM_LOG_INTERVAL_ADDR       28  // 2 bytes: Log interval (minutes)
#define EEPROM_WRITE_PTR_ADDR          30  // 4 bytes: Circular buffer write pointer
#define EEPROM_TOTAL_ENTRIES_ADDR      34  // 4 bytes: Total entries written
#define EEPROM_WATERING_ENABLED_ADDR   38  // 1 byte: Auto-watering enabled (0=off, 1=on)

// Data section (40-1023: circular buffer for log entries)
#define EEPROM_DATA_START_ADDR      40
#define EEPROM_DATA_END_ADDR        1023
#define EEPROM_DATA_SIZE            984
#define ENTRY_SIZE                  4
#define MAX_ENTRIES                 246  // 984 / 4

#define EEPROM_MAGIC_NUMBER         0xA5C3

// Entry flags
#define FLAG_NORMAL                 0x00
#define FLAG_WATERING_STARTED       0x01
#define FLAG_WATERING_COMPLETE      0x02

// ============================================================================
// CONFIGURATION STRUCTURE
// ============================================================================

struct Config {
    // Watering parameters
    uint8_t targetHumidity;         // Target moisture % (0-100) - stop watering
    uint8_t minThreshold;           // Minimum threshold for dry cycle (0-100) - start watering
    uint16_t waterAmountInitial;    // Initial water amount in mL × 10 (for 0.1 mL precision)
    uint16_t waterAmountIncrement;  // Increment water amount in mL × 10 (for each iteration)
    uint8_t checkInterval;          // Check interval in hours (1-24)
    uint8_t soakTime;               // Soak time after initial pump in minutes (1-30)
    uint8_t soakTimeIncrement;      // Soak time between increments in minutes (1-30)

    // Sensor parameters
    uint16_t sensorDry;             // ADC value when sensor is in air (14-bit: 0-16383)
    uint16_t sensorWet;             // ADC value when sensor in water @ 60cm (14-bit: 0-16383)
    float pumpRate;                 // Pump flow rate in mL/s

    // Display parameters
    uint16_t backlightTimeout;      // Backlight timeout (minutes, 1-60)
    uint16_t sensorWarmup;          // Sensor warmup delay (milliseconds, 100-2000)
    uint8_t numMeasurements;        // Number of samples for median (5-50)
    uint16_t measurementDelay;      // Delay between samples (milliseconds, 10-500)
    uint16_t logInterval;           // Log interval in minutes (1-60)
    uint8_t wateringEnabled;        // Auto-watering active (1=on, 0=off)
};

// Default configuration
Config config = {
    .targetHumidity = 75,           // Target 75% (upper limit - stop watering)
    .minThreshold = 55,             // Water when below 55% (lower limit - start watering)
    .waterAmountInitial = 500,      // 50.0 mL initial watering (stored as 500 = 50.0 * 10)
    .waterAmountIncrement = 100,    // 10.0 mL per increment (stored as 100 = 10.0 * 10)
    .checkInterval = 6,             // Check every 6 hours (during dry cycle)
    .soakTime = 15,                 // Wait 15 minutes after initial pump for moisture propagation
    .soakTimeIncrement = 1,         // Wait 1 minute between increments for moisture propagation
    .sensorDry = 11850,             // Typical 14-bit ADC value in air (user-calibrated)
    .sensorWet = 7700,              // 14-bit ADC value in water @ 60cm
    .pumpRate = 1.67,               // Typical peristaltic pump ~100mL/min = 1.67mL/s
    .backlightTimeout = 1,          // 1 minute LCD sleep
    .sensorWarmup = 1000,           // 1000ms warmup time
    .numMeasurements = 10,          // 10 samples for median
    .measurementDelay = 100,        // 100ms between samples
    .logInterval = 15,              // Log every 15 minutes
    .wateringEnabled = 1            // Auto-watering enabled by default
};

// ============================================================================
// LOG ENTRY STRUCTURE
// ============================================================================

struct LogEntry {
    uint8_t moisturePercent;        // 0-100%
    uint16_t rawADC;                // Median ADC value (14-bit: 0-16383)
    uint8_t flags;                  // Status flags (watering events)
};

// ============================================================================
// MENU STATE MACHINE
// ============================================================================

enum MenuState {
    STATUS_SCREEN,                  // Default: Live moisture display
    MAIN_MENU,                      // Menu selection
    SETTINGS_MENU,                  // Settings submenu
    SETTING_TARGET,                 // Adjust target humidity
    SETTING_MIN,                    // Adjust min threshold
    SETTING_AMOUNT_INITIAL,         // Adjust initial water amount
    SETTING_AMOUNT_INCREMENT,       // Adjust increment water amount
    SETTING_INTERVAL,               // Adjust check interval
    SETTING_SOAK_TIME,              // Adjust soak time after initial
    SETTING_SOAK_TIME_INCREMENT,    // Adjust soak time between increments
    SETTING_LOG_INTERVAL,           // Adjust log interval
    SETTING_SENSOR_WARMUP,          // Adjust sensor warmup
    SETTING_NUM_MEASUREMENTS,       // Adjust num samples
    SETTING_MEASUREMENT_DELAY,      // Adjust measurement delay
    SETTING_BACKLIGHT_TIMEOUT,      // Adjust LCD sleep timeout
    CALIBRATE_MENU,                 // Calibration submenu
    CAL_SENSOR_MENU,                // Sensor calibration submenu
    CAL_SENSOR_AIR,                 // Measure dry value (in air)
    CAL_SENSOR_WATER,               // Measure wet value (in water)
    CAL_SENSOR_DONE,                // Measurement complete
    CAL_PUMP_START,                 // Pump calibration start
    CAL_PUMP_RUNNING,               // Pump running
    CAL_PUMP_ENTER_ML,              // Enter measured mL
    CAL_PUMP_DONE,                  // Pump calibration done
    MANUAL_WATER_CONFIRM,           // Confirm manual watering
    MANUAL_WATER_RUNNING,           // Manual watering in progress
    DOWNLOAD_MENU,                  // Download data menu
    DOWNLOAD_CONFIRM,               // Confirm download
    DOWNLOAD_PROGRESS,              // Downloading...
    RESET_CONFIRM                   // Confirm reset to defaults
};

MenuState currentState = STATUS_SCREEN;
uint8_t menuIndex = 0;              // Current menu selection
uint8_t settingsIndex = 0;          // Settings menu item index
uint8_t calibrateIndex = 0;         // Calibrate menu item index

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

int currentMoisture = 0;            // Current moisture reading (%)
uint16_t currentRawADC = 0;         // Current raw ADC value
unsigned long lastCheckTime = 0;    // Last moisture check timestamp (unused, kept for compatibility)
unsigned long lastWateringTime = 0; // Last watering timestamp
unsigned long lastLogTime = 0;      // Last data log timestamp
uint8_t dryReadingCount = 0;        // Consecutive below-threshold readings (debounce counter)
unsigned long lastDisplayUpdate = 0; // Last display update timestamp
bool backlightOn = true;            // LCD backlight state
unsigned long lastActivity = 0;     // Last user activity timestamp

// Calibration temporary variables
uint16_t tempSensorDry = 0;
uint16_t tempSensorWet = 0;
unsigned long pumpCalibrationStart = 0;
uint16_t pumpCalibrationML = 0;

// Button debouncing
unsigned long lastButtonPress = 0;
#define DEBOUNCE_DELAY 200          // 200ms debounce

// Display update interval
#define DISPLAY_UPDATE_INTERVAL 2000 // 2 seconds

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize serial for debugging and CSV download
    Serial.begin(115200);
    Serial.print(F("Plant Watering System v"));
    Serial.println(F(VERSION_STRING));

    // Configure ADC for 14-bit resolution (0-16383)
    // CRITICAL: Without this, Arduino defaults to 10-bit (0-1023)
    analogReadResolution(14);

    // Initialize pins
    pinMode(SENSOR_DISABLE_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    // Ensure pump is off and sensor is disabled
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);  // HIGH = disabled (low-power mode)

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Plant Watering"));
    lcd.setCursor(0, 1);
    lcd.print(F("v"));
    lcd.print(F(VERSION_STRING));
    delay(2000);

    // Load configuration from EEPROM
    loadConfig();

    // Take initial moisture reading
    currentMoisture = readMoisture();

    // Display status screen
    displayMenu();

    Serial.println(F("Setup complete"));
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    unsigned long currentTime = millis();

    // Handle button input (always responsive)
    handleButtons();

    // Update display if on status screen AND backlight is on
    if (currentState == STATUS_SCREEN && backlightOn &&
        currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        currentMoisture = readMoisture();
        displayMenu();
        lastDisplayUpdate = millis();
    }

    // Check moisture + auto-water at every logInterval (only on status/menu screens)
    if (currentState == STATUS_SCREEN || currentState == MAIN_MENU) {
        unsigned long logIntervalMs = (unsigned long)config.logInterval * 60000UL;
        if (currentTime - lastLogTime >= logIntervalMs) {
            lastLogTime = currentTime;
            currentMoisture = readMoisture();
            logMoistureReading(FLAG_NORMAL);

            if (config.wateringEnabled) {
                if (currentMoisture <= (int)config.minThreshold) {
                    dryReadingCount++;
                    if (dryReadingCount >= 2) {
                        waterPlant();
                        logMoistureReading(FLAG_WATERING_COMPLETE);
                        dryReadingCount = 0;
                    }
                } else {
                    dryReadingCount = 0;
                }
            }
        }
    }

    // Update backlight (auto-sleep)
    updateBacklight();

    delay(50);  // Small delay to reduce CPU usage
}

// ============================================================================
// SENSOR READING
// ============================================================================

int readMoisture() {
    // Enable sensor (DIS LOW = active)
    digitalWrite(SENSOR_DISABLE_PIN, LOW);
    delay(config.sensorWarmup);

    // Allocate sample array
    int* samples = new int[config.numMeasurements];
    int minADC = 16383;
    int maxADC = 0;

    // Collect samples
    for (int i = 0; i < config.numMeasurements; i++) {
        int reading = analogRead(SENSOR_ADC_PIN);
        samples[i] = reading;

        if (reading < minADC) minADC = reading;
        if (reading > maxADC) maxADC = reading;

        if (i < config.numMeasurements - 1) {
            delay(config.measurementDelay);
        }
    }

    // Sort for median (bubble sort)
    for (int i = 0; i < config.numMeasurements - 1; i++) {
        for (int j = 0; j < config.numMeasurements - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                int temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    // Calculate median
    int medianReading;
    if (config.numMeasurements % 2 == 0) {
        int mid = config.numMeasurements / 2;
        medianReading = (samples[mid - 1] + samples[mid]) / 2;
    } else {
        medianReading = samples[config.numMeasurements / 2];
    }

    delete[] samples;

    currentRawADC = medianReading;
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);  // Disable sensor (DIS HIGH = sleep)

    // Convert to percentage using calibration values
    int moisturePercent = map(medianReading, config.sensorDry, config.sensorWet, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.print(F("ADC: "));
    Serial.print(medianReading);
    Serial.print(F(" ("));
    Serial.print(minADC);
    Serial.print(F("-"));
    Serial.print(maxADC);
    Serial.print(F(") -> "));
    Serial.print(moisturePercent);
    Serial.println(F("%"));

    return moisturePercent;
}

int readMoistureRaw() {
    // Read raw ADC value (for calibration)
    digitalWrite(SENSOR_DISABLE_PIN, LOW);
    delay(config.sensorWarmup);

    int* samples = new int[config.numMeasurements];

    for (int i = 0; i < config.numMeasurements; i++) {
        samples[i] = analogRead(SENSOR_ADC_PIN);
        if (i < config.numMeasurements - 1) {
            delay(config.measurementDelay);
        }
    }

    // Sort for median
    for (int i = 0; i < config.numMeasurements - 1; i++) {
        for (int j = 0; j < config.numMeasurements - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                int temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    int median;
    if (config.numMeasurements % 2 == 0) {
        int mid = config.numMeasurements / 2;
        median = (samples[mid - 1] + samples[mid]) / 2;
    } else {
        median = samples[config.numMeasurements / 2];
    }

    delete[] samples;
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);

    return median;
}

// ============================================================================
// WATERING LOGIC
// ============================================================================

bool shouldWater() {
    // Dry cycle logic: only water if at or below minimum threshold
    return currentMoisture <= config.minThreshold;
}

void waterPlant() {
    Serial.println(F("Starting watering cycle..."));

    int initialMoisture = currentMoisture;
    int iterationCount = 0;
    const int maxIterations = 20;  // Safety limit

    // PHASE 1: Initial pump
    {
        Serial.println(F("Phase 1: Initial pump"));
        float amountML = config.waterAmountInitial / 10.0;
        pumpWater(amountML);

        // Wait for moisture to propagate
        Serial.print(F("Waiting "));
        Serial.print(config.soakTime);
        Serial.println(F(" minutes for moisture propagation..."));

        unsigned long soakTimeMs = (unsigned long)config.soakTime * 60000UL;
        unsigned long soakStart = millis();
        while (millis() - soakStart < soakTimeMs) {
            currentMoisture = readMoisture();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Soaking  "));
            lcd.print(currentMoisture);
            lcd.print(F("%"));
            lcd.setCursor(0, 1);
            unsigned long remaining = (soakTimeMs - (millis() - soakStart)) / 1000;
            lcd.print(remaining);
            lcd.print(F("s left"));
        }

        initialMoisture = currentMoisture;
    }

    // PHASE 2: Incremental pump until target reached
    while (currentMoisture < config.targetHumidity && iterationCount < maxIterations) {
        iterationCount++;

        Serial.print(F("Phase 2: Increment "));
        Serial.println(iterationCount);

        // Pump increment amount (mL)
        float amountML = config.waterAmountIncrement / 10.0;
        pumpWater(amountML);

        // Wait for moisture to propagate
        Serial.print(F("Waiting "));
        Serial.print(config.soakTimeIncrement);
        Serial.println(F(" minutes for moisture propagation..."));

        // Display watering status during soak time
        unsigned long soakTimeMs = (unsigned long)config.soakTimeIncrement * 60000UL;
        unsigned long soakStart = millis();
        while (millis() - soakStart < soakTimeMs) {
            currentMoisture = readMoisture();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Soaking  "));
            lcd.print(currentMoisture);
            lcd.print(F("%"));
            lcd.setCursor(0, 1);
            unsigned long remaining = (soakTimeMs - (millis() - soakStart)) / 1000;
            lcd.print(remaining);
            lcd.print(F("s left"));
        }

        // Failsafe: if moisture didn't increase, stop watering
        if (currentMoisture <= initialMoisture) {
            Serial.println(F("ERROR: Moisture not increasing - stopping!"));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("ERROR!"));
            lcd.setCursor(0, 1);
            lcd.print(F("Check sensor/pump"));
            delay(5000);
            break;
        }

        initialMoisture = currentMoisture;
    }

    if (iterationCount >= maxIterations) {
        Serial.println(F("WARNING: Max watering iterations reached"));
    }

    Serial.println(F("Watering cycle complete"));
    lastWateringTime = millis();
}

void pumpWater(float mL) {
    // Calculate pump run time based on flow rate
    float seconds = mL / config.pumpRate;
    unsigned long pumpTimeMs = (unsigned long)(seconds * 1000);

    Serial.print(F("Pumping "));
    Serial.print(mL);
    Serial.print(F("mL ("));
    Serial.print(seconds);
    Serial.println(F("s)"));

    // Turn on pump
    digitalWrite(PUMP_PIN, HIGH);

    // Display pumping status
    unsigned long pumpStart = millis();
    while (millis() - pumpStart < pumpTimeMs) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Pumping..."));
        lcd.setCursor(0, 1);
        unsigned long remaining = (pumpTimeMs - (millis() - pumpStart)) / 1000;
        lcd.print(remaining);
        lcd.print(F("s left"));
        delay(500);
    }

    // Turn off pump
    digitalWrite(PUMP_PIN, LOW);
}

// ============================================================================
// EEPROM & CONFIGURATION
// ============================================================================

void loadConfig() {
    // Check magic number
    uint16_t magic = EEPROM.read(EEPROM_MAGIC_ADDR) | (EEPROM.read(EEPROM_MAGIC_ADDR + 1) << 8);

    if (magic != EEPROM_MAGIC_NUMBER) {
        Serial.println(F("No valid config found, using defaults"));
        saveConfig();
        return;
    }

    // Load configuration from EEPROM
    config.targetHumidity = EEPROM.read(EEPROM_TARGET_ADDR);
    config.minThreshold = EEPROM.read(EEPROM_MIN_ADDR);
    config.waterAmountInitial = EEPROM.read(EEPROM_AMOUNT_INITIAL_ADDR) | (EEPROM.read(EEPROM_AMOUNT_INITIAL_ADDR + 1) << 8);
    config.waterAmountIncrement = EEPROM.read(EEPROM_AMOUNT_INCREMENT_ADDR) | (EEPROM.read(EEPROM_AMOUNT_INCREMENT_ADDR + 1) << 8);
    config.checkInterval = EEPROM.read(EEPROM_INTERVAL_ADDR);
    config.soakTime = EEPROM.read(EEPROM_SOAK_TIME_ADDR);
    config.soakTimeIncrement = EEPROM.read(EEPROM_SOAK_TIME_INCREMENT_ADDR);
    config.sensorDry = EEPROM.read(EEPROM_CAL_DRY_ADDR) | (EEPROM.read(EEPROM_CAL_DRY_ADDR + 1) << 8);
    config.sensorWet = EEPROM.read(EEPROM_CAL_WET_ADDR) | (EEPROM.read(EEPROM_CAL_WET_ADDR + 1) << 8);

    // Load pump rate (float - 4 bytes)
    byte* pumpRatePtr = (byte*)&config.pumpRate;
    for (int i = 0; i < 4; i++) {
        pumpRatePtr[i] = EEPROM.read(EEPROM_PUMP_RATE_ADDR + i);
    }

    // Load display parameters
    config.backlightTimeout = EEPROM.read(EEPROM_BACKLIGHT_TIMEOUT_ADDR) | (EEPROM.read(EEPROM_BACKLIGHT_TIMEOUT_ADDR + 1) << 8);
    config.sensorWarmup = EEPROM.read(EEPROM_SENSOR_WARMUP_ADDR) | (EEPROM.read(EEPROM_SENSOR_WARMUP_ADDR + 1) << 8);
    config.numMeasurements = EEPROM.read(EEPROM_NUM_MEASUREMENTS_ADDR);
    config.measurementDelay = EEPROM.read(EEPROM_MEASUREMENT_DELAY_ADDR) | (EEPROM.read(EEPROM_MEASUREMENT_DELAY_ADDR + 1) << 8);
    config.logInterval = EEPROM.read(EEPROM_LOG_INTERVAL_ADDR) | (EEPROM.read(EEPROM_LOG_INTERVAL_ADDR + 1) << 8);
    config.wateringEnabled = EEPROM.read(EEPROM_WATERING_ENABLED_ADDR);

    Serial.println(F("Configuration loaded from EEPROM"));
}

void saveConfig() {
    // Write magic number
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_NUMBER & 0xFF);
    EEPROM.write(EEPROM_MAGIC_ADDR + 1, (EEPROM_MAGIC_NUMBER >> 8) & 0xFF);

    // Write configuration
    EEPROM.write(EEPROM_TARGET_ADDR, config.targetHumidity);
    EEPROM.write(EEPROM_MIN_ADDR, config.minThreshold);
    EEPROM.write(EEPROM_AMOUNT_INITIAL_ADDR, config.waterAmountInitial & 0xFF);
    EEPROM.write(EEPROM_AMOUNT_INITIAL_ADDR + 1, (config.waterAmountInitial >> 8) & 0xFF);
    EEPROM.write(EEPROM_AMOUNT_INCREMENT_ADDR, config.waterAmountIncrement & 0xFF);
    EEPROM.write(EEPROM_AMOUNT_INCREMENT_ADDR + 1, (config.waterAmountIncrement >> 8) & 0xFF);
    EEPROM.write(EEPROM_INTERVAL_ADDR, config.checkInterval);
    EEPROM.write(EEPROM_SOAK_TIME_ADDR, config.soakTime);
    EEPROM.write(EEPROM_SOAK_TIME_INCREMENT_ADDR, config.soakTimeIncrement);
    EEPROM.write(EEPROM_CAL_DRY_ADDR, config.sensorDry & 0xFF);
    EEPROM.write(EEPROM_CAL_DRY_ADDR + 1, (config.sensorDry >> 8) & 0xFF);
    EEPROM.write(EEPROM_CAL_WET_ADDR, config.sensorWet & 0xFF);
    EEPROM.write(EEPROM_CAL_WET_ADDR + 1, (config.sensorWet >> 8) & 0xFF);

    // Write pump rate (float - 4 bytes)
    byte* pumpRatePtr = (byte*)&config.pumpRate;
    for (int i = 0; i < 4; i++) {
        EEPROM.write(EEPROM_PUMP_RATE_ADDR + i, pumpRatePtr[i]);
    }

    // Write display parameters
    EEPROM.write(EEPROM_BACKLIGHT_TIMEOUT_ADDR, config.backlightTimeout & 0xFF);
    EEPROM.write(EEPROM_BACKLIGHT_TIMEOUT_ADDR + 1, (config.backlightTimeout >> 8) & 0xFF);
    EEPROM.write(EEPROM_SENSOR_WARMUP_ADDR, config.sensorWarmup & 0xFF);
    EEPROM.write(EEPROM_SENSOR_WARMUP_ADDR + 1, (config.sensorWarmup >> 8) & 0xFF);
    EEPROM.write(EEPROM_NUM_MEASUREMENTS_ADDR, config.numMeasurements);
    EEPROM.write(EEPROM_MEASUREMENT_DELAY_ADDR, config.measurementDelay & 0xFF);
    EEPROM.write(EEPROM_MEASUREMENT_DELAY_ADDR + 1, (config.measurementDelay >> 8) & 0xFF);
    EEPROM.write(EEPROM_LOG_INTERVAL_ADDR, config.logInterval & 0xFF);
    EEPROM.write(EEPROM_LOG_INTERVAL_ADDR + 1, (config.logInterval >> 8) & 0xFF);
    EEPROM.write(EEPROM_WATERING_ENABLED_ADDR, config.wateringEnabled);

    Serial.println(F("Configuration saved to EEPROM"));
}

// ============================================================================
// DATA LOGGING
// ============================================================================

void logMoistureReading(uint8_t flags) {
    // Get current write pointer
    uint32_t writePtr = EEPROM.read(EEPROM_WRITE_PTR_ADDR) |
                        (EEPROM.read(EEPROM_WRITE_PTR_ADDR + 1) << 8) |
                        (EEPROM.read(EEPROM_WRITE_PTR_ADDR + 2) << 16) |
                        (EEPROM.read(EEPROM_WRITE_PTR_ADDR + 3) << 24);

    // Create log entry
    LogEntry entry;
    entry.moisturePercent = currentMoisture;
    entry.rawADC = currentRawADC;
    entry.flags = flags;

    // Write to circular buffer
    int offset = EEPROM_DATA_START_ADDR + (writePtr * ENTRY_SIZE);

    // Check if we're at the end of buffer
    if (offset + ENTRY_SIZE > EEPROM_DATA_END_ADDR) {
        writePtr = 0;  // Wrap around
        offset = EEPROM_DATA_START_ADDR;
    }

    // Write entry to EEPROM
    EEPROM.write(offset, entry.moisturePercent);
    EEPROM.write(offset + 1, entry.rawADC & 0xFF);
    EEPROM.write(offset + 2, (entry.rawADC >> 8) & 0xFF);
    EEPROM.write(offset + 3, entry.flags);

    // Increment and save write pointer
    writePtr++;
    if (writePtr >= MAX_ENTRIES) writePtr = 0;

    EEPROM.write(EEPROM_WRITE_PTR_ADDR, writePtr & 0xFF);
    EEPROM.write(EEPROM_WRITE_PTR_ADDR + 1, (writePtr >> 8) & 0xFF);
    EEPROM.write(EEPROM_WRITE_PTR_ADDR + 2, (writePtr >> 16) & 0xFF);
    EEPROM.write(EEPROM_WRITE_PTR_ADDR + 3, (writePtr >> 24) & 0xFF);

    // Increment total entries written
    uint32_t totalEntries = EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR) |
                            (EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 1) << 8) |
                            (EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 2) << 16) |
                            (EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 3) << 24);
    totalEntries++;

    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR, totalEntries & 0xFF);
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR + 1, (totalEntries >> 8) & 0xFF);
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR + 2, (totalEntries >> 16) & 0xFF);
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR + 3, (totalEntries >> 24) & 0xFF);
}

void downloadLog() {
    // Get total entries
    uint32_t totalEntries = EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR) |
                            (EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 1) << 8) |
                            (EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 2) << 16) |
                            (EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 3) << 24);

    // Get write pointer
    uint32_t writePtr = EEPROM.read(EEPROM_WRITE_PTR_ADDR) |
                        (EEPROM.read(EEPROM_WRITE_PTR_ADDR + 1) << 8) |
                        (EEPROM.read(EEPROM_WRITE_PTR_ADDR + 2) << 16) |
                        (EEPROM.read(EEPROM_WRITE_PTR_ADDR + 3) << 24);

    Serial.println(F("Time(h),Moisture(%),RawADC,Flags"));

    if (totalEntries == 0) {
        Serial.println(F("No data logged"));
        return;
    }

    // Determine start position
    uint32_t startPtr = 0;
    uint32_t entriesToRead = totalEntries;

    if (totalEntries > MAX_ENTRIES) {
        // Buffer has wrapped, start from writePtr
        startPtr = writePtr;
        entriesToRead = MAX_ENTRIES;
    }

    // Read and print log entries
    for (uint32_t i = 0; i < entriesToRead; i++) {
        uint32_t ptr = (startPtr + i) % MAX_ENTRIES;
        int offset = EEPROM_DATA_START_ADDR + (ptr * ENTRY_SIZE);

        uint8_t moisturePercent = EEPROM.read(offset);
        uint16_t rawADC = EEPROM.read(offset + 1) | (EEPROM.read(offset + 2) << 8);
        uint8_t flags = EEPROM.read(offset + 3);

        // Calculate time in hours (assuming logInterval minutes)
        float timeHours = (i * config.logInterval) / 60.0;

        Serial.print(timeHours);
        Serial.print(F(","));
        Serial.print(moisturePercent);
        Serial.print(F(","));
        Serial.print(rawADC);
        Serial.print(F(","));
        Serial.println(flags);
    }

    Serial.println(F("Download complete"));
}

// ============================================================================
// BUTTON HANDLING
// ============================================================================

void handleButtons() {
    unsigned long currentTime = millis();

    // Check for button press
    bool upPressed = digitalRead(BTN_UP) == LOW;
    bool downPressed = digitalRead(BTN_DOWN) == LOW;
    bool selectPressed = digitalRead(BTN_SELECT) == LOW;
    bool backPressed = digitalRead(BTN_BACK) == LOW;

    if ((upPressed || downPressed || selectPressed || backPressed) &&
        (currentTime - lastButtonPress >= DEBOUNCE_DELAY)) {
        lastButtonPress = currentTime;
        lastActivity = currentTime;  // Reset backlight timer
        backlightOn = true;
        lcd.backlight();

        if (upPressed) handleMenuUp();
        else if (downPressed) handleMenuDown();
        else if (selectPressed) handleMenuSelect();
        else if (backPressed) handleMenuBack();

        displayMenu();
    }
}

void handleMenuUp() {
    switch (currentState) {
        case STATUS_SCREEN:
            currentState = MAIN_MENU;
            menuIndex = 0;
            break;
        case MAIN_MENU:
            menuIndex = (menuIndex - 1 + 6) % 6;
            break;
        case SETTINGS_MENU:
            settingsIndex = (settingsIndex - 1 + 14) % 14;
            break;
        case SETTING_TARGET:
        case SETTING_MIN:
        case SETTING_INTERVAL:
        case SETTING_SOAK_TIME:
        case SETTING_SOAK_TIME_INCREMENT:
        case SETTING_LOG_INTERVAL:
        case SETTING_SENSOR_WARMUP:
        case SETTING_NUM_MEASUREMENTS:
        case SETTING_MEASUREMENT_DELAY:
        case SETTING_BACKLIGHT_TIMEOUT:
            handleSettingAdjust(1);
            break;
        case SETTING_AMOUNT_INITIAL:
        case SETTING_AMOUNT_INCREMENT:
            handleSettingAdjust(1);
            break;
    }
}

void handleMenuDown() {
    switch (currentState) {
        case MAIN_MENU:
            menuIndex = (menuIndex + 1) % 6;
            break;
        case SETTINGS_MENU:
            settingsIndex = (settingsIndex + 1) % 14;
            break;
        case SETTING_TARGET:
        case SETTING_MIN:
        case SETTING_INTERVAL:
        case SETTING_SOAK_TIME:
        case SETTING_SOAK_TIME_INCREMENT:
        case SETTING_LOG_INTERVAL:
        case SETTING_SENSOR_WARMUP:
        case SETTING_NUM_MEASUREMENTS:
        case SETTING_MEASUREMENT_DELAY:
        case SETTING_BACKLIGHT_TIMEOUT:
            handleSettingAdjust(-1);
            break;
        case SETTING_AMOUNT_INITIAL:
        case SETTING_AMOUNT_INCREMENT:
            handleSettingAdjust(-1);
            break;
    }
}

void handleMenuSelect() {
    switch (currentState) {
        case STATUS_SCREEN:
            currentState = MAIN_MENU;
            menuIndex = 0;
            break;
        case MAIN_MENU:
            switch (menuIndex) {
                case 0: currentState = SETTINGS_MENU; settingsIndex = 0; break;
                case 1: currentState = CALIBRATE_MENU; calibrateIndex = 0; break;
                case 2: currentState = MANUAL_WATER_CONFIRM; break;
                case 3: currentState = DOWNLOAD_CONFIRM; break;
                case 4: currentState = RESET_CONFIRM; break;
                case 5:
                    config.wateringEnabled = config.wateringEnabled ? 0 : 1;
                    saveConfig();
                    displayMenu();
                    break;
                default: currentState = MAIN_MENU; break;
            }
            break;
        case SETTINGS_MENU:
            currentState = (MenuState)(SETTING_TARGET + settingsIndex);
            break;
        case CALIBRATE_MENU:
            if (calibrateIndex == 0) {
                currentState = CAL_SENSOR_MENU;
            } else if (calibrateIndex == 1) {
                currentState = CAL_PUMP_START;
            }
            break;
        case CAL_SENSOR_MENU:
            if (calibrateIndex == 0) {
                currentState = CAL_SENSOR_AIR;
            } else {
                currentState = CAL_SENSOR_WATER;
            }
            break;
        case CAL_SENSOR_AIR:
            tempSensorDry = readMoistureRaw();
            config.sensorDry = tempSensorDry;
            currentState = CAL_SENSOR_DONE;
            break;
        case CAL_SENSOR_WATER:
            tempSensorWet = readMoistureRaw();
            config.sensorWet = tempSensorWet;
            currentState = CAL_SENSOR_DONE;
            break;
        case CAL_SENSOR_DONE:
            saveConfig();
            currentState = CALIBRATE_MENU;
            break;
        case CAL_PUMP_START:
            currentState = CAL_PUMP_RUNNING;
            break;
        case CAL_PUMP_RUNNING:
            pumpCalibrationStart = millis();
            digitalWrite(PUMP_PIN, HIGH);
            break;
        case CAL_PUMP_ENTER_ML:
            config.pumpRate = (pumpCalibrationML / 10.0) / ((millis() - pumpCalibrationStart) / 1000.0);
            saveConfig();
            currentState = CAL_PUMP_DONE;
            break;
        case MANUAL_WATER_CONFIRM:
            waterPlant();
            currentState = STATUS_SCREEN;
            break;
        case DOWNLOAD_CONFIRM:
            downloadLog();
            currentState = STATUS_SCREEN;
            break;
        case RESET_CONFIRM:
            // Reset to defaults - already in config struct
            saveConfig();
            currentState = STATUS_SCREEN;
            break;
    }
}

void handleMenuBack() {
    switch (currentState) {
        case MAIN_MENU:
            currentState = STATUS_SCREEN;
            break;
        case SETTINGS_MENU:
            currentState = MAIN_MENU;
            menuIndex = 0;
            break;
        case SETTING_TARGET:
        case SETTING_MIN:
        case SETTING_AMOUNT_INITIAL:
        case SETTING_AMOUNT_INCREMENT:
        case SETTING_INTERVAL:
        case SETTING_SOAK_TIME:
        case SETTING_SOAK_TIME_INCREMENT:
        case SETTING_LOG_INTERVAL:
        case SETTING_SENSOR_WARMUP:
        case SETTING_NUM_MEASUREMENTS:
        case SETTING_MEASUREMENT_DELAY:
        case SETTING_BACKLIGHT_TIMEOUT:
            saveConfig();
            currentState = SETTINGS_MENU;
            settingsIndex = 0;
            break;
        case CALIBRATE_MENU:
            currentState = MAIN_MENU;
            menuIndex = 1;
            break;
        case CAL_SENSOR_MENU:
        case CAL_PUMP_START:
        case CAL_PUMP_DONE:
            currentState = CALIBRATE_MENU;
            calibrateIndex = 0;
            break;
        case CAL_SENSOR_AIR:
        case CAL_SENSOR_WATER:
        case CAL_SENSOR_DONE:
            currentState = CAL_SENSOR_MENU;
            break;
        case CAL_PUMP_RUNNING:
            digitalWrite(PUMP_PIN, LOW);
            currentState = CAL_PUMP_ENTER_ML;
            break;
        case CAL_PUMP_ENTER_ML:
            digitalWrite(PUMP_PIN, LOW);
            currentState = CAL_PUMP_START;
            break;
        default:
            currentState = STATUS_SCREEN;
            break;
    }
}

void handleSettingAdjust(int delta) {
    switch (currentState) {
        case SETTING_TARGET:
            config.targetHumidity = constrain(config.targetHumidity + delta, 0, 100);
            break;
        case SETTING_MIN:
            config.minThreshold = constrain(config.minThreshold + delta, 0, 100);
            break;
        case SETTING_AMOUNT_INITIAL:
            config.waterAmountInitial = constrain(config.waterAmountInitial + delta * 10, 10, 5000);
            break;
        case SETTING_AMOUNT_INCREMENT:
            config.waterAmountIncrement = constrain(config.waterAmountIncrement + delta * 10, 10, 5000);
            break;
        case SETTING_INTERVAL:
            config.checkInterval = constrain(config.checkInterval + delta, 1, 24);
            break;
        case SETTING_SOAK_TIME:
            config.soakTime = constrain(config.soakTime + delta, 1, 30);
            break;
        case SETTING_SOAK_TIME_INCREMENT:
            config.soakTimeIncrement = constrain(config.soakTimeIncrement + delta, 1, 30);
            break;
        case SETTING_LOG_INTERVAL:
            config.logInterval = constrain(config.logInterval + delta, 1, 60);
            break;
        case SETTING_SENSOR_WARMUP:
            config.sensorWarmup = constrain(config.sensorWarmup + delta * 100, 100, 2000);
            break;
        case SETTING_NUM_MEASUREMENTS:
            config.numMeasurements = constrain(config.numMeasurements + delta, 5, 50);
            break;
        case SETTING_MEASUREMENT_DELAY:
            config.measurementDelay = constrain(config.measurementDelay + delta * 10, 10, 500);
            break;
        case SETTING_BACKLIGHT_TIMEOUT:
            config.backlightTimeout = constrain(config.backlightTimeout + delta, 1, 60);
            break;
    }
}

// ============================================================================
// DISPLAY
// ============================================================================

void displayMenu() {
    lcd.clear();

    switch (currentState) {
        case STATUS_SCREEN:
            displayStatusScreen();
            break;
        case MAIN_MENU:
            displayMainMenu();
            break;
        case SETTINGS_MENU:
            displaySettingsMenu();
            break;
        case SETTING_TARGET:
        case SETTING_MIN:
        case SETTING_AMOUNT_INITIAL:
        case SETTING_AMOUNT_INCREMENT:
        case SETTING_INTERVAL:
        case SETTING_SOAK_TIME:
        case SETTING_SOAK_TIME_INCREMENT:
        case SETTING_LOG_INTERVAL:
        case SETTING_SENSOR_WARMUP:
        case SETTING_NUM_MEASUREMENTS:
        case SETTING_MEASUREMENT_DELAY:
        case SETTING_BACKLIGHT_TIMEOUT:
            displaySetting();
            break;
        case CALIBRATE_MENU:
            displayCalibrateMenu();
            break;
        case CAL_SENSOR_MENU:
            displayCalSensorMenu();
            break;
        case CAL_SENSOR_AIR:
        case CAL_SENSOR_WATER:
            displayCalSensorReading();
            break;
        case CAL_SENSOR_DONE:
            displayCalSensorDone();
            break;
        case CAL_PUMP_START:
            displayCalPumpStart();
            break;
        case CAL_PUMP_RUNNING:
            displayCalPumpRunning();
            break;
        case CAL_PUMP_ENTER_ML:
            displayCalPumpEnterML();
            break;
        case CAL_PUMP_DONE:
            displayCalPumpDone();
            break;
        case MANUAL_WATER_CONFIRM:
            displayManualWaterConfirm();
            break;
        case DOWNLOAD_CONFIRM:
            displayDownloadConfirm();
            break;
        case RESET_CONFIRM:
            displayResetConfirm();
            break;
    }
}

void displayStatusScreen() {
    lcd.setCursor(0, 0);
    lcd.print(F("Moisture: "));
    lcd.print(currentMoisture);
    lcd.print(F("%  "));

    lcd.setCursor(0, 1);
    if (!config.wateringEnabled) {
        lcd.print(F("Watering: OFF  "));
    } else if (currentMoisture <= config.minThreshold) {
        if (dryReadingCount >= 1) {
            lcd.print(F("Dry! Watering  "));
        } else {
            lcd.print(F("Need water!    "));
        }
    } else {
        lcd.print(F("OK             "));
    }
}

void displayMainMenu() {
    lcd.setCursor(0, 0);
    if (menuIndex == 5) {
        lcd.print(config.wateringEnabled ? "6.Watering: ON " : "6.Watering: OFF");
    } else {
        const char* menuItems[] = {
            "1.Settings",
            "2.Calibrate",
            "3.Manual Water",
            "4.Download Log",
            "5.Reset"
        };
        lcd.print(menuItems[menuIndex]);
    }
    lcd.setCursor(0, 1);
    lcd.print(F("Press SELECT"));
}

void displaySettingsMenu() {
    const char* settingItems[] = {
        "Target %",
        "Min Thresh %",
        "Init Water mL",
        "Incr Water mL",
        "Check hrs",
        "Soak min",
        "Soak Incr min",
        "Log min",
        "Warmup ms",
        "Samples",
        "Delay ms",
        "Backlight",
        "-",
        "-"
    };

    lcd.setCursor(0, 0);
    lcd.print(settingItems[settingsIndex]);
    lcd.setCursor(0, 1);
    lcd.print(F("Press SELECT"));
}

void displaySetting() {
    uint16_t value = 0;
    const char* unit = "";

    switch (currentState) {
        case SETTING_TARGET:
            value = config.targetHumidity;
            unit = "%";
            lcd.print(F("Target: "));
            break;
        case SETTING_MIN:
            value = config.minThreshold;
            unit = "%";
            lcd.print(F("Min: "));
            break;
        case SETTING_AMOUNT_INITIAL:
            value = config.waterAmountInitial / 10;
            unit = "mL";
            lcd.print(F("Init: "));
            break;
        case SETTING_AMOUNT_INCREMENT:
            value = config.waterAmountIncrement / 10;
            unit = "mL";
            lcd.print(F("Incr: "));
            break;
        case SETTING_INTERVAL:
            value = config.checkInterval;
            unit = "h";
            lcd.print(F("Interval: "));
            break;
        case SETTING_SOAK_TIME:
            value = config.soakTime;
            unit = "min";
            lcd.print(F("Soak: "));
            break;
        case SETTING_SOAK_TIME_INCREMENT:
            value = config.soakTimeIncrement;
            unit = "min";
            lcd.print(F("Soak Incr: "));
            break;
        case SETTING_LOG_INTERVAL:
            value = config.logInterval;
            unit = "min";
            lcd.print(F("Log Int: "));
            break;
        case SETTING_SENSOR_WARMUP:
            value = config.sensorWarmup;
            unit = "ms";
            lcd.print(F("Warmup: "));
            break;
        case SETTING_NUM_MEASUREMENTS:
            value = config.numMeasurements;
            unit = "";
            lcd.print(F("Samples: "));
            break;
        case SETTING_MEASUREMENT_DELAY:
            value = config.measurementDelay;
            unit = "ms";
            lcd.print(F("Delay: "));
            break;
        case SETTING_BACKLIGHT_TIMEOUT:
            value = config.backlightTimeout;
            unit = "min";
            lcd.print(F("Backlight: "));
            break;
    }

    lcd.setCursor(0, 1);
    lcd.print(value);
    lcd.print(unit);
    lcd.print(F(" (UP/DN)"));
}

void displayCalibrateMenu() {
    const char* calibrateItems[] = {
        "1.Sensor",
        "2.Pump",
        "3.-",
        "4.-"
    };

    lcd.setCursor(0, 0);
    lcd.print(calibrateItems[calibrateIndex]);
    lcd.setCursor(0, 1);
    lcd.print(F("Press SELECT"));
}

void displayCalSensorMenu() {
    const char* sensorCalItems[] = {
        "1.Measure Air",
        "2.Measure Water"
    };

    lcd.setCursor(0, 0);
    lcd.print(sensorCalItems[calibrateIndex]);
    lcd.setCursor(0, 1);
    lcd.print(F("Press SELECT"));
}

void displayCalSensorReading() {
    uint16_t reading = readMoistureRaw();
    lcd.setCursor(0, 0);
    if (currentState == CAL_SENSOR_AIR) {
        lcd.print(F("Air: "));
    } else {
        lcd.print(F("Water: "));
    }
    lcd.print(reading);

    lcd.setCursor(0, 1);
    lcd.print(F("SELECT to save"));
}

void displayCalSensorDone() {
    lcd.setCursor(0, 0);
    lcd.print(F("Cal saved!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Dry="));
    lcd.print(config.sensorDry);
    lcd.print(F(" Wet="));
    lcd.print(config.sensorWet);
}

void displayCalPumpStart() {
    lcd.setCursor(0, 0);
    lcd.print(F("Pump Calibration"));
    lcd.setCursor(0, 1);
    lcd.print(F("SELECT to run"));
}

void displayCalPumpRunning() {
    unsigned long elapsed = millis() - pumpCalibrationStart;
    lcd.setCursor(0, 0);
    lcd.print(F("Pump running..."));
    lcd.setCursor(0, 1);
    lcd.print(elapsed / 1000);
    lcd.print(F("s (BACK stop)"));
}

void displayCalPumpEnterML() {
    lcd.setCursor(0, 0);
    lcd.print(F("Enter mL:"));
    lcd.print(pumpCalibrationML / 10);
    lcd.setCursor(0, 1);
    lcd.print(F("UP/DN then SELECT"));
}

void displayCalPumpDone() {
    lcd.setCursor(0, 0);
    lcd.print(F("Rate: "));
    lcd.print(config.pumpRate);
    lcd.print(F("mL/s"));
    lcd.setCursor(0, 1);
    lcd.print(F("Press BACK"));
}

void displayManualWaterConfirm() {
    lcd.setCursor(0, 0);
    lcd.print(F("Manual Watering?"));
    lcd.setCursor(0, 1);
    lcd.print(F("SELECT=yes BACK=no"));
}

void displayDownloadConfirm() {
    lcd.setCursor(0, 0);
    lcd.print(F("Download Log"));
    lcd.setCursor(0, 1);
    lcd.print(F("SELECT=yes BACK=no"));
}

void displayResetConfirm() {
    lcd.setCursor(0, 0);
    lcd.print(F("Reset to"));
    lcd.setCursor(0, 1);
    lcd.print(F("Defaults? SEL/BCK"));
}

// ============================================================================
// BACKLIGHT AUTO-SLEEP
// ============================================================================

void updateBacklight() {
    unsigned long currentTime = millis();
    unsigned long backlightTimeoutMs = (unsigned long)config.backlightTimeout * 60000UL;

    if (backlightOn && (currentTime - lastActivity >= backlightTimeoutMs)) {
        backlightOn = false;
        lcd.noBacklight();
    }
}
