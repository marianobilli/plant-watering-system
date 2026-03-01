# Watering System Changelog

## v1.0 (2026-02-28) — Initial Release

### Features
- **Auto-watering mode:** Continuous soil moisture monitoring with automatic pump control
- **Dry cycle logic:** Waters only when soil moisture drops below configurable threshold
- **Incremental watering:** Pumps configured mL amounts, waits for capillary absorption, checks moisture again
- **Automatic failsafe:** Stops watering if moisture doesn't increase (detects pump/sensor faults)
- **14-bit ADC sensor reading:** Corrected ADC resolution with `analogReadResolution(14)`
- **DIS pin sensor control:** Extends sensor lifespan vs. VCC-cycling in Phase 1 POC
- **Configurable median sampling:** Robust outlier rejection, adjustable sample count and delays
- **EEPROM persistence:** All watering and sensor parameters saved across power cycles
- **Circular data logging:** ~2.7 days of moisture history in EEPROM (~400 entries @ 15-min intervals)
- **Menu-driven configuration:** Up/Down/Select/Back button navigation
- **Calibration wizards:**
  - Sensor calibration: air (dry) → water (wet) or depth lookup
  - Pump calibration: measure actual mL output, firmware calculates rate
- **Serial CSV download:** Connect USB, download full moisture history as CSV
- **LCD auto-sleep:** Configurable backlight timeout to save power

### Hardware
- Arduino UNO R4 WiFi with 16x2 I2C LCD
- Capacitive soil moisture sensor with DIS pin
- 12V peristaltic pump (S-3Z model) + MOSFET (IRLZ44N) + flyback diode (1N5819)
- 4× pushbuttons for menu navigation

### Configuration Defaults
**Watering:**
- `targetHumidity = 75%` (stop watering at this level)
- `minThreshold = 55%` (start watering at this level)
- `waterAmountInitial = 50 mL` (initial pump amount)
- `waterAmountIncrement = 10 mL` (subsequent pump amounts)
- `checkInterval = 6 hours` (check dry cycle)
- `soakTime = 5 minutes` (wait after initial pump)
- `soakTimeIncrement = 1 minute` (wait between increments)
- `pumpRate = 1.67 mL/s` (typical S-3Z pump)

**Sensor:**
- `sensorDry = 11850` (14-bit ADC in air, user-calibrated)
- `sensorWet = 7700` (14-bit ADC in water @ 60cm depth)
- `sensorWarmup = 1000ms` (empirically optimal)
- `numMeasurements = 10` (samples for median)
- `measurementDelay = 100ms` (between samples)

**Display:**
- `backlightTimeout = 1 minute`

### Pin Assignments
- A0: Sensor ADC
- D2: Sensor DIS (power save)
- D3: Pump MOSFET gate
- D4-D7: Buttons UP/DOWN/SELECT/BACK
- SDA/SCL: LCD I2C

### Critical Fixes vs. POC
1. ✅ Added `analogReadResolution(14)` in setup() — POC was missing this, causing ~16× calibration error
2. ✅ Switched from VCC-cycling to DIS pin sensor control (extends sensor life)
3. ✅ Fixed pin references from ESP32 GPIO27/GPIO26/GPIO34 to Arduino D2/D3/A0
4. ✅ Median instead of mean for ADC sampling (outlier-resistant)

### EEPROM Layout
- Bytes 0-1: Magic number (0xA5C3) for config validity
- Bytes 2-31: Configuration struct (watering + sensor parameters)
- Byte 32: Header checksum
- Bytes 33-N: Circular buffer for log entries (~4 bytes each)
- Remaining EEPROM: Unused/reserved

### Documentation
- `README.md` — Feature overview, pin assignments, calibration guide
- `build.md` — Detailed wiring diagram, assembly instructions, safety checklist
- `BOM_WATERING.md` — Complete parts list with suppliers
- `CHANGELOG.md` — Version history (this file)

### Known Limitations
- Single plant only (expandable to 2-4 plants in future versions with additional MOSFET+pump channels)
- No WiFi connectivity (Arduino UNO R4 has WiFi hardware but not used in v1.0)
- EEPROM circular buffer wraps after ~2.7 days at 15-min logging interval
- Manual watering via menu currently not implemented (stub in menu structure)

### Testing Verified
- ✅ Sensor reads correct 14-bit ADC values (10000–12000 in air, ~7700 in water @ 60cm)
- ✅ Pump activation via MOSFET gate works reliably
- ✅ Dry cycle logic: watering triggers when moisture ≤ minThreshold
- ✅ Incremental watering: pumps configured mL amounts, waits soak time, re-checks
- ✅ Failsafe: detects moisture not increasing, stops pump and alerts
- ✅ Configuration persistence: EEPROM saves/loads across power cycles
- ✅ Data logging: circular buffer logs moisture history every 15 minutes
- ✅ Serial CSV download: ~400 entries readable via Terminal @ 115200 baud

### Future Enhancements (v1.1+)
- [ ] WiFi integration: send data to cloud, remote monitoring/control
- [ ] Multiple plant support (2-4 pumps on separate MOSFETs)
- [ ] MQTT support for smart home integration
- [ ] Time-based watering override (force watering at specific hours)
- [ ] Soil type profiles (sandy, loamy, clay) with auto-threshold adjustment
- [ ] Humidity/temperature sensor integration
- [ ] Low power modes for battery operation
- [ ] Graceful SD card fallback if EEPROM fills
