# LCD Menu System Design — v1.1

Hardware: Arduino UNO R4 WiFi · 16×2 I2C LCD · 4 buttons

## Button Layout

| Button | Pin | Function |
|--------|-----|----------|
| UP | D4 | Navigate up / increase value |
| DOWN | D5 | Navigate down / decrease value |
| SELECT | D6 | Confirm / enter submenu / toggle |
| BACK | D7 | Cancel / return to previous screen |

Debounce delay: **200 ms**

---

## Default Configuration

| Parameter | Default | Unit | Notes |
|-----------|---------|------|-------|
| Target humidity | 75 | % | Stop watering at this moisture |
| Min threshold | 55 | % | Start watering below this |
| Initial water amount | 50 | mL | First pump dose |
| Increment water amount | 10 | mL | Subsequent doses |
| Check interval | 6 | hours | Dry cycle check frequency |
| Soak time (initial) | 15 | min | Wait after first pump |
| Soak time (increment) | 1 | min | Wait between increments |
| Sensor dry ADC | 11850 | — | 14-bit ADC value in air |
| Sensor wet ADC | 7700 | — | 14-bit ADC value in water @ 60cm |
| Pump rate | 1.67 | mL/s | Typical S-3Z peristaltic pump |
| Backlight timeout | 1 | min | LCD auto-sleep |
| Sensor warmup | 1000 | ms | Stabilisation delay after power-on |
| Num samples | 10 | — | Median filter sample count |
| Sample delay | 100 | ms | Delay between samples |
| Log interval | 15 | min | EEPROM circular buffer write rate |
| Watering enabled | ON | — | Auto-watering active by default |

---

## Menu Structure

```
STATUS_SCREEN (default idle)
│
└── [any button] → MAIN_MENU (6 items, scroll one at a time)
    ├── 1.Settings     → SETTINGS_MENU (12 items)
    │   ├── Target %          → SETTING_TARGET
    │   ├── Min Thresh %      → SETTING_MIN
    │   ├── Init Water mL     → SETTING_AMOUNT_INITIAL
    │   ├── Incr Water mL     → SETTING_AMOUNT_INCREMENT
    │   ├── Check hrs         → SETTING_INTERVAL
    │   ├── Soak min          → SETTING_SOAK_TIME
    │   ├── Soak Incr min     → SETTING_SOAK_TIME_INCREMENT
    │   ├── Log min           → SETTING_LOG_INTERVAL
    │   ├── Warmup ms         → SETTING_SENSOR_WARMUP
    │   ├── Samples           → SETTING_NUM_MEASUREMENTS
    │   ├── Delay ms          → SETTING_MEASUREMENT_DELAY
    │   └── Backlight         → SETTING_BACKLIGHT_TIMEOUT
    │
    ├── 2.Calibrate    → CALIBRATE_MENU
    │   ├── 1.Sensor   → CAL_SENSOR_MENU
    │   │   ├── 1.Measure Air   → CAL_SENSOR_AIR   → CAL_SENSOR_DONE
    │   │   └── 2.Measure Water → CAL_SENSOR_WATER  → CAL_SENSOR_DONE
    │   └── 2.Pump     → CAL_PUMP_START → CAL_PUMP_RUNNING
    │                               [BACK stops pump] → CAL_PUMP_ENTER_ML
    │                                                 → CAL_PUMP_DONE
    │
    ├── 3.Manual Water → MANUAL_WATER_CONFIRM → waterPlant() → STATUS_SCREEN
    ├── 4.Download Log → DOWNLOAD_CONFIRM → downloadLog() → STATUS_SCREEN
    ├── 5.Reset        → RESET_CONFIRM → saveDefaults() → STATUS_SCREEN
    └── 6.Watering: ON/OFF  [SELECT toggles, saves immediately]
```

---

## Screen Layout Examples (16×2 LCD)

### Status Screen (idle)

```
┌────────────────┐
│Moisture: 68%   │
│OK              │
└────────────────┘
```

Line 2 variants:

| Condition | Line 2 |
|-----------|--------|
| Watering disabled | `Watering: OFF  ` |
| Moisture ≤ minThreshold, 2nd dry reading | `Dry! Watering  ` |
| Moisture ≤ minThreshold, 1st dry reading | `Need water!    ` |
| Moisture > minThreshold | `OK             ` |

### Main Menu

Shows one item at a time. UP/DOWN scrolls; SELECT enters.

```
┌────────────────┐
│1.Settings      │
│Press SELECT    │
└────────────────┘
```

Item 6 (Watering toggle):

```
┌────────────────┐
│6.Watering: ON  │
│Press SELECT    │
└────────────────┘
```

### Settings Menu

```
┌────────────────┐
│Target %        │
│Press SELECT    │
└────────────────┘
```

### Setting Adjustment Screen

```
┌────────────────┐
│Target:         │
│75% (UP/DN)     │
└────────────────┘
```

```
┌────────────────┐
│Init:           │
│50mL (UP/DN)    │
└────────────────┘
```

```
┌────────────────┐
│Soak:           │
│15min (UP/DN)   │
└────────────────┘
```

BACK saves and returns to settings list.

### Calibrate Menu

```
┌────────────────┐
│1.Sensor        │
│Press SELECT    │
└────────────────┘
```

### Sensor Calibration — Measure Air

```
┌────────────────┐
│1.Measure Air   │
│Press SELECT    │
└────────────────┘
```

During measurement:

```
┌────────────────┐
│Air: 11850      │
│SELECT to save  │
└────────────────┘
```

### Sensor Calibration — Measure Water

```
┌────────────────┐
│2.Measure Water │
│Press SELECT    │
└────────────────┘
```

```
┌────────────────┐
│Water: 7700     │
│SELECT to save  │
└────────────────┘
```

After saving either:

```
┌────────────────┐
│Cal saved!      │
│Dry=11850 Wet=7700│
└────────────────┘
```

### Pump Calibration

```
┌────────────────┐
│Pump Calibration│
│SELECT to run   │
└────────────────┘
```

Running (press BACK to stop):

```
┌────────────────┐
│Pump running... │
│5s (BACK stop)  │
└────────────────┘
```

Enter collected mL:

```
┌────────────────┐
│Enter mL:15     │
│UP/DN then SELECT│
└────────────────┘
```

Done:

```
┌────────────────┐
│Rate: 1.67mL/s  │
│Press BACK      │
└────────────────┘
```

### Manual Watering

```
┌────────────────┐
│Manual Watering?│
│SELECT=yes BACK=no│
└────────────────┘
```

### Download Log

```
┌────────────────┐
│Download Log    │
│SELECT=yes BACK=no│
└────────────────┘
```

SELECT outputs CSV to Serial at 115200 baud (timestamp index, moisture %, raw ADC, flags).

### Reset to Defaults

```
┌────────────────┐
│Reset to        │
│Defaults? SEL/BCK│
└────────────────┘
```

---

## Navigation State Machine

```cpp
enum MenuState {
    STATUS_SCREEN,
    MAIN_MENU,
    SETTINGS_MENU,
    SETTING_TARGET,
    SETTING_MIN,
    SETTING_AMOUNT_INITIAL,
    SETTING_AMOUNT_INCREMENT,
    SETTING_INTERVAL,
    SETTING_SOAK_TIME,
    SETTING_SOAK_TIME_INCREMENT,
    SETTING_LOG_INTERVAL,
    SETTING_SENSOR_WARMUP,
    SETTING_NUM_MEASUREMENTS,
    SETTING_MEASUREMENT_DELAY,
    SETTING_BACKLIGHT_TIMEOUT,
    CALIBRATE_MENU,
    CAL_SENSOR_MENU,
    CAL_SENSOR_AIR,
    CAL_SENSOR_WATER,
    CAL_SENSOR_DONE,
    CAL_PUMP_START,
    CAL_PUMP_RUNNING,
    CAL_PUMP_ENTER_ML,
    CAL_PUMP_DONE,
    MANUAL_WATER_CONFIRM,
    MANUAL_WATER_RUNNING,
    DOWNLOAD_MENU,
    DOWNLOAD_CONFIRM,
    DOWNLOAD_PROGRESS,
    RESET_CONFIRM
};
```

### Button Actions by State

| State | UP | DOWN | SELECT | BACK |
|-------|----|------|--------|------|
| STATUS_SCREEN | → MAIN_MENU | → MAIN_MENU | → MAIN_MENU | — |
| MAIN_MENU | prev item | next item | enter item | → STATUS_SCREEN |
| SETTINGS_MENU | prev item | next item | enter setting | → MAIN_MENU |
| SETTING_* | increase | decrease | — | save → SETTINGS_MENU |
| CALIBRATE_MENU | prev item | next item | enter | → MAIN_MENU |
| CAL_SENSOR_MENU | prev item | next item | enter step | → CALIBRATE_MENU |
| CAL_SENSOR_AIR/WATER | — | — | save → CAL_SENSOR_DONE | → CAL_SENSOR_MENU |
| CAL_SENSOR_DONE | — | — | → CALIBRATE_MENU | → CAL_SENSOR_MENU |
| CAL_PUMP_START | — | — | → CAL_PUMP_RUNNING | → CALIBRATE_MENU |
| CAL_PUMP_RUNNING | — | — | — | stop pump → CAL_PUMP_ENTER_ML |
| CAL_PUMP_ENTER_ML | increase | decrease | calc rate → CAL_PUMP_DONE | → CAL_PUMP_START |
| CAL_PUMP_DONE | — | — | — | → CALIBRATE_MENU |
| MANUAL_WATER_CONFIRM | — | — | waterPlant() → STATUS | → STATUS |
| DOWNLOAD_CONFIRM | — | — | downloadLog() → STATUS | → STATUS |
| RESET_CONFIRM | — | — | saveDefaults() → STATUS | → STATUS |

---

## EEPROM Memory Map

Header: **40 bytes** (addresses 0–39). Data: addresses 40–1023 (circular buffer).

```
Addr  Size  Field
   0     2  Magic number (0xA5C3)
   2     2  Firmware version
   4     1  Target humidity (0–100)
   5     1  Min threshold (0–100)
   6     2  Initial water amount (mL × 10)
   8     2  Increment water amount (mL × 10)
  10     1  Check interval (hours)
  11     1  Soak time after initial pump (minutes)
  12     1  Soak time between increments (minutes)
  13     2  Sensor dry ADC (14-bit)
  15     2  Sensor wet ADC (14-bit)
  17     4  Pump rate (float, mL/s)
  21     2  Backlight timeout (minutes)
  23     2  Sensor warmup (milliseconds)
  25     1  Number of measurements (5–50)
  26     2  Measurement delay (milliseconds)
  28     2  Log interval (minutes)
  30     4  Circular buffer write pointer
  34     4  Total entries written
  38     1  Watering enabled (0=off, 1=on)
  39     1  (reserved / alignment)
  40  984  Log data: 246 entries × 4 bytes each
            Entry layout: moisture% (1B) | rawADC high (1B) | rawADC low (1B) | flags (1B)
```

Log flags: `0x00` normal · `0x01` watering started · `0x02` watering complete

---

## Value Adjustment Steps

| Parameter | Step | Min | Max |
|-----------|------|-----|-----|
| Target / Min threshold | 1% | 0 | 100 |
| Initial / Increment water | 1 mL (10 in storage) | 1 mL | 500 mL |
| Check interval | 1 h | 1 | 24 |
| Soak time (both) | 1 min | 1 | 30 |
| Log interval | 1 min | 1 | 60 |
| Sensor warmup | 1 ms (display) | 100 | 2000 |
| Num samples | 1 | 5 | 50 |
| Measurement delay | 1 ms | 10 | 500 |
| Backlight timeout | 1 min | 1 | 60 |

---

## Timing

- Button debounce: **200 ms**
- Display update (status screen): every **2 s**
- Auto-watering check: every **logInterval** minutes (default 15 min)
- Watering triggers after **2 consecutive** below-threshold readings
- Backlight auto-sleep: after **backlightTimeout** minutes of inactivity (default 1 min)
