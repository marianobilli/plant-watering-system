# Soil Humidity Monitor - User Guide

**2-Week EEPROM Data Logger for Plant Moisture Analysis**

A simplified monitoring-only variant designed to help you understand your plant's natural watering cycles before implementing automatic irrigation. Logs 1,344 measurements over 14 days to Arduino UNO R4 WiFi's 8KB EEPROM.

---

## Quick Start (5-Minute Setup)

1. **Waterproof sensor** (2 coats clear nail polish, dry 24h)
2. **Wire components** per schematic (20 connections)
3. **Upload firmware** via Arduino IDE (30 seconds)
4. **Calibrate sensor** (Main Menu → Calibrate → Run Wizard)
   - Watch live ADC values, wait for stabilization, press SELECT
5. **Insert in soil** → Auto-logging begins!
   - Status screen shows moisture % and raw ADC value

**After 2 weeks:** Download CSV via Serial Monitor (115200 baud) → Analyze in Excel/Python

### Key Features (v1.1)

- **Real-time ADC monitoring:** Status screen shows both moisture % and raw sensor value
- **Live calibration feedback:** Watch ADC values stabilize during calibration (updates every 0.5s)
- **Manual calibration editing:** Fine-tune dry/wet values without re-running calibration wizard
- **Enhanced diagnostics:** System Info displays calibration values and current settings
- **Improved UX:** Better visual feedback and more control over sensor configuration

---

## Table of Contents

1. [Hardware Assembly](#hardware-assembly)
2. [Firmware Upload](#firmware-upload)
3. [First-Time Setup](#first-time-setup)
4. [Operating the System](#operating-the-system)
5. [CSV Download Instructions](#csv-download-instructions)
6. [Data Analysis Workflow](#data-analysis-workflow)
7. [Troubleshooting](#troubleshooting)
8. [Technical Specifications](#technical-specifications)

---

## Hardware Assembly

### Components Required

See [`BOM_MONITOR.md`](BOM_MONITOR.md) for complete parts list (~$42 USD).

**Core items:**
- Arduino UNO R4 WiFi
- **Capacitive soil moisture sensor** (Cytron MAKER-SOIL-MOISTURE or compatible)
  - **Manufacturer:** Cytron Technologies
  - **Model:** MAKER-SOIL-MOISTURE
  - **Datasheet:** [PDF](https://download.kamami.pl/p1178856-Dokumentacja_MAKER-SOIL-MOISTURE%20Datasheet.pdf)
  - **Interface:** 4-wire Grove connector (VCC, GND, OUT, DIS)
  - **Output voltage:** 1.0V - 5.2V (higher = drier soil)
  - **Supply voltage:** 2.5V - 7.0V
  - **Note:** Other capacitive sensors may work but require calibration
- 16×2 I2C LCD display
- 4× tactile push buttons
- Breadboard + jumper wires
- USB-C cable (data + power)

### Wiring Diagram

**Pin Connections:**

| Component | Pin | Arduino Pin | Notes |
|-----------|-----|-------------|-------|
| **Soil Sensor** | VCC | D2 | GPIO power control (critical!) |
| | OUT | A0 | Analog output (14-bit ADC input) |
| | GND | GND | Common ground |
| | **DIS** | **GND** | **Disable pin - MUST connect to GND!** |
| **16×2 LCD** | VCC | 5V | I2C module |
| | GND | GND | |
| | SDA | SDA (A4) | I2C data |
| | SCL | SCL (A5) | I2C clock |
| **Button UP** | Pin 1 | D4 | Internal pullup enabled |
| | Pin 2 | GND | Active LOW |
| **Button DOWN** | Pin 1 | D5 | Internal pullup |
| | Pin 2 | GND | |
| **Button SELECT** | Pin 1 | D6 | Internal pullup |
| | Pin 2 | GND | |
| **Button BACK** | Pin 1 | D7 | Internal pullup |
| | Pin 2 | GND | |
| **USB Cable** | USB-C | Arduino USB port | Power + Serial data |

**Complete schematic:** See [`kicad/soil_humidity_monitor.kicad_sch`](kicad/soil_humidity_monitor.kicad_sch)

### CRITICAL: DIS Pin Wiring (Cytron MAKER-SOIL-MOISTURE)

**The DIS (Disable) pin MUST be connected to GND for the sensor to work!**

**What the DIS pin does:**
- **DIS = LOW (connected to GND):** Sensor enabled, outputs valid voltage (1.0-5.2V)
- **DIS = HIGH (floating or connected to VCC):** Sensor disabled, output invalid (~0.15V)

**Why your sensor might read wrong values:**

If you see ADC values around **500-600** (instead of 7,000-13,000), the DIS pin is likely:
- Not connected (floating)
- Accidentally connected to VCC
- Connected to a GPIO pin that's outputting HIGH

**Correct wiring:**
```
Sensor VCC → Arduino D2 (GPIO power)
Sensor OUT → Arduino A0 (ADC input)
Sensor GND → Arduino GND
Sensor DIS → Arduino GND  ← CRITICAL!
```

**Why we connect DIS to GND (not a GPIO):**
- Simple: No firmware changes needed
- Reliable: Sensor always enabled when VCC is powered
- Power savings still achieved: D2 GPIO controls VCC power

**Advanced option (not recommended for beginners):**
If you need even lower power consumption, you can control DIS from a GPIO pin:
```
Sensor DIS → Arduino D8
```

Then modify firmware to:
1. Set D8 LOW before taking sensor readings
2. Set D8 HIGH after readings complete

**Power consumption comparison:**
- **DIS = LOW:** 3.6mA active, sensor functional
- **DIS = HIGH:** 0.14mA sleep, sensor disabled (output unusable)

For this monitoring system, **always connect DIS to GND** - the power savings from GPIO control of VCC (D2) are already sufficient (0.02% duty cycle).

### Assembly Steps

1. **Prepare Sensor (CRITICAL - do this first!):**
   ```
   1. Apply clear nail polish to sensor PCB (front + back)
   2. Avoid coating the metal probes themselves
   3. Let dry 30 minutes
   4. Apply second coat
   5. Let dry 24 hours before use
   ```
   **Why?** Waterproofing extends sensor life from 3-6 months to 1-2+ years.

2. **Connect LCD:**
   - 5V → LCD VCC
   - GND → LCD GND
   - SDA (A4) → LCD SDA
   - SCL (A5) → LCD SCL

3. **Connect Sensor:**
   - D2 → Sensor VCC (GPIO-powered!)
   - GND → Sensor GND
   - A0 → Sensor OUT (analog output)
   - **GND → Sensor DIS (disable pin - MUST connect!)**

4. **Connect Buttons:**
   - D4 → Button UP Pin 1, GND → Button UP Pin 2
   - D5 → Button DOWN Pin 1, GND → Button DOWN Pin 2
   - D6 → Button SELECT Pin 1, GND → Button SELECT Pin 2
   - D7 → Button BACK Pin 1, GND → Button BACK Pin 2

5. **Double-Check:**
   - [ ] No shorts between 5V and GND
   - [ ] Sensor powered from D2 (not 5V rail)
   - [ ] LCD I2C connections not swapped
   - [ ] All button pins connected to GND

---

## Firmware Upload

### Prerequisites

1. **Install Arduino IDE** (2.0 or newer)
   - Download: https://www.arduino.cc/en/software

2. **Install Board Support:**
   - Open Arduino IDE
   - Tools → Boards Manager → Search "Arduino UNO R4"
   - Install "Arduino UNO R4 Boards" by Arduino

3. **Install Libraries:**
   - Sketch → Include Library → Manage Libraries
   - Search and install:
     - **LiquidCrystal I2C** by Frank de Brabander (v1.1.2+)
   - Built-in libraries (no install needed): Wire, EEPROM

### Upload Steps

1. **Open Firmware:**
   - File → Open → Navigate to `monitor/src/soil_humidity_monitor.ino`

2. **Configure Board:**
   - Tools → Board → Arduino UNO R4 WiFi
   - Tools → Port → (select your Arduino's COM port)

3. **Upload:**
   - Click Upload button (→) or press Ctrl+U
   - Wait for "Done uploading" message (~30 seconds)

4. **Verify:**
   - LCD should display: "Soil Monitor" / "v1.1"
   - Status screen should appear with "M:--% ADC:0" / "Log:0/1344"
   - Serial Monitor (115200 baud) should show: "Soil Humidity Monitor v1.1"

**Troubleshooting Upload Errors:**
- **Port not found:** Install CH340 USB drivers
- **Compilation error:** Check Arduino UNO R4 board support installed
- **Upload timeout:** Press reset button on Arduino, then upload again

---

## First-Time Setup

### 1. Initial Boot

LCD displays:
```
Soil Monitor
v1.1
```

Then automatically shows **Status Screen**:
```
M:--% ADC:0
Log:0/1344
```

**Note:** Moisture shows "--%" and ADC shows 0 until first sensor reading is taken (after calibration).

### 2. Sensor Calibration (REQUIRED)

**Why?** Each sensor has different ADC values for dry/wet conditions. Calibration ensures accurate 0-100% readings.

**The new calibration menu allows you to calibrate dry and wet values independently**, with both automatic (measured) and manual (edited) options for each.

**Method 1: Automatic Measurement (Recommended)**

Navigate to: Main Menu → Calibrate → Choose Dry or Wet → Measure Now

**For Dry Calibration:**
1. Main Menu → Calibrate → ">Dry Value" → SELECT
2. Select ">Measure Now" → SELECT
3. Display shows: "Hold in AIR" / "ADC:XXXXX SEL=OK"
4. Hold sensor in open air (not touching anything)
5. **Watch ADC value update every 0.5 seconds** (live reading)
6. Wait until value stabilizes (stops changing)
7. Press SELECT to capture dry value
8. Display shows: "Saved! D:XXXXX W:XXXXX" → Press SELECT
9. Typical dry value: 10,000-13,000 (14-bit ADC)

**For Wet Calibration:**
1. Main Menu → Calibrate → ">Wet Value" → SELECT
2. Select ">Measure Now" → SELECT
3. Display shows: "Wet soil/water" / "ADC:XXXXX SEL=OK"
4. Submerge sensor probes in glass of water (don't submerge entire PCB!)
5. **Watch ADC value update every 0.5 seconds** (live reading)
6. Wait until value stabilizes (usually 2-3 seconds)
7. Press SELECT to capture wet value
8. Display shows: "Saved! D:XXXXX W:XXXXX" → Press SELECT
9. Typical wet value: 7,500-9,000 (14-bit ADC)

**Method 2: Manual Edit (Advanced)**

Navigate to: Main Menu → Calibrate → Choose Dry or Wet → Edit Manually

**To edit dry value:**
1. Main Menu → Calibrate → ">Dry Value" → SELECT
2. Select ">Edit Manually" → SELECT
3. Display shows: "Dry (air) ADC" / "12400 UP/DN SEL"
4. Use UP/DOWN buttons to adjust (±50 per press)
   - **Note:** Value automatically rounds to nearest multiple of 50
   - Example: 11332 → rounds to 11350 → UP → 11400 → UP → 11450
5. Press SELECT to save
6. Display shows: "Saved! D:XXXXX W:XXXXX" → Press SELECT
7. Returns to Calibrate menu

**To edit wet value:**
1. Main Menu → Calibrate → ">Wet Value" → SELECT
2. Select ">Edit Manually" → SELECT
3. Display shows: "Wet (water) ADC" / "6000 UP/DN SEL"
4. Use UP/DOWN buttons to adjust (±50 per press)
   - **Note:** Value automatically rounds to nearest multiple of 50
   - Example: 9124 → rounds to 9100 → DOWN → 9050 → DOWN → 9000
5. Press SELECT to save
6. Display shows: "Saved! D:XXXXX W:XXXXX" → Press SELECT
7. Returns to Calibrate menu

**Benefits of independent calibration:**
- Recalibrate only dry OR wet without redoing both
- Test different dry calibration values without changing wet
- Fine-tune one value while keeping the other
- Faster workflow when you only need to adjust one value

**Calibration values persist through power cycles** - you only need to do this once!

**Viewing Calibration Values:**
- Main Menu → System Info shows current dry/wet ADC values
- Status screen always displays live raw ADC reading

### 3. Adjust Log Interval (Optional)

Default: 15 minutes (optimal for 14-day storage)

**To change:**

1. Main Menu → 1. Settings
2. Press SELECT
3. Display shows: "Log Interval: 15 min"
4. Use UP/DOWN buttons to adjust (1-60 minutes)
5. Press SELECT to save
6. Press BACK to exit

**Storage duration based on interval:**

| Interval | Duration | Use Case |
|----------|----------|----------|
| 10 min | 9.3 days | Fast data collection |
| 15 min | 14.0 days | **Default (optimal)** |
| 30 min | 28.0 days | Long-term trends |
| 60 min | 56.0 days | Extended monitoring |

---

## Operating the System

### Status Screen (Default View)

**What you see:**
```
M:75% ADC:7500
Log:456/1344
```

**Line 1:** Current moisture and raw sensor value
- **M:XX%** - Calibrated soil moisture (0-100%)
- **ADC:XXXXX** - Raw 14-bit ADC reading (0-16383)
- Updates every 2 seconds when backlight is on
- Shows "--%" before first calibration

**Line 2:** Logged entries / total capacity
- Example: "456/1344" = 456 readings stored, 1,344 max
- When full, oldest entries are automatically overwritten (circular buffer)
- **!** appears when buffer >90% full (warning to download data)

### Navigation Controls

| Button | Action |
|--------|--------|
| **UP** | Move up in menu / Increase value |
| **DOWN** | Move down in menu / Decrease value |
| **SELECT** | Enter menu / Confirm action |
| **BACK** | Exit menu / Cancel action |

**Backlight behavior:**
- Turns on when any button pressed
- Stays on for 1 minute
- Auto-sleeps to save power
- Data logging continues in background

### Menu Structure

```
STATUS SCREEN (default view: M:XX% ADC:XXXXX / Log:XXX/1344)
│
SELECT → MAIN MENU
         ├── Settings
         │   └── Log Interval
         │       └── Adjust 1-60 min (UP/DOWN) → SELECT saves
         │
         ├── Calibrate
         │   ├── Dry Value
         │   │   ├── Measure Now (automatic - hold sensor in air)
         │   │   │   └── Hold in AIR (live ADC display) → SELECT captures → Saved!
         │   │   └── Edit Manually (manual adjustment)
         │   │       └── 12400 UP/DN SEL (±50 increments) → SELECT saves
         │   └── Wet Value
         │       ├── Measure Now (automatic - put sensor in water)
         │       │   └── Wet soil/water (live ADC display) → SELECT captures → Saved!
         │       └── Edit Manually (manual adjustment)
         │           └── 6000 UP/DN SEL (±50 increments) → SELECT saves
         │
         ├── Download Data
         │   ├── "Download CSV?" → SELECT=Yes
         │   ├── Progress bar [####----] XX%
         │   └── "Download done! XXX entries"
         │
         ├── Clear Data
         │   └── "Clear all data?" → SELECT=Yes BCK=No
         │
         ├── System Info
         │   └── Cal D:12400 / W:6000 Int:15m
         │
         └── Reset to Defaults
             └── "Reset config?" → SELECT=Yes BCK=No
```

### Automatic Logging

**How it works:**
- Every 15 minutes (or your configured interval), the system:
  1. Powers on sensor (D2 = HIGH)
  2. Waits 200ms for warmup
  3. Takes 20 ADC readings (averaged to reduce noise)
  4. Powers off sensor (D2 = LOW)
  5. Converts to 0-100% using calibration
  6. Writes 4-byte entry to EEPROM
  7. Updates entry counter

**No user action required** - logging happens automatically in background.

**Entry count updates:**
- Status screen shows current count
- When reaching 1,344 entries (capacity), oldest entries overwrite automatically
- Entry count freezes at 1,344 when buffer is full

---

## CSV Download Instructions

### Method 1: Arduino IDE Serial Monitor (Easiest)

1. **Connect Arduino** via USB cable to computer

2. **Open Serial Monitor:**
   - Arduino IDE → Tools → Serial Monitor
   - Set baud rate: **115200** (bottom-right dropdown)
   - Set line ending: "Newline" or "Both NL & CR"

3. **Start Download on Arduino:**
   - Press SELECT (enter Main Menu)
   - Navigate: DOWN → DOWN → "3. Download Data"
   - Press SELECT
   - Display shows: "Ready to download?" / "SELECT=Yes BACK=No"
   - Press SELECT to confirm

4. **Monitor Progress:**
   - Serial Monitor shows metadata header
   - CSV data rows stream (Entry, Timestamp, Moisture%, etc.)
   - LCD shows progress bar: "Downloading... [####----] 50%"
   - Takes ~30-60 seconds for 1,344 entries

5. **Save CSV File:**
   - When complete, Serial Monitor shows: "# End of data export"
   - Select all text in Serial Monitor (Ctrl+A)
   - Copy (Ctrl+C)
   - Paste into text editor
   - Save as `moisture_data.csv`

### Method 2: Command-Line Serial (Advanced)

**Linux/Mac:**
```bash
# Find Arduino port
ls /dev/tty.* | grep usb

# Start serial capture
screen /dev/tty.usbserial-XXXXXX 115200 > moisture_data.csv

# On Arduino: Main Menu → Download Data → SELECT
# Wait for download to complete
# Stop capture: Ctrl+A, then K, then Y
```

**Windows (PowerShell):**
```powershell
# Open serial port
$port = new-Object System.IO.Ports.SerialPort COM3,115200,None,8,One
$port.Open()

# On Arduino: Main Menu → Download Data → SELECT

# Read data to file
$data = $port.ReadLine()
$data | Out-File -FilePath moisture_data.csv -Append

# Close port when done
$port.Close()
```

### CSV Format

**Example output:**
```csv
# Soil Humidity Monitor Data Export
# Firmware Version: 1.0
# Total Entries: 1234
# Log Interval: 15 minutes
# Sensor Calibration: Dry=12400, Wet=6000
# Buffer Status: NOT_WRAPPED
#
Entry,Timestamp_Hours,Timestamp_Minutes,Moisture_%,Raw_ADC,Flags
1,0.00,0,45,8500,0x00
2,0.25,15,46,8450,0x00
3,0.50,30,47,8400,0x00
...
1234,308.25,18495,52,7800,0x00
```

**Columns:**
- **Entry:** Sequential number (1-1344)
- **Timestamp_Hours:** Hours since monitoring started
- **Timestamp_Minutes:** Total minutes since start
- **Moisture_%:** Calibrated moisture (0-100%)
- **Raw_ADC:** Raw 14-bit ADC value (0-16383)
- **Flags:** Status bits (0x00 = normal, 0x01 = sensor error)

---

## Data Analysis Workflow

### Step 1: Import CSV to Excel

1. Open Excel
2. Data → From Text/CSV
3. Select `moisture_data.csv`
4. Delimiter: Comma
5. Click Load

**Skip comment lines** (lines starting with `#`) or delete them manually.

### Step 2: Create Charts

**Time-Series Plot:**
- X-axis: `Timestamp_Hours` or `Timestamp_Minutes`
- Y-axis: `Moisture_%`
- Chart type: Scatter with smooth lines

**What to look for:**
- **Dry-down rate:** How fast moisture drops naturally
- **Watering events:** Sharp increases (if manually watered)
- **Stabilization level:** Where moisture settles after watering
- **Daily patterns:** Does moisture change with temperature/sunlight?

### Step 3: Calculate Key Metrics

**Natural Dry-Down Rate:**
```
Rate = (Moisture_start - Moisture_end) / Time_hours
Example: (80% - 30%) / 120 hours = 0.42% per hour
```

**Optimal Watering Threshold:**
- Find moisture level where plant shows stress (wilting)
- Water before reaching this point
- Typical range: 10-30% depending on plant species

**Moisture Propagation Time:**
- How long after watering does sensor see moisture increase?
- Typical: 5-15 minutes (depends on soil type, water amount)

### Step 4: Configure Automatic Watering

Use insights to configure POC automatic watering system:

1. **Set Min Threshold:** 5-10% above stress point
   - Example: Plant wilts at 15% → Set `minThreshold = 20%`

2. **Set Target Humidity:** Peak moisture after manual watering
   - Example: Manual watering reaches 75% → Set `targetHumidity = 75%`

3. **Set Soak Time:** Moisture propagation time + buffer
   - Example: Sensor updates in 8 minutes → Set `soakTime = 10 minutes`

4. **Set Check Interval:** 2-3× natural dry-down period
   - Example: Dries from 80% → 30% in 5 days → Check moisture every 12 hours

### Example Analysis (Python)

```python
import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv('moisture_data.csv', comment='#')

# Plot time series
plt.figure(figsize=(12, 6))
plt.plot(df['Timestamp_Hours'], df['Moisture_%'], linewidth=1)
plt.xlabel('Time (hours)')
plt.ylabel('Soil Moisture (%)')
plt.title('2-Week Soil Moisture Monitoring')
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('moisture_trend.png', dpi=300)
plt.show()

# Calculate dry-down rate (linear regression)
from scipy.stats import linregress
slope, intercept, r_value, p_value, std_err = linregress(
    df['Timestamp_Hours'], df['Moisture_%']
)
print(f"Dry-down rate: {slope:.3f}% per hour")
print(f"R²: {r_value**2:.3f}")

# Find minimum moisture
min_moisture = df['Moisture_%'].min()
min_timestamp = df.loc[df['Moisture_%'].idxmin(), 'Timestamp_Hours']
print(f"Minimum moisture: {min_moisture}% at {min_timestamp:.1f} hours")
```

---

## Troubleshooting

### Sensor Issues

**Problem:** Moisture reads 0% or 100% constantly
- **Cause:** Sensor not calibrated, incorrect calibration values, or **DIS pin not connected to GND**
- **Fix:**
  1. **FIRST: Check DIS pin connection!** If ADC reads 500-600, DIS pin is not connected to GND
     - Verify DIS wire: Sensor DIS → Arduino GND
     - After connecting DIS to GND, ADC should read 7,000-13,000
  2. Check raw ADC value on status screen (should be 7,000-13,000 range for Cytron sensor)
  3. Run calibration wizard: Main Menu → Calibrate → Run Wizard
  4. Watch live ADC during calibration to ensure values stabilize
  5. Verify calibration: Main Menu → System Info (check Dry/Wet values)

**Problem:** Moisture fluctuates wildly (e.g., 20% → 80% → 30%)
- **Cause:** Poor electrical contact or sensor corrosion
- **Fix:**
  1. Check sensor wiring (AOUT to A0)
  2. Clean sensor probes with rubbing alcohol
  3. Re-waterproof if coating damaged
  4. Replace sensor if corroded

**Problem:** Moisture doesn't change when watering plant
- **Cause:** Sensor positioned in air pocket or outside water flow path
- **Fix:** Reposition sensor closer to roots, ensure soil contact

**Problem:** ADC values very low (500-600 instead of 7,000-13,000)
- **Cause:** **DIS pin not connected to GND** - sensor is disabled!
- **Symptoms:**
  - Status screen shows ADC:500-600 (instead of 7,000-13,000)
  - Moisture stuck at 100% even when sensor is in air
  - Calibration gives values like Dry=560, Wet=541 (completely wrong)
- **Fix:**
  1. **Connect Sensor DIS pin to Arduino GND** (critical!)
  2. Power cycle Arduino
  3. Check status screen - ADC should now read 10,000-13,000 in air
  4. Put sensor in water - ADC should drop to 7,000-9,000
  5. Re-run calibration wizard with correct ADC values
- **Why this happens:** The Cytron MAKER-SOIL-MOISTURE has a 4th wire (DIS) that must be pulled LOW to enable the sensor. If floating or HIGH, sensor outputs ~0.15V (invalid).

**Problem:** Calibration wizard shows unstable ADC values (constantly jumping)
- **Cause:** Electrical noise, poor sensor contact, or failing sensor
- **Fix:**
  1. Check sensor wiring connections (especially D2, A0, and **DIS to GND**)
  2. Ensure sensor is not near electrical interference (motors, WiFi routers)
  3. Try averaging: wait 5-10 seconds, watch for pattern in fluctuation
  4. If sensor physically damaged, replace it

**Problem:** Need to copy calibration values from another sensor
- **Cause:** Want to replicate working calibration without physical access
- **Fix:**
  1. Note down working sensor's values from System Info
  2. Main Menu → Calibrate → Edit Values
  3. Manually enter Dry and Wet ADC values
  4. Verify on status screen that moisture % looks reasonable

### EEPROM / Logging Issues

**Problem:** Entry count stuck at 0
- **Cause:** Logging interval expired but not saving entries
- **Fix:**
  1. Check System Info → verify interval not set to 0
  2. Reset to defaults: Main Menu → Reset to Defaults
  3. Recalibrate sensor

**Problem:** CSV download shows garbage data
- **Cause:** EEPROM corruption (rare) or wrong baud rate
- **Fix:**
  1. Verify Serial Monitor baud = **115200**
  2. Clear data: Main Menu → Clear Data
  3. Re-upload firmware if persists

**Problem:** "Buffer wrapped" but fewer than 1,344 entries
- **Cause:** Write pointer advanced incorrectly (firmware bug)
- **Fix:** Clear data and restart monitoring session

### LCD Display Issues

**Problem:** LCD shows nothing (blank screen)
- **Cause:** I2C address mismatch or wiring error
- **Fix:**
  1. Check LCD power (5V + GND)
  2. Verify SDA/SCL not swapped
  3. Try alternate I2C address: Change line 36 in `.ino`:
     ```cpp
     LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Try 0x3F instead of 0x27
     ```
  4. Run I2C scanner sketch to detect address

**Problem:** LCD shows garbled characters
- **Cause:** Loose I2C connection or voltage issue
- **Fix:**
  1. Re-seat I2C jumper wires
  2. Verify 5V power (not 3.3V)
  3. Check common ground connection

**Problem:** Backlight stays on forever
- **Cause:** Expected behavior if buttons pressed within 1-minute window
- **Fix:** Wait 1 minute without pressing buttons → auto-sleeps

### Button Issues

**Problem:** Buttons don't respond
- **Cause:** Wiring error or wrong internal pullup configuration
- **Fix:**
  1. Verify button pin 1 to Arduino, pin 2 to GND
  2. Check firmware lines 180-183 (internal pullups enabled)
  3. Test with multimeter: button pressed = LOW (0V)

**Problem:** Single button press registers multiple times
- **Cause:** Electrical noise or debounce timing too short
- **Fix:** Firmware handles debouncing (50ms delay) - if persists, add 0.1µF capacitor across button pins

### CSV Download Issues

**Problem:** Serial Monitor shows no output
- **Cause:** Wrong baud rate or Arduino not connected
- **Fix:**
  1. Set Serial Monitor baud = **115200** (bottom-right dropdown)
  2. Verify USB cable is data cable (not charge-only)
  3. Check Arduino IDE → Tools → Port shows Arduino

**Problem:** Download hangs at X%
- **Cause:** Serial buffer overflow (rare on UNO R4)
- **Fix:**
  1. Close other serial programs using same port
  2. Try slower download (increase delay in firmware line 740)
  3. Use command-line serial capture instead of Arduino IDE

---

## Technical Specifications

### System Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Storage Capacity** | 1,344 entries | 5,376 bytes EEPROM |
| **Storage Duration** | 14 days @ 15-min | User-adjustable 1-60 min |
| **Entry Size** | 4 bytes | Moisture%, ADC (2B), flags |
| **Configuration Size** | 32 bytes | Calibration, settings, pointers |
| **Unused EEPROM** | 2,784 bytes | Reserved for future features |
| **Buffer Type** | Circular | Auto-overwrites oldest entries |
| **ADC Resolution** | 14-bit (0-16383) | Arduino UNO R4 WiFi |
| **Sensor Power Duty Cycle** | 0.02% | Extends sensor life 4-8× |
| **Display Update Rate** | 2 seconds | When backlight active |
| **Backlight Timeout** | 1 minute | Auto-sleep to save power |
| **CSV Download Time** | 30-60 seconds | For 1,344 entries @ 115200 baud |

### EEPROM Endurance

- **Write Cycles:** 100,000 per address (Arduino UNO R4 WiFi spec)
- **Writes Per Day:** 96 entries (@ 15-min intervals)
- **Lifespan:** ~2,850 days (~7.8 years) per address rotation
- **Circular Buffer:** Each address written once every 14 days
- **Effective Lifespan:** 39,600 years (not a concern)

**Configuration header** (32 bytes) written only when user changes settings (~5 times over lifetime).

### Power Consumption

| Mode | Current | Notes |
|------|---------|-------|
| Active (logging) | ~40 mA | Sensor powered, LCD on |
| Idle (backlight off) | ~25 mA | Sensor off, LCD on |
| Sleep (future) | ~10 mA | Deep sleep between logs |

**USB-powered:** No external power supply needed (unlike POC variant with pump).

### Sensor Specifications (Cytron MAKER-SOIL-MOISTURE)

**Official Sensor Specs:**
- **Model:** Cytron MAKER-SOIL-MOISTURE
- **Datasheet:** [PDF](https://download.kamami.pl/p1178856-Dokumentacja_MAKER-SOIL-MOISTURE%20Datasheet.pdf)
- **Op-Amp Chip:** MCP6002 (dual) or MCP6004 (quad) - both variants work identically
- **Supply voltage:** 2.5V - 7.0V (5V nominal)
- **Output voltage:** 1.0V - 5.2V (higher = drier soil)
- **Current consumption:** 3.6mA @ 5V (active), 0.14mA (disabled via DIS pin)
- **Interface:** 4-wire Grove (VCC, GND, OUT, DIS)
- **Disable pin (DIS):** Active HIGH (pull LOW to enable sensor)

**Expected ADC Readings (14-bit, 5V supply):**

| Parameter | Voltage | ADC Value (14-bit) | Notes |
|-----------|---------|-------------------|-------|
| **Dry (air)** | 3.1-3.6V | 10,164-11,803 | Red LED on sensor |
| **Moist (typical soil)** | 2.7-3.1V | 8,852-10,164 | Green LED on sensor |
| **Wet (water)** | 2.3-2.7V | 7,540-8,852 | Blue LED on sensor |
| **Minimum output** | 1.0V | 3,277 | Fully saturated |
| **Maximum output** | 5.2V | 17,039 (clipped to 16,383) | Completely dry |

**Typical calibration values:**
- **Dry ADC (air):** 11,000-13,000
- **Wet ADC (water):** 7,500-9,000
- **Calibration range:** 3,000-6,000 ADC units

**Operational Characteristics:**

| Parameter | Value | Notes |
|-----------|-------|-------|
| ADC Resolution | 14-bit | 0-16,383 (Arduino UNO R4 WiFi) |
| Warmup Time | 200 ms | Before reading |
| Reading Time | 200 ms | 20 samples × 10ms averaged |
| Power-On Duration | 400 ms total | Per 15-min logging cycle |
| Duty Cycle | 0.02% | Extends sensor life 4-8× |
| Lifespan (continuous) | 3-6 months | Without GPIO power control |
| Lifespan (GPIO-powered) | 1-2 years | With waterproofing + duty cycling |

**Common Issues:**

| Symptom | Cause | Solution |
|---------|-------|----------|
| ADC reads 500-600 | DIS pin not connected to GND | Connect DIS → GND |
| Moisture always 100% | ADC below wet calibration value | Check DIS pin, re-calibrate |
| Moisture always 0% | ADC above dry calibration value | Re-calibrate sensor |
| Unstable readings | Poor wiring or corrosion | Check connections, clean sensor |

---

## Maintenance & Cleaning

### Regular Maintenance (Weekly)

- **Visual check:** Verify LCD displays current moisture
- **Entry count:** Check status screen, ensure logging continues
- **Sensor position:** Verify sensor hasn't shifted out of soil

### After Each Session (2-4 weeks)

1. **Download data** (backup before clearing)
2. **Clear EEPROM:** Main Menu → Clear Data
3. **Clean sensor:**
   - Remove from soil
   - Rinse with water
   - Wipe with soft cloth
   - Check waterproofing (re-coat if damaged)
4. **Restart monitoring** (or switch to POC automatic watering)

### Long-Term Storage

- **Remove sensor from soil** (prevents corrosion)
- **Disconnect USB power**
- **Store in dry location**
- **Configuration persists** - no need to recalibrate after storage

---

## Next Steps

### After 2-Week Monitoring

You now have quantitative data about your plant's watering needs!

**What to do with your data:**

1. **Analyze CSV data** (see Data Analysis Workflow above)
   - Calculate natural dry-down rate
   - Identify optimal moisture range for your plant
   - Determine how long soil stays moist after watering

2. **Improve your watering routine:**
   - Water when moisture drops to calculated threshold (e.g., 20%)
   - Water until reaching optimal peak moisture (e.g., 75%)
   - Adjust watering frequency based on observed dry-down patterns

3. **Continue monitoring:**
   - Download data periodically (every 2 weeks)
   - Clear buffer and restart monitoring
   - Compare seasonal changes in water consumption
   - Track how different environmental conditions affect moisture

4. **Use insights for decision-making:**
   - Understand when plants need water while on vacation
   - Detect if drainage is poor (moisture stays high too long)
   - Identify if watering too frequently (moisture never drops)
   - Optimize watering schedule for plant health

---

## Support & Feedback

- **Issues:** Report bugs at project repository
- **Firmware version:** Check Main Menu → System Info
- **Changelog:** See [`CHANGELOG.md`](CHANGELOG.md)
- **Schematic:** See [`kicad/soil_humidity_monitor.kicad_sch`](kicad/soil_humidity_monitor.kicad_sch)
- **BOM:** See [`BOM_MONITOR.md`](BOM_MONITOR.md)
- **Project overview:** See [`../CLAUDE.md`](../CLAUDE.md)

**Current firmware version:** v1.1 (stable)
