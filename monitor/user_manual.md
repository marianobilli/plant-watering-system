# User Manual - Soil Humidity Monitor

**Operation Guide for End Users**

---

## Introduction

### What is this device?

The Soil Humidity Monitor is a data-logging device that tracks soil moisture levels in your plants over time. It records measurements to internal memory (EEPROM) and allows you to download the data for analysis via USB connection.

### Project Objective

Most DIY automatic watering systems fail because they use guessed threshold values instead of measured data. This monitor helps you understand your plant's **natural watering cycles** before implementing automatic irrigation, ensuring optimal watering parameters based on real-world behavior.

### When to use this monitor

- **New to plant care:** Learn how fast your plant dries out naturally
- **Unknown plant species:** Different plants have vastly different water needs
- **Testing new soil mix:** Soil composition affects moisture retention dramatically
- **Before automation:** Gather data to configure automatic watering systems correctly

### What you'll learn

After 1-2 weeks of monitoring, you'll have quantitative data showing:
- **Dry-down rate:** How fast moisture drops naturally (e.g., 0.42% per hour)
- **Optimal thresholds:** When your plant needs water (before showing stress)
- **Target humidity:** Peak moisture level after watering
- **Moisture propagation time:** How long water takes to reach the sensor

**This data is essential for configuring reliable automatic watering systems.**

---

## First-Time Setup

### 1. Initial Boot

After powering on (USB connected), the LCD displays:

```
Soil Monitor
v1.5
```

Then automatically shows **Status Screen**:

```
M:--% ADC:0
Log:0/1344
```

**Note:** Moisture shows "--%" and ADC shows 0 until first sensor reading is taken (after calibration).

### 2. Sensor Calibration (REQUIRED)

**Why?** Each sensor has different ADC values for dry/wet conditions. Calibration ensures accurate 0-100% readings.

The calibration menu allows you to calibrate dry and wet values independently, with both automatic (measured) and manual (edited) options for each.

#### Method 1: Automatic Measurement (Recommended)

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

#### Method 2: Manual Edit (Advanced)

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

#### Method 3: Set by Depth (NEW in v1.5)

Navigate to: Main Menu → Calibrate → Wet Value → Set by Depth

**Why this matters:** Sensor readings vary dramatically with insertion depth (see [README.md](README.md#practical-lessons-learned) for detailed explanation). This method uses empirically measured values to calibrate without physically submerging the sensor.

1. Main Menu → Calibrate → ">Wet Value" → SELECT
2. Select ">Set by Depth" → SELECT (navigate with UP/DOWN if needed)
3. Display shows: "Insert depth:" / "60cm -> ADC:7700"
4. Use UP/DOWN buttons to select your typical planting depth:
   - 10cm → ADC:11800 (barely inserted - sensor mostly in air)
   - 20cm → ADC:10000 (shallow insertion)
   - 30cm → ADC:9200 (typical potted plants)
   - 40cm → ADC:8500 (deeper pots)
   - 50cm → ADC:8000 (deep containers)
   - 60cm → ADC:7700 (full insertion in water) **← Default, recommended**
5. Press SELECT to save selected depth value
6. Display shows: "Saved! D:XXXXX W:XXXXX" → Press SELECT
7. Returns to Calibrate menu

**Important:** Whichever depth you select becomes your **100% wet calibration**. The ADC values represent what the sensor reads at that insertion depth in water. Choose the depth that matches how deep you'll insert the sensor in your soil.

**When to use each method:**
- **Measure Now:** Most accurate, but requires deep water container
- **Edit Manually:** Fine-tuning after measurement, or copying known values
- **Set by Depth:** Quick calibration without water, ideal for field deployment

**Benefits of independent calibration:**
- Recalibrate only dry OR wet without redoing both
- Test different dry calibration values without changing wet
- Fine-tune one value while keeping the other
- Faster workflow when you only need to adjust one value

**Calibration values persist through power cycles** - you only need to do this once!

**Viewing Calibration Values:**
- Main Menu → System Info shows current dry/wet ADC values
- Status screen always displays live raw ADC reading

### 3. Configure Settings (Optional)

The Settings menu has 5 configurable parameters. Navigate with UP/DOWN, press SELECT to enter, adjust value, and SELECT again to save.

**To access Settings:**
1. Main Menu → 1. Settings → SELECT
2. Navigate through 5 options with UP/DOWN
3. Press SELECT to enter each setting

#### Setting 1: Log Interval

**Default:** 15 minutes (optimal for 14-day storage)

**Range:** 1-60 minutes

**Display:** "Log Interval" / "15min UP/DN SEL"

**Storage duration based on interval:**

| Interval | Duration | Use Case |
|----------|----------|----------|
| 10 min | 9.3 days | Fast data collection |
| 15 min | 14.0 days | **Default (optimal)** |
| 30 min | 28.0 days | Long-term trends |
| 60 min | 56.0 days | Extended monitoring |

#### Setting 2: LCD Sleep (Backlight Timeout)

**Default:** 1 minute

**Range:** 1-60 minutes

**Display:** "LCD Sleep" / "1min UP/DN SEL"

**Purpose:** Controls how long the LCD backlight stays on after button press. Shorter timeout saves power, longer timeout is more convenient if frequently checking status.

#### Setting 3: Sensor Warmup

**Default:** 1000 ms (1 second)

**Range:** 100-2000 ms (increments of 100ms)

**Display:** "Sensor Warmup" / "1000ms UP/DN SEL"

**Purpose:** Time to wait after enabling sensor (DIS LOW) before taking readings. 1000ms provides optimal accuracy. See [README.md](README.md#optimal-sensor-warmup-time) for detailed explanation.

**When to adjust:**
- **Decrease to 500ms:** Faster logging cycles, slight noise acceptable
- **Increase to 1500ms:** Unstable readings or cold environment

#### Setting 4: Num Samples (Number of Measurements)

**Default:** 10 samples

**Range:** 5-50 samples

**Display:** "Num Samples" / "10 UP/DN SEL"

**Purpose:** How many ADC readings to take for median calculation. More samples = better noise rejection but longer reading time.

**Typical values:**
- 5 samples: Fast readings, minimal noise filtering
- 10 samples: **Default balance** (good noise rejection, reasonable speed)
- 20-50 samples: Maximum noise rejection (use in electrically noisy environments)

#### Setting 5: Sample Delay (Measurement Delay)

**Default:** 100 ms

**Range:** 10-500 ms (increments of 10ms)

**Display:** "Sample Delay" / "100ms UP/DN SEL"

**Purpose:** Delay between individual ADC samples during multi-sample reading.

**Total reading time = Sensor Warmup + (Num Samples × Sample Delay)**

Example: 1000ms + (10 × 100ms) = 2000ms per reading

**When to adjust:**
- **Decrease to 50ms:** Faster overall reading time
- **Increase to 200-500ms:** Allow more time for sensor output to stabilize between samples

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
- Example: "456/1344" = 456 readings stored, 1344 max
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
         ├── Settings (5 options)
         │   ├── 1. Log Interval
         │   │   └── Adjust 1-60 min (UP/DOWN) → SELECT saves
         │   ├── 2. LCD Sleep (Backlight Timeout)
         │   │   └── Adjust 1-60 min (UP/DOWN) → SELECT saves
         │   ├── 3. Sensor Warmup
         │   │   └── Adjust 100-2000 ms (UP/DOWN) → SELECT saves
         │   ├── 4. Num Samples (Number of Measurements)
         │   │   └── Adjust 5-50 samples (UP/DOWN) → SELECT saves
         │   └── 5. Sample Delay (Measurement Delay)
         │       └── Adjust 10-500 ms (UP/DOWN) → SELECT saves
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
         │       ├── Edit Manually (manual adjustment)
         │       │   └── 6000 UP/DN SEL (±50 increments) → SELECT saves
         │       └── Set by Depth (lookup table - no water needed)
         │           └── 60cm -> ADC:7700 (UP/DN changes depth) → SELECT saves
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
  1. Enables sensor (D2 = LOW, DIS pin enabled)
  2. Waits 500ms for warmup (configurable via Settings)
  3. Takes 10 ADC readings (median calculation, reduces noise)
  4. Disables sensor (D2 = HIGH, DIS pin disabled)
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

**1. Connect Arduino** via USB cable to computer

**2. Open Serial Monitor:**
- Arduino IDE → Tools → Serial Monitor
- Set baud rate: **115200** (bottom-right dropdown)
- Set line ending: "Newline" or "Both NL & CR"

**3. Start Download on Arduino:**
- Press SELECT (enter Main Menu)
- Navigate: DOWN → DOWN → "3. Download Data"
- Press SELECT
- Display shows: "Ready to download?" / "SELECT=Yes BACK=No"
- Press SELECT to confirm

**4. Monitor Progress:**
- Serial Monitor shows metadata header
- CSV data rows stream (Entry, Timestamp, Moisture%, etc.)
- LCD shows progress bar: "Downloading... [####----] 50%"
- Takes ~60-120 seconds for 1,344 entries

**5. Save CSV File:**
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
# Firmware Version: 1.5
# Total Entries: 1234
# Log Interval: 15 minutes
# Sensor Calibration: Dry=11850, Wet=9200
# Buffer Status: NOT_WRAPPED
#
Entry,Timestamp_Hours,Timestamp_Minutes,Moisture_%,Median_ADC,Flags
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
- **Median_ADC:** Median 14-bit ADC value from 10 samples (0-16383)
- **Flags:** Status bits (0x00 = normal, 0x01 = sensor error)

**Note:** Min/Max ADC values are displayed in live Serial Monitor output during logging but not stored in EEPROM or CSV export (v1.3+ memory optimization).

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
plt.title('1-Week Soil Moisture Monitoring')
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

### After 1-Week Monitoring

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

## Technical Specifications (Operational)

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

### Buffer Behavior

- **Type:** Circular buffer (auto-overwrite oldest entries when full)
- **Capacity:** 1,344 entries (fixed)
- **Entry size:** 4 bytes (moisture%, ADC, flags)
- **Warning threshold:** 90% full (exclamation mark on status screen)

### Display Behavior

- **Update rate:** 2 seconds (when backlight active)
- **Backlight timeout:** 1 minute
- **Data logging:** Continues in background regardless of backlight state

---

## Troubleshooting (Operational Issues)

### Sensor Reading Issues

**Problem:** Moisture reads 0% or 100% constantly
- **Cause:** Sensor not calibrated, incorrect calibration values, or DIS pin not connected to D2
- **Fix:**
  1. **FIRST: Check DIS pin connection!** If ADC reads 500-600, DIS pin wiring is incorrect (see [build.md](build.md#sensor-wiring-issues))
  2. Check raw ADC value on status screen (should be 7,000-13,000 range for Cytron sensor)
  3. Run calibration wizard: Main Menu → Calibrate → Measure Now
  4. Watch live ADC during calibration to ensure values stabilize
  5. Verify calibration: Main Menu → System Info (check Dry/Wet values)

**Problem:** Moisture fluctuates wildly (e.g., 20% → 80% → 30%)
- **Cause:** Poor electrical contact or sensor corrosion
- **Fix:**
  1. Check sensor wiring (OUT to A0) - see [build.md](build.md#wiring-diagram)
  2. Clean sensor probes with rubbing alcohol
  3. Re-waterproof if coating damaged
  4. Replace sensor if corroded

**Problem:** Moisture doesn't change when watering plant
- **Cause:** Sensor positioned in air pocket or outside water flow path
- **Fix:** Reposition sensor closer to roots, ensure soil contact

**Problem:** Need to copy calibration values from another sensor
- **Cause:** Want to replicate working calibration without physical access
- **Fix:**
  1. Note down working sensor's values from System Info
  2. Main Menu → Calibrate → Edit Manually (for both Dry and Wet)
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

## Support & Feedback

- **Hardware issues:** See [build.md](build.md#troubleshooting-hardware--assembly)
- **Firmware version:** Check Main Menu → System Info
- **Changelog:** See [`CHANGELOG.md`](CHANGELOG.md)
- **Project overview:** See [README.md](README.md)
- **Report issues:** See project repository
