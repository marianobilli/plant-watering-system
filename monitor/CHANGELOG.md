# Changelog - Soil Humidity Monitor

All notable changes to the monitoring variant firmware will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [1.1] - 2026-01-02

### Added
- **Live calibration feedback:** ADC value now updates every 0.5 seconds during calibration wizard, allowing users to watch sensor stabilize before capturing values
- **Manual calibration editing:** New "Edit Values" submenu under Calibrate menu for fine-tuning dry/wet ADC values without re-running full calibration wizard
- **Raw ADC display:** Status screen now shows both moisture percentage and raw 14-bit ADC value (`M:XX% ADC:XXXXX`)
- **Enhanced System Info:** System Info menu now displays current calibration values (Dry, Wet) and log interval setting
- **Troubleshooting guidance:** Added documentation for unstable ADC readings and manual calibration value copying

### Changed
- **Calibration UX improvement:** Calibration screens now show live ADC readings during air/water steps instead of static instructions
- **Manual edit precision:** UP/DOWN buttons adjust calibration values by ±1 instead of ±50 for more precise fine-tuning
- **Code organization:** Version information centralized to single constants (`VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_STRING`)
- **Menu structure:** Calibrate menu now has two options: "Run Wizard" (calibration wizard) and "Edit Values" (manual editing)

### Fixed
- Calibration values now displayed immediately after calibration completes, showing both dry and wet values on confirmation screen

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
