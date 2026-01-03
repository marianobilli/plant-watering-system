# Changelog - Soil Humidity Monitor

All notable changes to the monitoring variant firmware will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [1.4.0] - 2026-01-04

### Changed
- **Sensor control method**: Switched from VCC GPIO power cycling to DIS pin GPIO control
  - Sensor VCC now connects to 5V (constant power) instead of D2
  - D2 now controls DIS pin: HIGH = disabled (sleep), LOW = enabled (active)
  - **INVERTED LOGIC** compared to previous power control (v1.0-1.3)
- **Pin definition rename**: `SENSOR_POWER_PIN` → `SENSOR_DISABLE_PIN` for clarity
- **Wiring update**: Sensor DIS pin connects to D2, VCC connects to 5V rail
- **Warmup time reduced**: 1000ms → 500ms (VCC already stable, faster sensor response)

### Benefits
- Faster sensor wake-up (no VCC capacitor charging delay)
- More stable power supply (VCC always connected to 5V rail)
- Sensor retains internal state between readings (improved consistency)
- Same longevity benefits (0.02% duty cycle still applies, extends sensor life 4-8×)

### Migration Notes (IMPORTANT!)

**Hardware change required:**
1. REMOVE: Sensor VCC from D2
2. ADD: Sensor VCC to Arduino 5V
3. REMOVE: Sensor DIS from GND
4. ADD: Sensor DIS to Arduino D2

**Firmware incompatible with old wiring:**
- v1.4 firmware + old wiring (VCC on D2) = WRONG READINGS
- v1.0-1.3 firmware + new wiring (DIS on D2) = WRONG READINGS
- Must upgrade firmware AND rewire sensor together

**Configuration preserved:**
- EEPROM configuration remains compatible (same 32-byte header)
- Calibration values preserved (no need to recalibrate unless ADC characteristics change)
- Existing logged data can be downloaded before upgrade

### Technical Details
- Sensor disable current: 0.14mA (per Cytron MAKER-SOIL-MOISTURE datasheet)
- Sensor active current: 3.6mA (per datasheet)
- Duty cycle unchanged: 0.02% (1 second per 15 minutes at 500ms warmup)
- Average current: ~0.15mA (negligible increase from previous 0.008mA)

---

## [1.3.0] - 2026-01-03

### Changed
- **Memory optimization**: Removed min/max ADC values from EEPROM storage (saves 4 bytes per entry)
- **Storage capacity doubled**: 672 → 1,344 entries (7 days → 14 days @ 15-min intervals)
- **Log entry size**: Reduced from 8 to 4 bytes (moisture %, median ADC, flags only)
- **CSV format simplified**: Removed `Min_ADC` and `Max_ADC` columns (6 columns instead of 8)
- **Updated sensor defaults**: Warmup 500ms → 1000ms, samples 20 → 10, delay 10ms → 100ms (improved stability)

### Technical Details
- **Entry format**: 4 bytes (1 moisture + 2 median ADC + 1 flags)
- **Storage capacity**: 1,344 entries × 4 bytes = 5,376 bytes
- **Duration @ 15-min intervals**: 14.0 days (was 7.0 days)
- **Min/max ADC**: Still calculated and displayed in live Serial Monitor output, but not stored in EEPROM or CSV export
- **EEPROM write reduction**: 50% fewer bytes written per log interval (extends EEPROM lifespan)

### Benefits
- 2× longer monitoring periods without downloading data
- Simpler CSV format for analysis (fewer columns)
- Reduced EEPROM wear (50% fewer writes per entry)
- Min/max still available for real-time debugging via Serial Monitor (115200 baud)

---

## [1.2.0] - 2026-01-03

### Added
- **Configurable sensor parameters** (via Settings menu):
  - Backlight timeout: 1-60 minutes (was hardcoded to 1 min)
  - Sensor warmup delay: 100-2000 ms in 100ms increments
  - Number of measurements: 5-50 samples for median calculation
  - Measurement delay: 10-500 ms in 50ms increments
- **Min/Max ADC tracking**: CSV export now includes min/max ADC values per entry
- **Enhanced Settings menu**: Expanded from 1 to 5 configurable parameters

### Changed
- **Log entry size**: Expanded from 4 to 8 bytes to store min/max ADC
- **Storage capacity**: Reduced from 1,344 to 672 entries (14 days → 7 days @ 15-min intervals)
- **CSV format**: Added `Min_ADC` and `Max_ADC` columns, renamed `Raw_ADC` to `Median_ADC`
- **Median calculation**: Now handles odd/even sample counts correctly

### Removed
- **5 discarded samples**: No longer discard initial readings (faster, configurable warmup instead)

### Fixed
- **Memory efficiency**: Dynamic sample allocation (saves RAM with fewer samples)
- **Median calculation**: Handles odd sample counts (not just even)
- **Display refresh timing**: Timer now resets AFTER sensor reading completes (ensures consistent 2-second intervals)
- **Sample delay increment**: Corrected to 10ms steps (was incorrectly 50ms)

---

## [1.1] - 2026-01-02

### Added
- **Live calibration feedback:** ADC value now updates every 0.5 seconds during calibration wizard, allowing users to watch sensor stabilize before capturing values
- **Independent dry/wet calibration:** Calibrate menu restructured to allow separate calibration of dry and wet values
  - Each value (dry/wet) has its own submenu with "Measure Now" (automatic) and "Edit Manually" options
  - Users can recalibrate just dry OR just wet without redoing both
  - **New menu structure:** Calibrate → Dry Value/Wet Value → Measure Now/Edit Manually
- **Raw ADC display:** Status screen now shows both moisture percentage and raw 14-bit ADC value (`M:XX% ADC:XXXXX`)
- **Enhanced System Info:** System Info menu now displays current calibration values (Dry, Wet) and log interval setting
- **Troubleshooting guidance:** Added documentation for unstable ADC readings and manual calibration value copying

### Changed
- **Calibration UX improvement:** Calibration screens now show live ADC readings during measurement instead of static instructions
- **Manual edit step size:** UP/DOWN buttons now adjust calibration values by ±50 with auto-rounding to nearest multiple of 50
  - When entering manual edit mode, current value is automatically rounded to nearest 50
  - Each UP/DOWN press adjusts by exactly 50 (e.g., 11350 → 11400 → 11450)
  - Results in cleaner calibration values (11350 instead of 11332)
- **Code organization:** Version information centralized to single constants (`VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_STRING`)
- **Flexible calibration workflow:** No longer forced to calibrate both dry AND wet in sequence - calibrate only what you need
- **Sensor reading stability:** Increased warmup delay from 200ms to 500ms and discard first 5 unstable samples before averaging

### Fixed
- **Calibration confirmation:** Both measurement AND manual edit now show "Saved! D:XXXXX W:XXXXX" confirmation screen after saving
  - Previously only measurement showed confirmation, manual edit returned directly to menu
  - Provides consistent user feedback for all calibration methods
- **Improved data accuracy:** Sensor readings now discard initial unstable samples after power-on, preventing inaccurate logged values during automatic logging cycles
- **CRITICAL: ADC resolution configuration** - Added `analogReadResolution(14)` in setup() to enable 14-bit ADC mode (Arduino UNO R4 WiFi defaults to 10-bit for compatibility)
  - **Symptom:** Users experienced calibration values 16× too low (e.g., Dry=560 instead of 12,400)
  - **Fix:** Firmware now explicitly configures 14-bit mode; existing users must re-upload and re-calibrate
- **Documentation:** Added Cytron MAKER-SOIL-MOISTURE sensor specifications and DIS pin wiring requirements to README.md

---

## [1.0] - 2024-12-31

### Added
- Initial release of monitoring variant firmware
- 1,344-entry circular buffer in EEPROM (14 days @ 15-min intervals)
- 15-minute default logging interval (user-adjustable 1-60 minutes)
- CSV data download via Serial Monitor (115200 baud)
- Sensor calibration wizard (dry air + wet water/soil)
- Settings menu for log interval adjustment
- LCD backlight auto-sleep (1-minute timeout)
- System Info screen showing entry count and firmware version
- Clear Data function with confirmation dialog
- Reset to Defaults function with confirmation dialog
- GPIO-powered sensor control (extends sensor lifespan 4-8×)
- Automatic circular buffer overwrite when full
- EEPROM configuration persistence (survives power cycles)
- 14-bit ADC support for Arduino UNO R4 WiFi
- Serial debugging output (moisture readings, log events)

### Technical Details
- **Storage:** 5,376 bytes EEPROM (32-byte header + 1,344×4-byte entries)
- **ADC Resolution:** 14-bit (0-16383) for Arduino UNO R4 WiFi
- **Sensor Power Duty Cycle:** 0.02% (powered only during readings)
- **Display Update Rate:** 2 seconds when backlight active
- **Button Debouncing:** 200ms
- **Averaging:** 20 ADC samples per reading (reduces noise)

---

## Version Numbering

Version format: `MAJOR.MINOR`

- **MAJOR version:** Incompatible EEPROM format changes or major feature overhauls
- **MINOR version:** New features, improvements, bug fixes (backward compatible)

**EEPROM compatibility:**
- v1.x uses magic number `0xA5C3` and can read each other's configurations
- Firmware version stored in EEPROM header for diagnostics
