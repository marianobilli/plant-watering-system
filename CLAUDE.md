# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

An intelligent, research-driven automated plant watering system that addresses common DIY irrigation failures through scientifically-informed design. The project is split into two phases:

- **Phase 1 (POC)**: Single-plant system using Arduino UNO R4 WiFi - **CURRENT PHASE** ✅ Firmware complete
- **Phase 2 (Production)**: Multi-plant (6x) system with ESP32, cloud connectivity, and mobile app

## Development Environment

### Arduino Development (Phase 1)
- **IDE**: Arduino IDE 2.0+ or PlatformIO
- **Board**: Arduino UNO R4 WiFi (Renesas RA4M1, 14-bit ADC, 5V logic)
- **Main file**: `src/poc/plant_watering_poc.ino`

### Required Libraries
Install via Arduino IDE Library Manager:
- **LiquidCrystal I2C** by Frank de Brabander (v1.1.2+)
- Wire, EEPROM, SPI, SD (built-in)

### Upload Process
```bash
# 1. Install board support: Tools → Boards Manager → "Arduino UNO R4 Boards"
# 2. Select board: Tools → Board → Arduino UNO R4 WiFi
# 3. Select port: Tools → Port → (your Arduino port)
# 4. Upload: Click Upload button or Ctrl+U
```

### Serial Monitor
Open at **115200 baud** for debugging output (ADC readings, watering cycles, errors).

## Core Architecture

### Two-Phase Watering Cycle

The system's central design pattern prevents the #1 cause of DIY watering failures (constant over-watering):

**Phase 1: Watering Phase** (moisture ≤ Min Threshold)
- Waters in small increments (default: 50mL)
- Waits `soakTime` (default: 5 min) for capillary action
- Re-measures moisture after each increment
- Continues until `targetHumidity` (default: 80%) reached
- Failsafe: stops if moisture doesn't increase (detects pump/sensor failures)

**Phase 2: Dry Cycle Phase** (moisture > Min Threshold)
- Checks moisture every `checkInterval` (default: 6 hours)
- Does NOT water unless moisture drops to `minThreshold` (default: 10%)
- Mimics natural rainfall patterns

See `WATERING_LOGIC.md` for detailed explanation with examples.

### Operating Modes

**Watering Mode** (default): Automatic watering with dry cycle logic
**Monitor Mode**: Passive data logging to SD card (CSV format, every 5 minutes) - use this to learn your plant's natural cycles before automating

### GPIO-Powered Sensor Pattern

Critical for sensor longevity (extends life from 3-6 months → 1-2+ years):

```cpp
// Power sensor only during readings (seconds per hour vs. continuous)
digitalWrite(SENSOR_POWER_PIN, HIGH);
delay(200);  // Warmup
int reading = analogRead(SENSOR_ADC_PIN);
digitalWrite(SENSOR_POWER_PIN, LOW);  // Power off immediately
```

### Configuration Storage

All settings persist in EEPROM:
- Operating mode, target/min humidity, water amount
- Check interval, soak time
- Sensor calibration (dry/wet ADC values)
- Pump flow rate

## Hardware Constraints & Best Practices

### Arduino UNO R4 Specifics
- **14-bit ADC** (0-16383 range, not 0-1023 like classic Arduino)
- Typical sensor calibration: ~12400 (air/dry), ~6000 (water/wet)
- Must calibrate for accurate readings: Main Menu → Calibrate → Sensor

### ESP32 Considerations (Phase 2)

**CRITICAL ADC Constraint:**
- ADC2 pins (GPIO0, 2, 4, 12-15, 25-27) **CANNOT** be used while WiFi is active
- **Always use ADC1 pins (GPIO32-39) for moisture sensors**
- Sensor readings will fail or return garbage if using ADC2 with WiFi

**WiFi Does Not Auto-Reconnect:**
ESP32 will NOT automatically reconnect after signal loss or router reboot. Must implement explicit reconnection logic in loop():

```cpp
if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(5000);
}
```

**Additional WiFi Stability:**
- Use static IP (prevents DHCP lease expiration disconnects)
- Set router to 20MHz channel width (not 40MHz)
- Keep signal strength better than -70 dBm

**Deep Sleep Bug:**
- After random wake cycles, ESP32 may sleep and never wake
- Fix: add 500ms delay after waking before reading RTC memory
- Use `ULL` suffix for long durations: `esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 1000000ULL);`

### Mechanical Best Practices

**Reservoir Placement:**
- Keep water reservoir **BELOW plant level** to prevent siphoning
- If above plants, water continues draining after pump stops

**Tubing:**
- Use **opaque tubing** (transparent grows algae rapidly in light)
- 5-6mm inner diameter recommended (balances flow vs. clogging)
- Flush periodically; clean with vinegar when clogged

**Distribution Method:**
- Closed-end hose with drilled holes > Y-splitters
- Splitters cause uneven distribution (some plants get all water, others get air)

### Sensor Longevity

**Waterproofing (CRITICAL):**
- Apply 2 coats of clear nail polish to sensor PCB before use
- Without this, sensors corrode and fail within weeks

**Power Management:**
- Power sensors from GPIO, not constant power
- Average 20+ readings to reduce noise

## Key Files & Documentation

### Essential Reading
- `README.md` - Project overview, quick start, common mistakes
- `PROJECT_OVERVIEW.md` - Complete specs, ESP32 constraints, maintenance schedule
- `WATERING_LOGIC.md` - Detailed two-phase algorithm explanation with examples
- `lessons_learned.md` - Research findings from 50+ failed DIY projects

### Hardware Guides
- `BOM_POC.md` - Phase 1 parts list with links (~550 PLN / ~$140 USD)
- `BOM_FINAL.md` - Phase 2 parts list (6-plant system, ~$140-240)
- `WIRING_POC.md` - Complete pin assignments and MOSFET circuit
- `MENU_DESIGN.md` - 16×2 LCD menu structure

### Firmware
- `src/poc/plant_watering_poc.ino` - Phase 1 complete firmware (ready to upload)
- `src/poc/README.md` - Upload instructions, calibration, troubleshooting
- `src/final/` - Phase 2 (future multi-plant ESP32 system)

## Common Calibration Tasks

### Sensor Calibration (REQUIRED before first use)
1. Navigate: Main Menu → Calibrate → Sensor
2. Hold sensor in **air** → Press SELECT (records dry value)
3. Put sensor in **water** → Press SELECT (records wet value)
4. Values saved to EEPROM automatically

### Pump Calibration (for accurate dosing)
1. Navigate: Main Menu → Calibrate → Pump
2. Prepare measuring cup
3. Run pump until measurable water collected (e.g., 100mL)
4. Enter amount collected via menu
5. System calculates mL/s rate

Typical pump rate: ~1.67 mL/s (100 mL/min)

## Troubleshooting Quick Reference

**Sensor reads 0% or 100% constantly** → Run calibration wizard
**Pump doesn't run** → Check common ground (Arduino GND → 12V GND), verify MOSFET is IRLZ44N (not IRF44N)
**Moisture doesn't increase after watering** → Check pump actually pumps water, verify tubing connections
**System doesn't water automatically** → Check Mode is "Watering" (not "Monitor"), verify moisture ≤ Min Threshold
**SD card fails** → Format as FAT32 (not exFAT), try different card, system auto-switches to Watering mode on SD failure
**LCD shows nothing** → Try I2C address 0x3F instead of 0x27 (line 36 in .ino)

## Architectural Notes for Future Development

### Phase 2 Multi-Plant Architecture
- **6 independent stations**: Each with own sensor, pump, configuration
- **Shared resources**: Water reservoir, power supply, WiFi connection
- **Water level monitoring**: Ultrasonic (HC-SR04) or laser ToF (VL53L0X)
- **Cloud platform**: Arduino IoT Cloud (primary candidate)
- **Mobile app**: React Native or Flutter (multi-device support, per-plant config)
- **Failsafes**: Auto-disable pumps on low water, alert on sensor failures

### Key Design Principles
1. **Fail-safe by default**: Stop watering if moisture doesn't increase
2. **Configurable but opinionated**: Sane defaults based on research
3. **Sensor longevity first**: GPIO-powered, averaged readings, proper waterproofing
4. **Natural watering cycles**: Dry cycle logic prevents root rot
5. **Moisture propagation awareness**: Soak time prevents flooding

## Important Context When Making Changes

### Do Not Break
- Dry cycle logic (core to preventing over-watering)
- Soak time waits (critical for capillary action)
- GPIO sensor power control (extends sensor life 4-8x)
- Failsafe protection (stops if moisture doesn't increase)
- EEPROM persistence (configuration survives power loss)

### ADC Range Awareness
- Arduino UNO R4 uses **14-bit ADC** (0-16383), not 10-bit (0-1023)
- ESP32 uses 12-bit ADC (0-4095)
- Always check `analogRead()` range when porting between platforms

### Timing Considerations
- Soak time is in **minutes**, not seconds (moisture propagation is slow)
- Check interval is in **hours** (plants don't need constant monitoring)
- ADC warmup delay is 200ms (capacitive sensors need stabilization time)

## Phase 1 Current Status

✅ **Complete and ready to upload:**
- Firmware with full menu system
- Two operating modes (Watering + Monitor)
- Sensor and pump calibration wizards
- EEPROM configuration persistence
- SD card data logging (Monitor mode)
- Failsafe protection
- Serial debugging output

**Next milestone:** Hardware assembly and real-world testing (1-2 weeks monitoring)

---

## Monitoring Variant (Phase 1b)

**Purpose:** Simplified data-collection system to understand plant watering needs *before* implementing automatic irrigation.

### Overview

The monitoring variant is a stripped-down version of the POC system that focuses purely on data collection:

- **No watering functionality** (pump, MOSFET circuit removed)
- **EEPROM-based logging** (no SD card needed)
- **Circular buffer storage** (automatic overwrite of oldest data)
- **USB Serial CSV download** (no card reader needed)
- **2-week data capacity** (1,344 entries @ 15-min intervals)
- **~$43 cost** (vs $65 for POC with pump/SD card)

**Key insight:** Most DIY watering failures come from guessing thresholds instead of measuring actual plant behavior. This variant provides 2 weeks of quantitative moisture data to configure the POC automatic watering system correctly.

### When to Use Each Variant

| Use Case | Variant | Why |
|----------|---------|-----|
| **New to plant care** | Monitor first | Learn your plant's natural dry-down rate |
| **Unknown plant species** | Monitor first | Different plants have vastly different water needs |
| **Testing new soil mix** | Monitor first | Soil composition affects moisture retention |
| **Ready to automate** | POC variant | Already know optimal thresholds from monitoring |
| **Experienced gardener** | POC variant | Understand plant needs, want automation |

**Recommended workflow:**
1. Build **monitor variant** → collect 2 weeks of data
2. Analyze CSV → determine optimal `minThreshold`, `targetHumidity`, `soakTime`
3. Upgrade to **POC variant** → add pump + MOSFET circuit
4. Configure automatic watering with measured thresholds
5. Test in Monitor mode (dry run) before enabling Watering mode

### Hardware Differences

**Monitor Variant Removes:**
- ❌ 12V peristaltic pump (~$10)
- ❌ IRLZ44N MOSFET + resistors + diode (~$1)
- ❌ SD card module + card (~$5)
- ❌ 12V 2A power supply (~$7)

**Monitor Variant Uses:**
- ✅ Arduino UNO R4 WiFi (8KB EEPROM for storage)
- ✅ Capacitive soil moisture sensor (A0)
- ✅ 16×2 I2C LCD (status + menu)
- ✅ 4× buttons (UP, DOWN, SELECT, BACK)
- ✅ USB cable for power + CSV download

**Pin Usage:**
- D2: Sensor power control (GPIO-powered)
- A0: Sensor ADC input
- D4-D7: Button inputs (internal pullups)
- SDA/SCL (A4/A5): I2C LCD
- **D3, D10-D13: Unused** (no pump, no SD card)

### Firmware Location & Features

**Main file:** `src/monitor/soil_humidity_monitor.ino` (~900 lines)

**Core Features:**
- **Circular buffer:** 1,344 entries × 4 bytes in EEPROM (5,376 bytes)
- **Configuration persistence:** 32-byte header (calibration, settings, pointers)
- **Auto-logging:** Every 15 minutes (user-adjustable 1-60 min)
- **CSV download:** Via Serial @ 115200 baud with progress bar
- **Sensor calibration:** Wizard for dry/wet ADC values
- **Settings menu:** Adjust log interval without reflashing
- **System info:** Entry count, days of data, firmware version
- **Clear data:** Erase all logs with confirmation dialog
- **Backlight auto-sleep:** 1-minute timeout to save power

**EEPROM Memory Map:**
```
0-31    : Configuration header (magic, version, calibration, pointers)
32-5407 : Data section (1,344 × 4-byte entries)
5408-8191: Unused (reserved for future expansion)
```

**Entry Format (4 bytes):**
- Byte 0: Moisture % (0-100)
- Byte 1-2: Raw ADC value (0-16383, 14-bit)
- Byte 3: Status flags (sensor error, out of range)

### Storage Specifications

**Default Configuration (15-minute intervals):**
- **Capacity:** 1,344 entries
- **Duration:** Exactly 14.0 days (336 hours)
- **EEPROM usage:** 66% (2,784 bytes unused)
- **Logging duty cycle:** 0.02% (sensor powered 200ms per 15 min)

**Alternative Intervals:**

| Interval | Entries | Duration | Use Case |
|----------|---------|----------|----------|
| 10 min | 1,344 | 9.3 days | Fast data collection |
| 15 min | 1,344 | 14.0 days | **Default (optimal)** |
| 30 min | 1,344 | 28.0 days | Long-term trends |
| 60 min | 1,344 | 56.0 days | Extended monitoring |

**EEPROM Endurance:**
- 100K write cycles per address (Arduino UNO R4 WiFi spec)
- Each address written once every 14 days (circular buffer)
- Effective lifespan: ~39,600 years (not a concern)

### CSV Download Format

**Header Metadata:**
```csv
# Soil Humidity Monitor Data Export
# Firmware Version: 1.0
# Total Entries: 1234
# Log Interval: 15 minutes
# Sensor Calibration: Dry=12400, Wet=6000
# Buffer Status: NOT_WRAPPED
#
Entry,Timestamp_Hours,Timestamp_Minutes,Moisture_%,Raw_ADC,Flags
```

**Data Rows:**
```csv
1,0.00,0,45,8500,0x00
2,0.25,15,46,8450,0x00
3,0.50,30,47,8400,0x00
```

**Columns Explained:**
- **Entry:** Sequential number (1-1344)
- **Timestamp_Hours:** Hours since monitoring started (calculated from entry index)
- **Timestamp_Minutes:** Total minutes since start (Entry × Interval)
- **Moisture_%:** Calibrated moisture (0-100%, calculated from ADC)
- **Raw_ADC:** 14-bit ADC value (0-16383, for debugging calibration)
- **Flags:** Status byte (0x00 = normal, 0x01 = sensor error)

**Download Method:**
1. Arduino IDE → Tools → Serial Monitor (115200 baud)
2. Arduino: Main Menu → Download Data → SELECT
3. Copy all text from Serial Monitor
4. Paste into text editor → Save as `.csv`
5. Import to Excel/Python for analysis

### Data Analysis Workflow

**Step 1: Import CSV to Excel/Python**
```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('moisture_data.csv', comment='#')
plt.plot(df['Timestamp_Hours'], df['Moisture_%'])
plt.xlabel('Time (hours)')
plt.ylabel('Soil Moisture (%)')
plt.title('2-Week Moisture Monitoring')
plt.show()
```

**Step 2: Calculate Key Metrics**

| Metric | Calculation | Purpose |
|--------|-------------|---------|
| **Dry-down rate** | (Moisture_start - Moisture_end) / Hours | How fast moisture drops naturally |
| **Stress threshold** | Moisture % when plant wilts | Minimum safe moisture level |
| **Optimal watering threshold** | Stress threshold + 5-10% buffer | When to start watering |
| **Post-watering peak** | Maximum moisture after manual watering | Target humidity for automatic watering |
| **Moisture propagation time** | Minutes until sensor sees moisture increase | Soak time for capillary action |

**Step 3: Configure POC Automatic Watering**

Use measured values to set POC parameters:

```cpp
// Example configuration based on monitoring data
config.minThreshold = 20;        // Plant showed stress at 15%, add 5% buffer
config.targetHumidity = 75;      // Manual watering reached 78%, round to 75%
config.waterAmount = 50;         // Small increments for safety
config.soakTime = 10;            // Moisture appeared in 8 minutes, add 2 min buffer
config.checkInterval = 12;       // Dried from 75% → 20% in 5 days, check every 12h
```

### Upgrading from Monitor to POC

**To add automatic watering functionality:**

1. **Keep existing wiring** (sensor, LCD, buttons)
2. **Add pump components:**
   - 12V peristaltic pump
   - IRLZ44N MOSFET + 2× 10kΩ resistors + 1N5819 diode
   - 12V 2A power supply
3. **Wire MOSFET circuit:**
   - Arduino D3 → 10kΩ → MOSFET gate
   - MOSFET gate → 10kΩ → GND (pull-down)
   - MOSFET drain → Pump negative
   - Pump positive → 12V+
   - 1N5819 diode across pump (cathode to +12V)
   - **Critical:** Arduino GND → 12V GND (common ground!)
4. **Flash POC firmware:** `src/poc/plant_watering_poc.ino`
5. **Calibrate pump:** Main Menu → Calibrate → Pump
6. **Configure watering params:** Use values from monitoring data analysis
7. **Test in Monitor mode** (dry run, no actual watering)
8. **Enable Watering mode** when confident

**See `WIRING_POC.md` for detailed MOSFET circuit diagram.**

### Files & Documentation

**Complete monitoring variant documentation:**

| File | Purpose |
|------|---------|
| `src/monitor/soil_humidity_monitor.ino` | Main firmware (~900 lines) |
| `src/monitor/README.md` | Setup guide, calibration, CSV download, troubleshooting |
| `src/monitor/BOM_MONITOR.md` | Parts list (~$43 USD) |
| `src/monitor/soil_humidity_monitor.kicad_sch` | KiCad schematic (s-expression format) |

**Key schematic references:**
- `WIRING_POC.md` lines 113-157: Sensor wiring (GPIO-powered pattern)
- `WIRING_POC.md` lines 160-192: LCD I2C connections
- `WIRING_POC.md` lines 196-228: Button wiring with internal pullups

### Common Monitoring Variant Questions

**Q: Why EEPROM instead of SD card?**
- Simpler (no SPI wiring, no FAT filesystem)
- More reliable (no card corruption issues)
- USB-powered (no external supply needed)
- Automatic circular buffer (no manual file management)
- Cheaper (~$5 savings)

**Q: Why 15-minute intervals?**
- Optimal for exactly 14-day storage in 8KB EEPROM
- Balances data resolution with EEPROM endurance
- Sensor powered 0.02% of time (extends life 4-8×)
- User can adjust 1-60 minutes via Settings menu

**Q: What if I need more than 2 weeks of data?**
- Download CSV after 2 weeks → Clear data → Restart logging
- Increase interval to 30 or 60 minutes (extends to 28-56 days)
- Concatenate multiple CSV files in Excel/Python for long-term analysis

**Q: Can I use monitoring variant permanently?**
- Yes! Some users prefer manual watering with data-driven decisions
- Monitor variant provides ongoing moisture trends without automation complexity
- Lower failure risk (no pump/MOSFET failure modes)

**Q: How do I know when calibration is wrong?**
- CSV shows moisture outside 0-100% range
- Sensor reads same value in air vs. water
- Moisture percentage doesn't match visual soil dryness
- **Fix:** Re-run calibration wizard (Main Menu → Calibrate Sensor)

### Monitoring Variant Troubleshooting

**Problem:** Entry count stuck at 0
- **Cause:** Logging interval not set or EEPROM header corrupted
- **Fix:** Main Menu → Settings → Set interval → Main Menu → Reset to Defaults

**Problem:** CSV download shows garbage data
- **Cause:** Wrong Serial Monitor baud rate (not 115200)
- **Fix:** Set Serial Monitor to 115200 baud, re-download

**Problem:** Moisture reads 0% or 100% constantly
- **Cause:** Sensor not calibrated or wrong ADC pin
- **Fix:** Verify A0 connection, run calibration wizard

**Problem:** LCD shows "Log: 1344/1344" immediately after Clear Data
- **Cause:** Write pointer not reset (firmware bug)
- **Fix:** Power cycle Arduino, re-upload firmware if persists

**For complete troubleshooting guide, see `src/monitor/README.md`**

**Next milestone:** Hardware assembly and real-world testing (1-2 weeks monitoring)
