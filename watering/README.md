# Plant Watering System - Phase 2 (Auto-Watering Firmware)

Version 1.0 — First full auto-watering implementation combining reliable sensor reading with automatic watering logic.

## Overview

Phase 2 transforms the monitor-only firmware (Phase 1) into a **fully automatic plant watering system**. The firmware reads soil moisture via a 14-bit ADC sensor (with DIS pin control for power efficiency), and automatically activates a 12V peristaltic pump when soil moisture drops below a configurable threshold.

**Key improvements over POC:**
- ✅ **Correct 14-bit ADC handling** — includes `analogReadResolution(14)` to prevent ~16× calibration error
- ✅ **DIS pin sensor control** — extends sensor lifespan vs. VCC-cycling
- ✅ **Configurable median sampling** — robustness + flexibility (adjust sample count, delays)
- ✅ **EEPROM persistence** — all watering and sensor parameters saved across power cycles
- ✅ **Circular logging** — ~2.7 days of moisture history in 2KB EEPROM (~400 entries @ 15-min intervals)
- ✅ **Automatic failsafe** — stops watering if moisture doesn't increase (pump/sensor fault detection)

## Hardware Requirements

### Arduino & Sensor
- **1× Arduino UNO R4 WiFi** (or UNO R3 with WiFi shield if needed)
- **1× Capacitive Soil Moisture Sensor** with DIS pin (e.g., DF-Robot SEN0193 or similar)
- **1× 16x2 I2C LCD Display** (address 0x27 or 0x3F)
- **4× Pushbuttons** (UP/DOWN/SELECT/BACK navigation)

### Pump & Power Electronics
- **1× 12V Peristaltic Pump** (e.g., S-3Z model, ~1.67 mL/s flow rate)
- **1× IRLZ44N N-Channel Logic-Level MOSFET** (gate driver for pump)
- **1× 1N5819 Schottky Diode** (flyback protection on pump coil)
- **2× 10kΩ Resistors** (MOSFET gate current limit + pull-down)
- **1× 12V 2A Power Supply** (for pump + tube + connectors)
- **Dual USB + 12V Power** (Arduino from USB, pump from 12V) or single 12V supply with buck converter

## Pin Assignments

| Pin | Function | Purpose |
|-----|----------|---------|
| A0 | Soil Moisture Sensor ADC | Analog input (14-bit, 0-16383) |
| D2 | Sensor DIS Pin | Disable pin control (HIGH=sleep, LOW=active) |
| D3 | Pump MOSFET Gate | MOSFET control signal (HIGH=pump ON) |
| D4 | Button UP | Navigation up |
| D5 | Button DOWN | Navigation down |
| D6 | Button SELECT | Menu select / confirm |
| D7 | Button BACK | Menu back / cancel |
| SDA (A4) | LCD I2C Data | I2C communication |
| SCL (A5) | LCD I2C Clock | I2C communication |

## Firmware Features

### Operating Mode: Auto-Watering (Default)
- Continuous monitoring with configurable check intervals (1-24 hours)
- Dry cycle logic: only waters when soil moisture ≤ `minThreshold`
- Incremental watering: pumps configurable water amounts, waits for capillary absorption, checks moisture
- Automatic failsafe: stops if moisture doesn't increase (pump fault, sensor error, etc.)
- Soak time: waits N minutes between increments for moisture to propagate through soil

### Configuration (EEPROM-Persisted)
**Watering Parameters:**
- `targetHumidity` — Stop watering when moisture reaches this % (default: 75%)
- `minThreshold` — Start watering when moisture drops below this % (default: 55%)
- `waterAmountInitial` — Initial pump amount in mL (default: 50 mL)
- `waterAmountIncrement` — Each subsequent pump amount in mL (default: 10 mL)
- `checkInterval` — Hours between dry-cycle checks (default: 6 hours)
- `soakTime` — Minutes to wait after initial pump (default: 5 minutes)
- `soakTimeIncrement` — Minutes to wait between increments (default: 1 minute)
- `pumpRate` — mL/s calibrated from pump hardware (default: 1.67 mL/s)

**Sensor Parameters:**
- `sensorDry` — 14-bit ADC value in air (default: 11850, user-calibrated)
- `sensorWet` — 14-bit ADC value in water (default: 7700 @ 60cm depth)
- `sensorWarmup` — Sensor stabilization delay in ms (default: 1000ms)
- `numMeasurements` — Sample count for median (default: 10)
- `measurementDelay` — ms between samples (default: 100ms)

**Display:**
- `backlightTimeout` — LCD auto-sleep delay in minutes (default: 1 minute)

### Menu Structure
```
STATUS SCREEN
└─ [Select] → MAIN MENU
   ├─ Settings
   │  ├─ Target % (watering stop threshold)
   │  ├─ Min Threshold (watering start trigger)
   │  ├─ Water Amount (mL per increment)
   │  ├─ Check Interval (hours)
   │  ├─ Soak Time (minutes)
   │  ├─ Sensor Warmup (ms)
   │  ├─ Num Samples (count)
   │  └─ LCD Sleep (minutes)
   ├─ Calibrate
   │  ├─ Sensor (air → water or depth lookup)
   │  └─ Pump (measure actual mL output)
   ├─ Manual Water (trigger one cycle now)
   ├─ Download Log (Serial CSV dump)
   └─ Reset Defaults
```

### Data Logging
- **Circular EEPROM buffer:** ~400 entries × ~4 bytes = 2KB in EEPROM
- **Logging interval:** Configurable (default: 15 minutes)
- **Duration:** ~2.7 days of continuous monitoring at 15-min intervals before buffer wraps
- **CSV download:** Connect USB Serial Monitor @ 115200 baud, select "Download Log" → see CSV lines:
  ```
  1665.3,78,8567,0
  1680.5,79,8545,0
  ...
  ```
  (Time[hours], Moisture[%], Raw ADC, Flags)

## Building & Setup

See `build.md` for:
- Step-by-step wiring with corrected Arduino D2/D3 pins
- MOSFET circuit assembly (breadboard diagram)
- Sensor DIS pin vs. VCC-cycling explanation
- Power supply connections (dual USB+12V or single 12V)
- Safety checklist

## Calibration

### Sensor Calibration
1. Insert sensor in **air** (dry) — records ADC value (default: 11850)
2. Insert sensor in **water** at known depth (60cm recommended) — records ADC value (default: 7700)
3. Firmware maps [dry ADC → 0%, wet ADC → 100%]

See **monitor v1.5 sensor data** (from Phase 1): observed "healthy" moist soil reads 77–79% (ADC ~8567–8617)

### Pump Calibration
1. Run **Pump Calibration** wizard → pump runs for timed duration
2. Measure water collected in graduated cylinder (mL)
3. Enter mL — firmware calculates flow rate (mL/s)
4. Verifies pump is functional and rate is consistent with configuration

## Default Threshold Recommendation

Based on **monitor/data/data.csv** (Phase 1 real data):
- Healthy moist soil: 77–79% (ADC 8567–8617) over 8+ hours
- **Default:** `minThreshold = 55%` (triggers watering) → `targetHumidity = 75%` (stops when sufficiently moist)
- This provides earlier watering triggers for more frequent soil hydration, stopping at a lower moisture level

## Verification Steps

1. **Wiring test (no firmware):**
   - Connect MOSFET circuit, apply 12V
   - Briefly connect Arduino D3 to 5V → pump should spin

2. **Sensor test:**
   - Upload firmware
   - Open Serial Monitor @ 115200 baud
   - Verify ADC reads 10000–12000 in air (not 500–900, which indicates missing `analogReadResolution(14)`)

3. **Pump calibration:**
   - Run calibration wizard
   - Measure actual water output with graduated cylinder
   - Enter mL, firmware calculates rate

4. **Sensor calibration:**
   - Air (dry value) → water (wet value)
   - Verify % reads 0% in air, 100% in water

5. **Threshold test:**
   - Temporarily set `minThreshold = 70%`
   - Observe auto-watering triggers as expected
   - Reset to 40% after testing

6. **Log download:**
   - After a few watering cycles, open Serial Monitor
   - Select "Download Log" → observe CSV lines with moisture history

## Version History

See `CHANGELOG.md`

## Support

- **Wiring issues?** See `build.md` or `../WIRING_POC.md`
- **Sensor not reading?** Run Sensor Calibration wizard (Settings → Calibrate)
- **Pump not working?** Run Pump Calibration wizard to verify flow rate
- **Arduino won't upload?** Check USB cable, verify CH340 drivers (if using CH340 UART)

---

*Phase 2 v1.0 — Auto-Watering Firmware*
*Based on Phase 1 monitor v1.5 (sensor reading) + POC (watering logic)*
*Combines proven sensor code with enhanced watering reliability*
