P# POC Wiring Guide - Arduino UNO R4 WiFi with MOSFET Pump Control

## Pin Assignments

### Arduino UNO R4 WiFi Pin Allocation

| Function | Arduino Pin | Notes |
|----------|------------|-------|
| Soil Moisture Sensßor (ADC) | A0 | Analog input |
| Sensor Power Control | D2 | Powers sensor only during readings |
| Pump MOSFET Control | D3 | Controls IRLZ44N gate |
| LCD SDA (I2C Data) | SDA (A4) | Default I2C SDA |
| LCD SCL (I2C Clock) | SCL (A5) | Default I2C SCL |
| Button UP | D4 | With internal pull-up |
| Button DOWN | D5 | With internal pull-up |
| Button SELECT | D6 | With internal pull-up |
| Button BACK | D7 | With internal pull-up |
| SD Card MOSI (SPI) | D11 | SPI data out |
| SD Card MISO (SPI) | D12 | SPI data in |
| SD Card SCK (SPI) | D13 | SPI clock |
| SD Card CS (Chip Select) | D10 | SPI chip select |

**Advantages of Arduino UNO R4 WiFi:**
- **No ADC/WiFi conflict** (unlike ESP32's ADC2 limitation)
- All analog pins (A0-A5) work simultaneously with WiFi
- 5V logic works directly with LCD and most sensors
- Built-in pull-up resistors on all digital pins
- More stable WiFi reconnection than ESP32

---

## MOSFET Pump Control Circuit

### Components:
- IRLZ44N N-Channel Logic-Level MOSFET
- 1N5819 Schottky diode
- 2× 10kΩ resistors (gate resistor + pull-down)
- 12V peristaltic pump

### Circuit Diagram:

```
                                   +12V Power Supply
                                         |
                                         |
                                    [+] PUMP [-]
                                         |
                                         +--- Diode Cathode (|<)
                                         |
                    Diode Anode ---------|
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
ESP32 GPIO27 ---[10kΩ]--- GATE          |
                   |                     |
                 [10kΩ]                  |
                   |                     |
                  GND (ESP32) -----------+ (COMMON GROUND!)
```

### Step-by-Step Wiring:

1. **MOSFET Orientation** (TO-220 package, metal tab facing away):
   - Left pin: GATE
   - Middle pin: DRAIN
   - Right pin: SOURCE

2. **SOURCE to Ground:**
   - MOSFET SOURCE → GND (breadboard ground rail)

3. **DRAIN to Pump:**
   - MOSFET DRAIN → Pump NEGATIVE wire
   - Pump POSITIVE wire → 12V+ (power supply positive)

4. **Flyback Diode (1N5819):**
   - CATHODE (silver stripe) → Pump POSITIVE (12V+)
   - ANODE → Pump NEGATIVE (same node as MOSFET DRAIN)
   - **Critical:** Diode orientation matters! Wrong direction = no protection

5. **MOSFET GATE Control:**
   - Arduino D3 → 10kΩ resistor → MOSFET GATE
   - MOSFET GATE → 10kΩ resistor → GND (pull-down, prevents floating)

6. **Common Ground Connection:**
   - Arduino GND → 12V Power Supply GND (shared ground rail on breadboard)
   - **Critical:** Without common ground, MOSFET won't work!

### MOSFET Control Code:
```cpp
// Pin definitions
#define PUMP_PIN 3  // Arduino D3

// Setup
pinMode(PUMP_PIN, OUTPUT);
digitalWrite(PUMP_PIN, LOW);  // Pump OFF by default

// Turn pump ON
digitalWrite(PUMP_PIN, HIGH);

// Turn pump OFF
digitalWrite(PUMP_PIN, LOW);
```

---

## Soil Moisture Sensor Wiring

### GPIO-Powered Sensor (Recommended):

```
Arduino D2 (Power) → Sensor VCC
Arduino GND        → Sensor GND
Arduino A0 (ADC)   → Sensor AOUT (analog output)
```

### Reading Code:
```cpp
// Pin definitions
#define SENSOR_POWER_PIN 2  // Arduino D2
#define SENSOR_ADC_PIN A0   // Arduino A0

// Power on sensor
pinMode(SENSOR_POWER_PIN, OUTPUT);
digitalWrite(SENSOR_POWER_PIN, HIGH);
delay(200);  // Warmup time for stable reading

// Read sensor (with averaging to reduce noise)
long sum = 0;
const int numSamples = 20;
for (int i = 0; i < numSamples; i++) {
    sum += analogRead(SENSOR_ADC_PIN);
    delay(10);
}
int avgReading = sum / numSamples;

// Power off sensor
digitalWrite(SENSOR_POWER_PIN, LOW);

// Convert to percentage (calibrate these values!)
// Arduino UNO R4: ADC is 14-bit (0-16383), not 10-bit like classic Arduino
int moisturePercent = map(avgReading, 12400, 6000, 0, 100);  // Typical for 14-bit ADC
moisturePercent = constrain(moisturePercent, 0, 100);
```

**Important Notes for Arduino UNO R4 WiFi:**
- **14-bit ADC:** UNO R4 has 14-bit resolution (0-16383), not 10-bit (0-1023) like classic Arduino
- **Calibration required:** Example values (12400 dry, 6000 wet) will vary by sensor
- **Waterproof sensor first!** See BOM for methods
- **No ADC/WiFi conflict:** Unlike ESP32, all analog pins work with WiFi active

---

## LCD Display Wiring (I2C)

### Connections:

```
Arduino 5V  → LCD VCC
Arduino GND → LCD GND
Arduino SDA (A4) → LCD SDA
Arduino SCL (A5) → LCD SCL
```

**Notes:**
- Arduino UNO R4 WiFi has dedicated SDA/SCL pins (also labeled A4/A5)
- **Use 5V power** for LCD - Arduino UNO R4 has 5V logic, perfect for LCD backlight
- Default I2C address: 0x27 or 0x3F (scan with I2C scanner if unsure)
- No level shifting needed - both Arduino and LCD use 5V logic

### Test Code:
```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Address, columns, rows

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Plant Watering");
    lcd.setCursor(0, 1);
    lcd.print("System v1.0");
}
```

---

## Button Wiring (with Internal Pull-ups)

### Connections (all buttons use same pattern):

```
Button Pin 1 → Arduino Digital Pin (D4, D5, D6, or D7)
Button Pin 2 → GND
```

**No external resistors needed!** Arduino UNO R4 WiFi has internal pull-up resistors on all digital pins.

### Button Code:
```cpp
// Pin definitions
#define BTN_UP     4  // Arduino D4
#define BTN_DOWN   5  // Arduino D5
#define BTN_SELECT 6  // Arduino D6
#define BTN_BACK   7  // Arduino D7

// Setup (enable internal pull-ups)
pinMode(BTN_UP,     INPUT_PULLUP);
pinMode(BTN_DOWN,   INPUT_PULLUP);
pinMode(BTN_SELECT, INPUT_PULLUP);
pinMode(BTN_BACK,   INPUT_PULLUP);

// Reading (active LOW - button pressed = LOW)
if (digitalRead(BTN_UP) == LOW) {
    // UP button pressed
    delay(50);  // Debounce
    while (digitalRead(BTN_UP) == LOW);  // Wait for release
    delay(50);  // Debounce
}
```

---

## SD Card Module Wiring (SPI)

### Connections:

```
Arduino 5V      → SD Module VCC
Arduino GND     → SD Module GND
Arduino D11     → SD Module MOSI
Arduino D12     → SD Module MISO
Arduino D13     → SD Module SCK
Arduino D10     → SD Module CS
```

**Notes:**
- Arduino UNO R4 WiFi has hardware SPI on pins 11, 12, 13
- Chip Select (CS) can be any digital pin, D10 is standard
- SD card module should work with both 3.3V and 5V logic (verify your module)
- Format SD card as FAT32 for best compatibility
- Use 4GB-32GB cards (larger cards may need exFAT formatting)

### Test Code:
```cpp
#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 10

void setup() {
    Serial.begin(115200);

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized successfully");

    // Create test file
    File testFile = SD.open("test.txt", FILE_WRITE);
    if (testFile) {
        testFile.println("Hello from Arduino!");
        testFile.close();
        Serial.println("Test file written");
    }
}
```

**Monitor Mode Usage:**
- Monitor mode logs moisture readings every 5 minutes to SD card
- Data saved in CSV format: `moisture_log_YYYYMMDD.csv`
- Each entry: timestamp (ms), moisture (%), raw ADC value
- Used to understand plant's natural watering cycles before automating

---

## Power Supply Connections

### During POC Development (Recommended):

**Option 1: Dual Supply (Easiest for debugging)**
- ESP32: USB cable from computer (5V via USB)
- Pump: 12V 2A power supply
- **Critical:** Connect grounds together!

```
Computer USB → ESP32 (USB-C port)
12V Adapter (+) → Pump positive
12V Adapter (-) → Breadboard GND rail
ESP32 GND pin → Breadboard GND rail (SAME rail as 12V GND!)
```

**Option 2: Single 12V Supply (More portable)**
- 12V supply → Buck converter → 5V output → ESP32 VIN/5V pin
- 12V supply → Pump (via MOSFET)
- Shared ground

---

## Complete Breadboard Layout (Text Diagram)

```
                                    +12V Rail
                                        |
    ┌──────────────────────────────────┴────────────────────────┐
    │                                                            │
    │  [Pump]                                                    │
    │    |                                                       │
    │   Diode (1N5819)                                          │
    │    |                                                       │
    │  DRAIN                                                     │
    │    |                                                       │
    │ [IRLZ44N]                                                 │
    │    |                                                       │
    │  SOURCE                                                    │
    │    |                                                       │
    ├────┴───────────────────────────────────────────────────────┤
    │                      GND Rail (COMMON GROUND)              │
    └──┬─────────┬────────┬─────────┬─────────┬─────────────────┘
       │         │        │         │         │
     ESP32    Buttons  Sensor   MOSFET   12V Supply
      GND               GND    Pull-down    GND
                                10kΩ
```

---

## Pin Connection Checklist

Before powering on, verify:

- [ ] ESP32 GND and 12V GND are connected (common ground)
- [ ] MOSFET SOURCE connected to GND
- [ ] MOSFET DRAIN connected to pump negative
- [ ] MOSFET GATE has 10kΩ pull-down to GND
- [ ] ESP32 GPIO27 → 10kΩ → MOSFET GATE
- [ ] 1N5819 diode cathode (stripe) to +12V
- [ ] 1N5819 diode anode to pump negative
- [ ] Sensor powered from GPIO26 (not always-on 3.3V)
- [ ] Sensor analog output to GPIO34 (ADC1 pin)
- [ ] LCD SDA to GPIO21, SCL to GPIO22
- [ ] All 4 buttons to GPIO with other pin to GND
- [ ] SD card MOSI to D11, MISO to D12, SCK to D13, CS to D10
- [ ] SD card VCC to 5V, GND to GND
- [ ] No shorts between power and ground
- [ ] Pump not connected to ESP32 pins directly (must use MOSFET!)

---

## Safety Notes

1. **Never connect pump directly to ESP32 GPIO** - it will destroy the ESP32!
2. **Always connect common ground** between ESP32 and 12V supply
3. **Double-check diode orientation** - stripe (cathode) to positive
4. **Test MOSFET first** with LED instead of pump to verify circuit
5. **Waterproof sensor before use** - water damage is permanent
6. **Don't power ESP32 from 12V directly** - use USB or buck converter to 5V

---

## Troubleshooting

### Pump doesn't run:
- Check common ground connection
- Verify MOSFET orientation (SOURCE to GND, DRAIN to pump)
- Test GPIO27 voltage: should be 3.3V when HIGH, 0V when LOW
- Check 12V power supply is connected
- Verify MOSFET is logic-level (IRLZ44N, not IRF44N)

### Sensor readings incorrect:
- Verify sensor is powered (GPIO26 = HIGH)
- Check GPIO34 connection (must be ADC1 pin)
- Calibrate sensor in air and water
- Ensure sensor is waterproofed

### LCD doesn't display:
- Check I2C address (scan with I2C scanner sketch)
- Verify SDA/SCL connections (not swapped)
- Check LCD power supply voltage
- Adjust contrast potentiometer on LCD backpack

### ESP32 won't boot:
- Disconnect GPIO0, 2, 12, 15 during boot
- Check USB cable (data + power, not just power)
- Install CH340 drivers if needed
