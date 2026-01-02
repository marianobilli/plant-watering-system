# Changelog - Soil Humidity Monitor

All notable changes to the monitoring variant firmware will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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
