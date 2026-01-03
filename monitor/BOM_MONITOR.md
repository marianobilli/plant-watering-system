# Bill of Materials - Soil Humidity Monitor

**Simplified monitoring variant for 2-week data collection**

This is a subset of the POC system - removed pump, MOSFET circuit, SD card, and 12V power supply.

---

## Core Components

| Item | Quantity | Specification | Purpose | Est. Price (USD) |
|------|----------|---------------|---------|------------------|
| **Arduino UNO R4 WiFi** | 1 | Renesas RA4M1, 14-bit ADC, 8KB EEPROM, 5V logic | Main controller + EEPROM storage | $27.50 |
| **Capacitive Soil Moisture Sensor** | 1 | **Must have DIS pin** (Cytron MAKER-SOIL-MOISTURE or equivalent), analog output | Measures soil moisture (0-100%) | $3.00 |
| **16×2 I2C LCD Display** | 1 | PCF8574 I2C backpack, 0x27 or 0x3F address, 5V | Shows moisture + menu + entry count | $3.50 |
| **Tactile Push Buttons** | 4 | 6mm momentary, 4-pin, breadboard-compatible | Menu navigation (UP/DOWN/SELECT/BACK) | $0.50 |
| **Breadboard** | 1 | 830-point half-size or full-size | Prototyping platform | $3.00 |
| **Jumper Wires** | 1 set | Male-to-male, 20-30 pieces, various lengths | Connections | $2.00 |
| **USB-C Cable** | 1 | Data + power (not charge-only), 1-2m | Power + CSV download via Serial | $3.00 |

**Total Estimated Cost: ~$42.50 USD** (excluding optional items)

---

## Optional Components

| Item | Quantity | Purpose | Est. Price (USD) |
|------|----------|---------|------------------|
| **Clear Nail Polish** | 1 bottle | Waterproof sensor PCB (2 coats) - extends life 4-8× | $2.00 |
| **Small Container** | 1 | Hold water for sensor calibration | $1.00 |

---

## Component Details & Specifications

### Arduino UNO R4 WiFi
- **Critical Features:**
  - 14-bit ADC (0-16383 range) - higher resolution than classic Arduino
  - 8KB EEPROM - stores 1,344 entries (14 days @ 15-min intervals)
  - 5V logic - works directly with LCD and sensors (no level shifting)
  - Built-in pull-up resistors - no external resistors needed for buttons
- **Where to Buy:**
  - Official Arduino Store
  - DigiKey, Mouser, SparkFun
  - Amazon (verify authentic Arduino, not clones)
- **Part Number:** ABX00087

### Capacitive Soil Moisture Sensor
- **Recommended:** Cytron MAKER-SOIL-MOISTURE (SKU: SEN-MOIST)
- **Why Capacitive:** Corrosion-resistant (vs resistive sensors that fail in weeks)
- **CRITICAL REQUIREMENT (v1.4+):** Must have 4-pin interface with **DIS (disable) pin**
  - 3-pin sensors (VCC/GND/OUT only) are **NOT compatible** with v1.4 firmware
  - DIS pin used for GPIO-controlled power management (extends sensor life 4-8×)
- **Specifications:**
  - 4-pin interface: VCC, GND, OUT, **DIS**
  - Analog output (0-3.3V or 0-5V depending on VCC)
  - Operating voltage: 3.3V-5V (recommended 5V for full ADC range)
  - Typical dry value: ~12400 (14-bit ADC)
  - Typical wet value: ~6000 (14-bit ADC)
  - DIS pin: HIGH = disabled (0.14mA sleep), LOW = enabled (3.6mA active)
- **CRITICAL:** Waterproof with clear nail polish before use!
- **Where to Buy:**
  - Cytron official store (confirmed DIS pin support)
  - AliExpress, eBay (search "v1.2 capacitive soil moisture 4-pin DIS")
  - Amazon (verify 4-pin interface with DIS pin, not 3-pin)

### 16×2 I2C LCD Display
- **Specifications:**
  - 2 rows × 16 characters
  - Blue backlight with white/black characters (most common)
  - I2C interface via PCF8574 backpack (reduces wiring to 4 pins)
  - Default address: 0x27 or 0x3F (scannable via I2C scanner)
  - Operating voltage: 5V
- **Where to Buy:**
  - AliExpress, eBay (search "1602 I2C LCD")
  - Amazon, DigiKey, Adafruit
- **Note:** Includes I2C backpack pre-soldered (verify before purchasing)

### Tactile Push Buttons
- **Specifications:**
  - 6mm × 6mm momentary push button
  - 4-pin (2 pins per side, internally connected)
  - Breadboard-compatible (0.1" spacing)
  - Tactile feedback
- **Where to Buy:**
  - AliExpress, eBay (buy 10-50 pack, they're cheap)
  - Amazon, DigiKey, SparkFun
- **Note:** No external pull-up resistors needed (Arduino has internal pull-ups)

---

## Comparison to POC Variant

**Removed Components (from POC):**
- ❌ 12V Peristaltic Pump ($8-12)
- ❌ IRLZ44N MOSFET ($0.50)
- ❌ 1N5819 Flyback Diode ($0.30)
- ❌ 2× 10kΩ Resistors ($0.10)
- ❌ SD Card Module ($2.00)
- ❌ SD Card ($3-5)
- ❌ 12V 2A Power Supply ($6-8)

**Cost Savings: ~$20-28** compared to POC variant

**Added Functionality (vs POC):**
- ✅ CSV download via USB Serial (replaces SD card)
- ✅ Circular buffer auto-overwrite (no manual data management)
- ✅ 14-day continuous logging in EEPROM
- ✅ Real-time entry count on status screen
- ✅ Progress bar during CSV download

---

## Shopping List Template

**Quick Copy-Paste for Online Stores:**

```
1× Arduino UNO R4 WiFi (ABX00087)
1× Capacitive Soil Moisture Sensor (4-pin with DIS, Cytron MAKER-SOIL-MOISTURE)
1× 16×2 I2C LCD Display (blue backlight, PCF8574)
4× 6mm Tactile Push Buttons (momentary, 4-pin)
1× 830-point Breadboard
1× Jumper Wire Kit (male-to-male, 20+ pieces)
1× USB-C Data Cable (1-2m)
1× Clear Nail Polish (for sensor waterproofing)
```

---

## Purchasing Tips

1. **Arduino Authenticity:** Buy from authorized distributors to avoid clones with fake chips
2. **Sensor Version:** Verify it has **4-pin interface with DIS pin** - 3-pin sensors NOT compatible with v1.4+
3. **LCD I2C:** Ensure I2C backpack is **pre-soldered** (saves you soldering work)
4. **USB Cable:** Must support **data transfer**, not just charging (verify "data cable")
5. **Buttons:** Buy extra (10-20 pack) - they're cheap and useful for future projects

---

## Regional Availability

### United States
- **Arduino, LCD, Buttons:** Adafruit, SparkFun, DigiKey, Mouser
- **Sensors:** Amazon, eBay, AliExpress
- **Shipping:** 2-5 days (US stores), 2-4 weeks (AliExpress)

### Europe
- **Arduino:** Official Arduino Store (Italy), local electronics shops
- **All Components:** AliExpress, eBay
- **Shipping:** 1-3 days (EU stores), 2-4 weeks (AliExpress)

### Asia
- **All Components:** Local electronics markets, Taobao, AliExpress
- **Shipping:** Same-day to 1 week

---

## Storage Duration vs. Logging Interval

Current configuration (15-minute intervals):
- **Storage:** 1,344 entries in 5,376 bytes
- **Duration:** Exactly 14 days (336 hours)
- **EEPROM Usage:** 66% (2,784 bytes unused)

Alternative configurations:

| Interval | Entries | Duration | EEPROM Usage | Use Case |
|----------|---------|----------|--------------|----------|
| 10 min | 1,344 | 9.3 days | 66% | Faster data collection |
| 15 min | 1,344 | 14.0 days | 66% | **Default (optimal)** |
| 30 min | 1,344 | 28.0 days | 66% | Long-term trends |
| 60 min | 1,344 | 56.0 days | 66% | Extended monitoring |

**User-adjustable via Settings menu (1-60 minutes)**

---

## Maintenance & Consumables

**Expected Lifespan:**

| Component | Lifespan | Notes |
|-----------|----------|-------|
| Arduino | 10+ years | No moving parts, solid-state |
| LCD | 5-10 years | Backlight may dim over time |
| Buttons | 100K+ presses | Mechanical wear, easily replaceable |
| Sensor (waterproofed) | 1-2 years | Electrolysis if constantly powered |
| Sensor (DIS-controlled) | 2-4 years | Extended by 15-min duty cycle (v1.4+) |
| USB Cable | 3-5 years | Connector wear from plugging/unplugging |

**No recurring costs** (no pump consumables, no power supply replacement)

---

## Assembly Difficulty

- **Skill Level:** Beginner (no soldering required)
- **Assembly Time:** 15-30 minutes
- **Tools Needed:** None (breadboard prototyping)
- **Soldering:** Optional (only if LCD doesn't have I2C backpack pre-soldered)

---

## Next Steps After Purchase

1. **Waterproof sensor:** Apply 2 coats clear nail polish, let dry 24 hours
2. **Upload firmware:** Flash `soil_humidity_monitor.ino` via Arduino IDE
3. **Calibrate sensor:** Main Menu → Calibrate → Sensor (air → water)
4. **Insert in soil:** Begin 2-week monitoring session
5. **Download data:** Serial Monitor @ 115200 baud, save CSV output
6. **Analyze:** Import CSV to Excel/Python to understand plant's natural watering cycles

**See README.md for detailed usage workflow**
