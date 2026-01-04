# Soil Humidity Monitor

**1-Week EEPROM Data Logger for Plant Moisture Analysis**

A monitoring-only Arduino system designed to help you understand your plant's natural watering cycles before implementing automatic irrigation. Logs 1,344 measurements over 14 days to Arduino UNO R4 WiFi's 8KB EEPROM.

---

## Objectives & Purpose

### Why this project exists

Most DIY automatic watering systems fail not because of hardware issues, but because they use **guessed threshold values** instead of measured data. When should you water? At 20% moisture? 30%? What's your target humidity after watering? 70%? 80%? How long does water take to reach the sensor after you pour it on the soil?

**Without quantitative data, you're guessing.**

### Problem it solves

This monitoring system solves the fundamental problem of DIY irrigation: **lack of baseline data**. By logging 2 weeks of moisture measurements, you learn:

- Your plant's natural dry-down rate (how fast moisture drops)
- The moisture level where your plant shows stress (wilting, drooping)
- How long water takes to propagate through soil to the sensor
- Optimal watering thresholds for your specific plant + soil combination

### How it fits into the larger project

This monitor is **Phase 1b** of a larger plant watering automation project:

1. **Phase 1a (POC):** Single-plant automatic watering system with pump
2. **Phase 1b (Monitor):** Simplified data-collection variant **(this project)**
3. **Phase 2 (Production):** Multi-plant system with ESP32 and cloud connectivity

**Recommended workflow:**
1. Build **monitor variant** → collect 2 weeks of data
2. Analyze CSV → determine optimal `minThreshold`, `targetHumidity`, `soakTime`
3. Upgrade to **POC variant** (add pump + MOSFET circuit)
4. Configure automatic watering with measured thresholds
5. Test thoroughly before deploying

See [`../CLAUDE.md`](../CLAUDE.md) for complete project context.

---

## Key Features (v1.5)

- **Real-time ADC monitoring:** Status screen shows both moisture % and raw sensor value
- **Depth-based wet calibration:** Set wet value by insertion depth (10-60cm) without submerging in water
- **Configurable sensor parameters:** Warmup time, number of samples, measurement delays
- **Min/Max ADC tracking:** CSV export includes min/max ADC values for each measurement
- **Live calibration feedback:** Watch ADC values stabilize during calibration (updates every 0.5s)
- **Manual calibration editing:** Fine-tune dry/wet values without re-running calibration wizard
- **Expanded settings menu:** 5 configurable parameters (log interval, LCD sleep, sensor timing)

---

## Quick Start

**For detailed instructions, see [build.md](build.md) (assembly) and [user_manual.md](user_manual.md) (operation).**

### 1. Waterproof Sensor
Apply 2 coats of clear nail polish to sensor PCB, dry 24 hours.

### 2. Wire Components
Connect sensor, LCD, and 4 buttons per wiring diagram.
**→ See [build.md](build.md#wiring-diagram) for detailed connections**

### 3. Upload Firmware
Install Arduino IDE, board support, and libraries. Upload firmware.
**→ See [build.md](build.md#firmware-upload) for step-by-step instructions**

### 4. Calibrate Sensor
Run calibration wizard: Main Menu → Calibrate → Measure Now.
**→ See [user_manual.md](user_manual.md#sensor-calibration-required) for calibration methods**

### 5. Insert in Soil
Auto-logging begins! Status screen shows moisture % and raw ADC value.
**→ See [user_manual.md](user_manual.md#operating-the-system) for menu navigation**

### 6. Download Data After 2 Weeks
Connect via USB Serial, download CSV at 115200 baud.
**→ See [user_manual.md](user_manual.md#csv-download-instructions) for download methods**

---

## Practical Lessons Learned

### Optimal Sensor Warmup Time

**Finding:** Through real-world testing, **1000ms warmup time** provides the most accurate and stable readings.

**Why this matters:**
- The sensor's internal circuitry (MCP6002/MCP6004 op-amp) needs time to stabilize after being enabled
- When VCC is constantly powered and only the DIS pin is toggled, the sensor still requires warmup for the output stage to settle
- Shorter warmup times (200-500ms) can work but produce less consistent readings
- Longer warmup times (1500-2000ms) don't improve accuracy further

**Default configuration:**
- Firmware default: **1000ms**
- User-adjustable via Settings menu: 100-2000ms range
- Recommended for most applications: Keep at 1000ms

**When to adjust:**
- **Decrease to 500ms:** If you need faster logging cycles and can tolerate slight noise
- **Increase to 1500ms:** If readings seem unstable or sensor is in cold environment

### Insertion Depth Impact on Readings

**Finding:** Sensor ADC values change dramatically based on insertion depth in water, even when fully submerged.

**Measured data** (Cytron MAKER-SOIL-MOISTURE, dry value = 11,850 ADC):

| Insertion Depth | ADC Value | ADC Change from Dry | Notes |
|-----------------|-----------|---------------------|-------|
| Air (dry baseline) | 11,850 | 0 | 0% moisture (dry calibration) |
| 10cm (barely inserted) | 11,800 | -50 | Sensor mostly in air |
| 20cm | 10,000 | -1,850 | Shallow insertion |
| 30cm | 9,200 | -2,650 | Typical potted plants |
| 40cm | 8,500 | -3,350 | Deeper pots |
| 50cm | 8,000 | -3,850 | Deep containers |
| 60cm (full insertion) | 7,700 | -4,150 | 100% moisture (if used as wet calibration) |

**Key insight:** The ADC value you use for wet calibration becomes your "100% moisture" reference. If you calibrate wet at 60cm (ADC 7,700), then 7,700 = 100% wet. If you calibrate at 30cm (ADC 9,200), then 9,200 = 100% wet for your system.

**Why this matters:**

1. **Calibration consistency:** Your wet calibration must match your actual deployment depth
   - If you calibrate at 60cm depth (ADC 7,700 = 100% wet) but deploy at 30cm depth (which reads ADC 9,200), you'll see ~40% moisture when the soil is actually at 0% (completely dry)
   - **Always calibrate at the depth you'll use in soil**

2. **Depth-based calibration feature:** Firmware v1.5+ includes "Set by Depth" option
   - Navigate: Main Menu → Calibrate → Wet Value → Set by Depth
   - Select your typical planting depth (10-60cm)
   - System automatically sets appropriate wet ADC value
   - No need to physically submerge sensor in water

3. **Physical explanation:**
   - Capacitive sensors measure the dielectric constant of material between the sensing plates
   - More surface area in water = higher capacitance = lower ADC reading
   - At 10cm insertion, most of the sensor is still in air (minimal capacitance change)
   - At 60cm insertion, entire sensing area is submerged (maximum capacitance change)

4. **Soil vs water calibration:**
   - Water gives more consistent readings than saturated soil
   - Soil contains air pockets (reduces effective moisture)
   - For most accurate calibration: use water at your deployment depth
   - For field calibration: use fully saturated soil (packed, no air gaps)

**Best practices:**

- ✅ **Measure dry value once** with sensor completely in air (this doesn't change with depth)
- ✅ **Measure wet value at your deployment depth** (20-40cm typical for potted plants)
- ✅ **Insert sensor to the same depth every time** you move it
- ✅ **Use the "Set by Depth" feature** if you don't have water deep enough for full immersion
- ❌ **Don't calibrate at 60cm and deploy at 20cm** (40% moisture error!)
- ❌ **Don't submerge past the white PCB line** (damages electronics)

**Example scenario:**

You're monitoring a plant with sensor inserted **30cm deep**:
1. Calibrate dry: Hold in air → 11,850 ADC (standard)
2. Calibrate wet (Option A): Submerge to 30cm in water → 9,200 ADC
3. Calibrate wet (Option B): Main Menu → Calibrate → Wet → Set by Depth → 30cm
4. System now correctly maps: 11,850 (0% dry) to 9,200 (100% wet)
5. Moisture readings will be accurate for your 30cm deployment depth

---

## Technical Specifications (Overview)

**For detailed hardware specs, see [build.md](build.md#technical-specifications-hardware).**
**For operational specs, see [user_manual.md](user_manual.md#technical-specifications-operational).**

### Core Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Platform** | Arduino UNO R4 WiFi | Renesas RA4M1, 14-bit ADC, 8KB EEPROM |
| **Storage Capacity** | 1,344 entries | 5,376 bytes EEPROM (4 bytes/entry) |
| **Storage Duration** | 14.0 days @ 15-min | User-adjustable 1-60 min intervals |
| **ADC Resolution** | 14-bit (0-16383) | Higher resolution than classic Arduino (10-bit) |
| **Sensor Type** | Capacitive | Cytron MAKER-SOIL-MOISTURE recommended |
| **Sensor Duty Cycle** | 0.02% | Extends sensor life from 3-6 months to 1-2+ years |
| **Power** | USB 5V | ~40mA active, ~25mA idle |
| **Cost** | ~$42 USD | See [`BOM_MONITOR.md`](BOM_MONITOR.md) for parts list |

### Storage Duration by Interval

| Interval | Entries | Duration | Use Case |
|----------|---------|----------|----------|
| 10 min | 1,344 | 9.3 days | Fast data collection |
| **15 min** | **1,344** | **14.0 days** | **Default (optimal)** |
| 30 min | 1,344 | 28.0 days | Long-term trends |
| 60 min | 1,344 | 56.0 days | Extended monitoring |

### Data Export Format

CSV file via USB Serial (115200 baud) with columns:
- **Entry:** Sequential number (1-1344)
- **Timestamp_Hours:** Hours since monitoring started
- **Timestamp_Minutes:** Total minutes since start
- **Moisture_%:** Calibrated moisture (0-100%)
- **Median_ADC:** Median 14-bit ADC value from 10 samples
- **Flags:** Status bits (sensor error, out of range)

---

## Project Structure

### Documentation Files

| File | Purpose |
|------|---------|
| **[README.md](README.md)** | **This file** - Project overview, objectives, lessons learned |
| **[build.md](build.md)** | Assembly guide for hobbyists (BOM, wiring, firmware upload) |
| **[user_manual.md](user_manual.md)** | Operation guide (setup, calibration, data download, analysis) |
| **[BOM_MONITOR.md](BOM_MONITOR.md)** | Complete parts list with supplier links (~$42 USD) |
| **[CHANGELOG.md](CHANGELOG.md)** | Version history and release notes |

### Source Files

| Path | Description |
|------|-------------|
| `src/soil_humidity_monitor/soil_humidity_monitor.ino` | Main firmware (~900 lines) |
| `kicad/soil_humidity_monitor.kicad_sch` | KiCad schematic (s-expression format) |
| `kicad/schematic.png` | Schematic diagram (visual reference) |

### Parent Project

This monitoring variant is part of a larger plant watering automation project:
- **Parent project:** [`../CLAUDE.md`](../CLAUDE.md) - Complete project overview
- **POC variant:** [`../poc/`](../poc/) - Automatic watering system with pump
- **Project README:** [`../README.md`](../README.md) - Top-level project documentation

---

## Support & Feedback

- **Build questions:** See [build.md](build.md#troubleshooting-hardware--assembly)
- **Operation questions:** See [user_manual.md](user_manual.md#troubleshooting-operational-issues)
- **Firmware version:** Check Main Menu → System Info (should show v1.5)
- **Changelog:** See [`CHANGELOG.md`](CHANGELOG.md) for version history
- **Report issues:** See project repository
- **Hardware schematic:** See [`kicad/schematic.png`](kicad/schematic.png) or [`kicad/soil_humidity_monitor.kicad_sch`](kicad/soil_humidity_monitor.kicad_sch)

**Current firmware version:** v1.5 (stable)
