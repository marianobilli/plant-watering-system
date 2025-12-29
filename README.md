# DIY Plant Watering System

> An intelligent, research-based automated plant watering system designed to avoid common DIY mistakes. Features dry cycle logic, moisture propagation timing, and remote monitoring.

[![Project Status](https://img.shields.io/badge/Status-POC_Phase-yellow)]()
[![Hardware](https://img.shields.io/badge/Hardware-Arduino_UNO_R4-blue)]()
[![License](https://img.shields.io/badge/License-MIT-green)]()

---

## 📋 Overview

This project implements lessons learned from dozens of DIY plant watering projects across the maker community. Built in two phases: **POC (single plant)** → **Production (6 plants with cloud + mobile app)**.

### Key Improvements Over Typical DIY Systems

- **Capacitive sensors** (not resistive) with proper waterproofing → prevents corrosion failure
- **GPIO-powered sensors** → extends sensor life from months to years
- **Dry cycle logic** → prevents constant watering, allows natural soil drying
- **Soak time** → waits for moisture propagation before re-checking (prevents over-watering)
- **Failsafe mechanisms** → water level monitoring, post-watering verification
- **Explicit WiFi reconnection** → prevents "worked for a week then went offline" failures
- **Peristaltic pumps** → precise dosing, safe to run dry

📚 **Research:** [lessons_learned.md](lessons_learned.md)

## 🚀 Project Status

### ✅ Phase 1: Proof of Concept (POC) - **Current Phase**

Single plant system using Arduino UNO R4 WiFi, 16×2 LCD menu, breadboard assembly. Validates all watering logic before scaling to 6 plants.

**Hardware:** Arduino UNO R4 WiFi • Cytron capacitive sensor (MCP6004) • 12V peristaltic pump • IRLZ44N MOSFET • 16×2 I2C LCD • 4 buttons

**Status:** ✅ Firmware complete and ready to upload

| Documentation | Description |
|--------------|-------------|
| [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) | Complete specifications, dry cycle logic, watering algorithm |
| [BOM_POC.md](BOM_POC.md) | Parts list with links (Allegro.pl/Botland.pl) • ~550 PLN total |
| [WIRING_POC.md](WIRING_POC.md) | Pin assignments, MOSFET circuit, sensor wiring |
| [MENU_DESIGN.md](MENU_DESIGN.md) | Complete 16×2 LCD menu system and navigation |
| [src/poc/](src/poc/) | **Arduino firmware** with upload guide and usage instructions |

---

### ⏳ Phase 2: Production System (Future)

Multi-plant system with cloud connectivity and mobile app.

**Hardware:** ESP32 • 6× sensors • 6× pumps • Water level sensor • 3D printed enclosure

**Features:** Mobile app control • Arduino IoT Cloud • Multi-device management • Per-plant configuration

| Documentation | Description |
|--------------|-------------|
| [BOM_FINAL.md](BOM_FINAL.md) | 6-plant system parts list and cost analysis (~$140-240) |

## 💡 How It Works

### Intelligent Watering Algorithm

The system uses a research-based **two-phase cycle** to prevent the most common DIY watering mistakes:

**Watering Phase:** Waters in 50mL increments with 5-minute soak time until upper limit (80%) reached

**Dry Cycle Phase:** Checks every 6 hours, only waters when moisture drops to lower limit (10%)

**Key Features:**
- **Moisture Propagation Wait**: 5 minutes for capillarity after each watering
- **Incremental Watering**: Prevents flooding by watering in small amounts
- **Failsafe Protection**: Stops if moisture doesn't increase (detects failures)
- **Per-Plant Configuration**: All parameters configurable (Phase 2)

**Detailed explanation:** [WATERING_LOGIC.md](WATERING_LOGIC.md) • [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md#watering-logic-with-moisture-propagation)

### Sensor Longevity Design

Unlike typical DIY projects where sensors fail within months, this system:

- **Powers sensors from GPIO** → only energized during readings (seconds per hour vs. continuous)
- **Uses proper waterproofing** → nail polish coating prevents PCB corrosion
- **Averages 20+ readings** → reduces noise and improves accuracy

**Result:** Sensor lifespan extended from 3-6 months → 1-2+ years

**Implementation details:** [WIRING_POC.md - Soil Moisture Sensor](WIRING_POC.md#soil-moisture-sensor-wiring)

## 🛠️ Quick Start

### Building the POC (Phase 1)

1. **📦 Order Components** → See [BOM_POC.md](BOM_POC.md) for complete parts list with links (~550 PLN / ~$140 USD)

2. **🎨 Waterproof Sensor** → Apply 2 coats of clear nail polish to sensor PCB (critical for longevity!)

3. **🔌 Wire Hardware** → Follow [WIRING_POC.md](WIRING_POC.md) for complete breadboard layout and pin assignments

4. **💻 Upload Firmware** → See [src/poc/README.md](src/poc/README.md) for complete upload instructions and dependencies

5. **⚙️ Calibrate** → Use LCD menu to calibrate sensor (air + water) and pump flow rate

6. **🌱 Configure & Test** → Set target humidity, dry cycle parameters, and test manual watering

**Detailed guide:** See individual documentation files linked above for step-by-step instructions.

## 📁 Repository Structure

```
plant-watering-system/
├── 📄 README.md                 # Project overview (you are here)
├── 📄 PROJECT_OVERVIEW.md       # Detailed specs, features, best practices
├── 📄 WATERING_LOGIC.md         # Two-phase watering algorithm explained
├── 📄 lessons_learned.md        # Research findings from 50+ DIY projects
│
├── 📋 Phase 1: POC Documentation
│   ├── BOM_POC.md               # Parts list with Allegro.pl/Botland.pl links
│   ├── WIRING_POC.md            # Complete wiring guide and pin assignments
│   └── MENU_DESIGN.md           # 16×2 LCD menu system design
│
├── 📋 Phase 2: Production Documentation
│   └── BOM_FINAL.md             # 6-plant system parts and cost analysis
│
└── 💻 src/                      # Firmware
    ├── poc/                     # Phase 1: Arduino UNO R4 WiFi code (ready to upload)
    └── final/                   # Phase 2: Multi-plant with cloud (future)
```

## ⚠️ Common DIY Mistakes to Avoid

Based on research from 50+ failed DIY watering projects:

| ❌ Common Mistake | ✅ This Project's Solution |
|------------------|---------------------------|
| Resistive sensors corrode in weeks | Capacitive sensors with waterproofing |
| Sensors always powered → fail in months | GPIO-powered → only on during readings |
| Constant watering → root rot | Dry cycle logic with configurable thresholds |
| Immediate re-watering → overwatering | Soak time waits for moisture propagation |
| ESP32 WiFi doesn't reconnect | Explicit reconnection logic in firmware |
| Submersible pumps die if run dry | Peristaltic pumps safe to run dry |
| No verification if watering worked | Failsafe checks moisture increase |

📚 **Full research:** [lessons_learned.md](lessons_learned.md)

## 🤝 Contributing

This is a personal home project, but contributions are welcome! Ways to help:

- 🐛 Report issues or bugs
- 💡 Suggest improvements
- 📝 Share calibration data for different soil types
- 🔧 Contribute firmware improvements or mobile app features

---

## 📜 License

MIT License - Feel free to use, modify, and share this project.

---

## 🙏 Acknowledgments

This project builds on the collective knowledge of the maker community:
- Reddit: /r/arduino, /r/esp32, /r/homeautomation
- Hackster.io, Instructables, GitHub projects
- Maker blogs and forums

Special thanks to everyone who documented their failures and successes—your shared knowledge made this project possible.

---

## 📞 Contact & Support

- **Issues:** Use GitHub Issues for bug reports or questions
- **Discussions:** Use GitHub Discussions for general questions
- **Documentation:** All guides are in the repository (see links above)

**Ready to start?** Begin with [BOM_POC.md](BOM_POC.md) to order components, then follow [WIRING_POC.md](WIRING_POC.md) for assembly!
