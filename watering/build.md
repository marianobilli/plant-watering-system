# Build Guide - Plant Watering System Phase 2

Step-by-step assembly instructions with corrected Arduino UNO R4 WiFi pin assignments and complete wiring diagrams.

## Overview

This guide covers:
1. ✅ Pin assignments and hardware connections
2. ✅ MOSFET pump control circuit wiring
3. ✅ Sensor DIS pin control (low-power mode)
4. ✅ Power supply dual-rail setup
5. ✅ Breadboard layout
6. ✅ Safety checklist

---

## Pin Assignments Summary

### Arduino UNO R4 WiFi Pins

| Arduino Pin | Function | Component | Notes |
|-------------|----------|-----------|-------|
| **A0** | Sensor ADC | Soil moisture sensor AOUT | Analog input, 14-bit (0-16383) |
| **D2** | Sensor DIS | Soil moisture sensor DIS pin | HIGH=disabled (sleep), LOW=enabled (active) |
| **D3** | Pump gate | MOSFET IRLZ44N gate | Digital output, logic-level (5V) |
| **D4** | Button UP | Pushbutton to GND | Navigation up (with internal pull-up) |
| **D5** | Button DOWN | Pushbutton to GND | Navigation down (with internal pull-up) |
| **D6** | Button SELECT | Pushbutton to GND | Menu select / confirm (with internal pull-up) |
| **D7** | Button BACK | Pushbutton to GND | Menu back / cancel (with internal pull-up) |
| **SDA (A4)** | LCD I2C Data | LCD SDA pin | I2C communication line |
| **SCL (A5)** | LCD I2C Clock | LCD SCL pin | I2C communication line |
| **GND (2 pins)** | Ground | 12V supply, pump, buttons, sensors | **CRITICAL: Common ground** |
| **5V** | Power | LCD VCC, sensor VCC | 5V Arduino rail |
| **USB-C** | Power | USB power adapter | Arduino power input |

---

## MOSFET Pump Control Circuit

### Components Required
- 1× IRLZ44N N-Channel Logic-Level MOSFET (TO-220 package)
- 1× 1N5819 Schottky Diode (20V, 1A)
- 2× 10kΩ resistors (¼W carbon film)
- 12V 2A power supply (barrel connector)
- Breadboard or perfboard for mounting

### MOSFET Pinout (TO-220 Package)
```
        [Metal tab facing away]

        GATE (left pin)
        DRAIN (middle pin, metal tab)
        SOURCE (right pin)
```

### Circuit Diagram

```
                         +12V Power Supply
                              |
                              |
                         [+] PUMP [-]
                              |
                              +--- 1N5819 Cathode (stripe |<)
                              |
                         Anode ---------|
                              |
                         DRAIN (MOSFET)
                              |
                          [IRLZ44N]
                              |
                         SOURCE
                              |
                            GND (12V supply)
                              |
                              |
    Arduino D3 ---[10kΩ]--- GATE          |
                      |                    |
                    [10kΩ]                 |
                      |                    |
                     GND (Arduino)---------|
                                 (COMMON GROUND!)
```

### Step-by-Step MOSFET Wiring

#### 1. MOSFET Orientation
- Place MOSFET on breadboard with **metal tab facing away from you**
- Left pin (toward you): GATE
- Middle pin (metal tab): DRAIN
- Right pin (away from you): SOURCE

#### 2. SOURCE to Ground
```
MOSFET SOURCE (right pin) → Breadboard GND rail (blue row)
                            ↓
                        Breadboard GND connects to:
                        - Arduino GND
                        - 12V supply GND (CRITICAL!)
```

#### 3. DRAIN to Pump Negative
```
MOSFET DRAIN (middle pin) → Pump NEGATIVE wire
Pump POSITIVE wire → +12V rail
```

#### 4. Flyback Diode (1N5819)
**CRITICAL: Orientation matters!**
```
1N5819 Cathode (stripe side |<) → +12V rail (pump positive rail)
1N5819 Anode (other end) → Pump NEGATIVE rail (same node as MOSFET DRAIN)
```

**Why:** Pump coil creates high-voltage spike when MOSFET turns off. Diode clamps it to protect the MOSFET.

#### 5. MOSFET GATE Control
```
Arduino D3 (5V output) → 10kΩ resistor → MOSFET GATE (left pin)
                                              ↓
                                         [10kΩ resistor]
                                              ↓
                                            GND
```

**Two 10kΩ resistors:**
- **Top resistor:** Current-limiting gate driver (protects Arduino output)
- **Bottom resistor:** Pull-down to GND (prevents floating gate, keeps pump OFF by default)

#### 6. Common Ground Connection
```
Arduino GND pin ----→ Breadboard GND rail
                           ↓
12V supply GND -----→ Same GND rail
                           ↓
All component GND connections share this rail
```

**Why:** Without common ground, the MOSFET gate signal is floating and won't work reliably.

---

## Soil Moisture Sensor Wiring

### Sensor Pinout
Most capacitive soil moisture sensors have 4 pins:
1. **VCC** → Arduino 5V or 3.3V (constant power)
2. **GND** → Arduino GND
3. **AOUT** → Arduino A0 (analog output)
4. **DIS** → Arduino D2 (disable pin, active LOW)

### DIS Pin Control (vs. VCC-Cycling)

**Monitor v1.5 approach (Phase 2 uses this):**
```
Sensor VCC → Arduino 3.3V or 5V (ALWAYS powered)
Sensor DIS → Arduino D2

Arduino firmware control:
  digitalWrite(SENSOR_DISABLE_PIN, LOW);  // DIS LOW = sensor ENABLED, reading active
  delay(1000);  // Wait for stabilization
  adcValue = analogRead(SENSOR_ADC_PIN);
  digitalWrite(SENSOR_DISABLE_PIN, HIGH);  // DIS HIGH = sensor DISABLED, low-power mode
```

**Advantages:**
- Extends sensor lifespan (low-power sleep mode when not reading)
- No sudden power spikes
- Faster wake-up than powering from GPIO

### Sensor Breadboard Connections
```
Sensor Pin | Arduino Pin | Breadboard
-----------|-------------|-------------
VCC        | 5V          | 5V rail
GND        | GND         | GND rail
AOUT       | A0          | Row with A0 wire
DIS        | D2          | Row with D2 wire
```

---

## LCD I2C Display Wiring

### LCD Pinout
Most 16x2 I2C LCD backpacks have 4 pins on the back:
1. **GND** → GND
2. **VCC** → 5V (Arduino supply)
3. **SDA** → A4 (I2C data)
4. **SCL** → A5 (I2C clock)

### Default I2C Address
- Try **0x27** first (most common)
- If display doesn't initialize, try **0x3F**
- To scan: load Arduino IDE → File → Examples → Wire → I2C_scanner

### Breadboard Connections
```
LCD Pin | Arduino Pin | Breadboard Connection
--------|-------------|----------------------
GND     | GND         | GND rail (blue)
VCC     | 5V          | 5V rail (red)
SDA     | A4 (SDA)    | Breadboard column with SDA wire
SCL     | A5 (SCL)    | Breadboard column with SCL wire
```

---

## Pushbutton Wiring (UP/DOWN/SELECT/BACK)

### Button Pinout (All buttons identical)
- Pin 1 → Arduino digital pin (D4, D5, D6, or D7)
- Pin 2 → GND

**No pull-up resistors needed!** Arduino UNO R4 has internal pull-up resistors.

### Breadboard Connections
```
Button | Arduino Pin | Breadboard Connection
-------|-------------|----------------------
UP     | D4          | One pin to D4 column, other pin to GND rail
DOWN   | D5          | One pin to D5 column, other pin to GND rail
SELECT | D6          | One pin to D6 column, other pin to GND rail
BACK   | D7          | One pin to D7 column, other pin to GND rail
```

### Button Logic (Active LOW)
```cpp
// In firmware:
pinMode(BTN_UP, INPUT_PULLUP);
if (digitalRead(BTN_UP) == LOW) {
    // Button is pressed
}
```

---

## Complete Breadboard Layout Diagram

```
                            ┌─────────────────────┐
                            │   +5V Rail (red)    │
                            │   +12V Rail         │
                            │   GND Rail (blue)   │
                            └─────────────────────┘
                                      │
                    ┌─────────────────┼─────────────────┐
                    │                 │                 │
                    │            [Arduino UNO R4]       │
                    │                                   │
                    │  USB-C: 5V power                  │
                    │  GND pins: → GND rail             │
                    │  D2: → Sensor DIS                 │
                    │  D3: → 10kΩ → MOSFET GATE        │
                    │  A0: → Sensor AOUT                │
                    │  D4-D7: → Pushbuttons             │
                    │  SDA: → LCD SDA                   │
                    │  SCL: → LCD SCL                   │
                    │  5V pin: → 5V rail                │
                    │                                   │
                    └───────────────────────────────────┘

    [Soil Sensor]           [MOSFET Circuit]       [LCD Display]
    VCC → 5V rail           D3 → 10kΩ → GATE       VCC → 5V
    GND → GND               ↓                       GND → GND
    DIS → D2                [10kΩ] → GND           SDA → A4
    AOUT → A0               DRAIN → Pump−          SCL → A5
                            SOURCE → GND

    [Pump + Diode]          [12V Power Supply]
    + → Diode cathode       + → Pump+
    − → MOSFET DRAIN        − → GND rail (common)
                               Diode anode

    [Pushbuttons]
    UP: D4 → GND
    DOWN: D5 → GND
    SELECT: D6 → GND
    BACK: D7 → GND
```

---

## Power Supply Setup

### Option 1: Dual Supply (Recommended for Development)

```
┌─────────────────────────────────────┐
│  Computer USB                       │
│  ↓                                  │
│  Arduino USB-C → 5V rail           │
│                                     │
│  12V power adapter                 │
│  Positive (+) → +12V rail          │
│  Negative (−) → GND rail           │
│                                     │
│  Arduino GND → GND rail (SAME!)    │
└─────────────────────────────────────┘
```

**Advantages:**
- Easy debugging (Arduino still powered via USB)
- Pump has dedicated 12V supply
- Can disconnect 12V without losing Arduino (for reprogramming)

**Step-by-step:**
1. Connect USB-C from computer to Arduino → Arduino lights up
2. Connect 12V adapter positive to +12V rail on breadboard
3. Connect 12V adapter negative to GND rail on breadboard
4. Verify: Arduino GND is on same breadboard rail as 12V adapter negative

### Option 2: Single 12V Supply (More Permanent)

```
┌──────────────────────────────────────┐
│  12V power adapter                   │
│  Positive (+) → +12V rail           │
│  Negative (−) → GND rail            │
│                                      │
│  Buck converter (12V → 5V)          │
│  Input: +12V rail                   │
│  Output: → Arduino VIN/5V pin       │
│  Ground: GND rail                   │
│                                      │
│  MOSFET pump circuit also uses      │
│  12V rail                           │
└──────────────────────────────────────┘
```

**Advantages:**
- Single power supply (more portable)
- No USB cable needed for continuous operation

**Disadvantages:**
- Need buck converter (~$5–15)
- Cannot reprogram Arduino while 12V is disconnected

---

## Pre-Power Checklist

**✓ BEFORE applying power, verify ALL of these:**

### Power Connections
- [ ] Arduino GND pin connected to breadboard GND rail
- [ ] 12V supply negative connected to same GND rail (COMMON GROUND!)
- [ ] No jumper wires bridging +5V and GND rails (shorts)
- [ ] No jumper wires bridging +12V and GND rails
- [ ] USB cable securely inserted into Arduino USB-C port

### MOSFET Circuit
- [ ] MOSFET SOURCE (right pin) → GND rail
- [ ] MOSFET DRAIN (middle pin) → Pump negative wire
- [ ] MOSFET GATE (left pin) ← 10kΩ resistor ← Arduino D3
- [ ] 10kΩ pull-down resistor from MOSFET GATE → GND
- [ ] Diode cathode (silver stripe) → +12V rail
- [ ] Diode anode → Pump negative (same node as MOSFET DRAIN)

### Sensor Wiring
- [ ] Sensor VCC → 5V rail
- [ ] Sensor GND → GND rail
- [ ] Sensor AOUT → Breadboard column connected to Arduino A0
- [ ] Sensor DIS → Breadboard column connected to Arduino D2

### LCD Wiring
- [ ] LCD VCC → 5V rail
- [ ] LCD GND → GND rail
- [ ] LCD SDA → Breadboard column connected to Arduino SDA (A4)
- [ ] LCD SCL → Breadboard column connected to Arduino SCL (A5)

### Buttons
- [ ] Button UP: one pin to D4 column, other to GND
- [ ] Button DOWN: one pin to D5 column, other to GND
- [ ] Button SELECT: one pin to D6 column, other to GND
- [ ] Button BACK: one pin to D7 column, other to GND

### Pump Hardware
- [ ] Pump tubing connected to intake/outlet correctly
- [ ] Pump electrical connectors soldered/crimped securely
- [ ] Pump NOT submerged in water (only intake tube should be)

---

## Power-On Test (No Firmware Yet)

1. **Connect USB** → Arduino LED should light up
2. **Turn on 12V supply** → Breadboard connections should be secure
3. **Briefly connect D3 to 5V** (use jumper wire):
   - Pump should activate (audible click or sound)
   - Remove jumper → Pump should stop
   - **If pump doesn't activate:**
     - Check MOSFET source to GND
     - Check pump power (12V at pump positive)
     - Check common ground (Arduino GND = 12V supply GND)

---

## After Upload: Firmware Test

Once `plant_watering.ino` is uploaded:

1. **Open Serial Monitor** (115200 baud) → should see startup messages
2. **Status screen** on LCD should show current moisture %
3. **Press SELECT** → Main menu opens
4. **Settings → Sensor Calibration:**
   - Place sensor in **air** → note ADC value (should be 10000–12000)
   - Place sensor in **water** → note ADC value (should be ~6000)
5. **Check moisture reading:**
   - In air: should show ~0%
   - In water: should show ~100%
6. **Manual watering:**
   - Settings → Manual Water → SELECT
   - Pump should activate and run for configured time
   - Moisture should increase slightly if sensor is in moist soil

---

## Troubleshooting

### Pump doesn't run at all
- ✓ Check 12V power supply (measure with multimeter: should read 12V)
- ✓ Check MOSFET orientation (metal tab facing away, SOURCE to GND)
- ✓ Test MOSFET with multimeter: GATE voltage should change when Arduino D3 goes HIGH
- ✓ Verify common ground (Arduino GND = 12V GND)

### Sensor reads 0% always
- ✓ Check `analogReadResolution(14)` is in firmware setup() — without it, readings are ~16× off
- ✓ Verify A0 connection (not A1, A2, etc.)
- ✓ Run sensor calibration wizard (Settings → Calibrate → Sensor)

### LCD doesn't display
- ✓ Check I2C address: File → Examples → Wire → I2C_scanner
- ✓ Verify SDA/SCL not swapped (SDA = A4, SCL = A5)
- ✓ Check LCD power (should be 5V, verify with multimeter)

### Arduino won't upload
- ✓ Check USB cable (data cable, not just power)
- ✓ Select correct board: Tools → Board → Arduino UNO R4
- ✓ Select correct COM port
- ✓ Try double-clicking RESET button on Arduino, then upload

### Buttons don't work
- ✓ Verify buttons are wired to D4-D7 with other pin to GND
- ✓ Check for solder bridges between button pins
- ✓ Test button with multimeter (should show continuity to GND when pressed)

---

## Safety Reminders

⚠️ **CRITICAL SAFETY NOTES:**

1. **Never connect pump directly to Arduino GPIO** — maximum GPIO current is ~20mA, pump draws 2A. Use MOSFET!
2. **Always use common ground** between Arduino and 12V supply. Without it, circuit won't work!
3. **Double-check diode orientation** — stripe (cathode) must point toward +12V. Wrong orientation = MOSFET destroyed!
4. **Test circuit with LED first** before connecting 12V pump (for extra safety)
5. **Waterproof sensor before use** — Encapsulate in resin or silicone boot to prevent water damage
6. **Don't power Arduino from 12V directly** — use USB or regulated 5V buck converter
7. **Check all connections twice** before applying power!

---

## Next Steps

1. ✅ Upload `plant_watering.ino` to Arduino
2. ✅ Run sensor calibration (air → water)
3. ✅ Run pump calibration (measure actual mL output)
4. ✅ Set watering thresholds (minThreshold = 55%, targetHumidity = 75%)
5. ✅ Configure watering amounts (initial: 50mL, increment: 10mL)
6. ✅ Configure soak times (after initial: 5min, between increments: 1min)
7. ✅ Place system near plant and test automatic watering

---

*Phase 2 Build Guide v1.0*
*Plant Watering System with Arduino UNO R4 WiFi*
*Generated 2026-02-28*
