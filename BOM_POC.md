# Bill of Materials - POC (Phase 1)

**Note:** This POC uses the **Arduino UNO R4 WiFi** (already owned in GeeekPi kit) to start immediately without waiting for additional components. The final 6-plant system (Phase 2) will use ESP32 for lower cost and power consumption.

## Components Purchased / Already Owned

### Main POC Hardware

| Qty | Component | Specifications | Price (PLN) | Status | Link | Notes |
|-----|-----------|----------------|-------------|--------|------|-------|
| 1 | ✅ **Arduino UNO R4 WiFi** | Renesas RA4M1, WiFi, 5V logic | Included in kit (~408 PLN) | ✅ Owned | [GeeekPi Kit](https://www.amazon.com/GeeekPi-Arduino-Development-Starter-Tutorial/dp/B0CSLF61WF) | **POC Microcontroller** |
| 1 | ✅ Peristaltic Pump | 12V dosing pump | ~25 PLN | ✅ Owned | [Allegro](https://allegro.pl/oferta/pompa-perystaltyczna-dozujaca-12v-wyprzedaz-12598165922) | Self-priming, can run dry |
| 5 | ✅ IRLZ44N MOSFET | N-channel logic-level, 55V, 47A, TO-220 | ~8 PLN (5pcs) | ✅ Owned | [Botland](https://botland.com.pl/n-mosfet/1950-n-mosfet-irlz44n-55v47a-tht-5szt-5904422373276.html) | Logic-level, works with 5V |
| 10 | ✅ 1N5819 Schottky Diode | 40V, 1A | ~3 PLN (10pcs) | ✅ Owned | [Botland](https://botland.com.pl/diody-schottky/306-dioda-schottky-1n5819-1a-40v-10szt-5903351244480.html) | Flyback protection |
| 1 | ✅ Cytron Soil Moisture Sensor | MCP6004 op-amp, 1.8-6V, analog | ~22 PLN | ✅ Owned | [Allegro](https://allegro.pl/oferta/maker-soil-moisture-sensor-pojemnosciowy-czujnik-wilgotnosci-gleby-18074990169) | **MUST waterproof!** |
| 1 | ✅ Resistor Kit | 600pcs THT 0.25W 1/4W, 20×30 values | ~17 PLN | ✅ Owned | [Allegro](https://allegro.pl/oferta/zestaw-600-rezystorow-tht-opornik-0-25w-1-4w-1-20x30-roznych-wartosci-17379583399) | Includes 10kΩ for MOSFET gate |
| 1 | ✅ LCD Display 1602 | 16x2 I2C LCD | Included in kit | ✅ Owned | From GeeekPi kit | **16×2 size** |
| 4+ | ✅ Push Buttons | Tactile switches | Included in kit | ✅ Owned | From GeeekPi kit | Menu navigation |
| 1 | ❌ SD Card Module | SPI interface, microSD slot | ~8 PLN | Need to buy | [Botland](https://botland.com.pl/moduly-pamieci/2275-modul-z-czytnikiem-kart-microsd-spi-5904422311038.html) | For monitor mode data logging |
| 1 | ❌ microSD Card | 4GB-32GB, any brand | ~15 PLN | Need to buy | Local electronics store | Data storage |
| 1 | ✅ Breadboard | 830 tie-points | Included in kit | ✅ Owned | From GeeekPi kit | For POC prototyping |
| 1 | ✅ Jumper Wire Kit | M-M, M-F, F-F | Included in kit | ✅ Owned | From GeeekPi kit | Extensive assortment |
| Various | ✅ LEDs & Resistors | Status indicators | Included in kit | ✅ Owned | From GeeekPi kit | Optional for debugging |

### Additional Components Already Owned
| Qty | Component | Specifications | Status | Notes |
|-----|-----------|----------------|--------|-------|
| 1 | ✅ 12V Power Supply | 12V 2A AC/DC adapter | ✅ Owned | For pump power |

## Still Need to Purchase for POC

### Consumables (Purchase Before Assembly)
| Qty | Component | Specifications | Est. Price (PLN) | Notes |
|-----|-----------|----------------|------------------|-------|
| 1 | ❗ Clear Nail Polish | Any brand | ~5-10 PLN | **Sensor waterproofing - CRITICAL!** |
| 1 | ✅ Small Container | ~500mL-1L plastic bottle | ~0 PLN | Water reservoir (reuse existing) |

### Not Needed for POC (Phase 2 Only)
| Qty | Component | Specifications | Est. Price (PLN) | Link | Notes |
|-----|-----------|----------------|------------------|------|-------|
| - | ❌ ESP32 DevKit | ESP-WROOM-32, USB-C, CH340, WiFi+BT, 4MB | ~35 PLN | [Allegro](https://allegro.pl/oferta/mikrokontroler-esp-wroom-32-esp32-usb-c-ch340-wifi-bt-esp-32-4mb-do-arduino-12575360436) | **Not needed for POC - using Arduino UNO R4 WiFi instead** |
| - | ❌ ESP32 Pin Adapter | 30-pin screw terminal shield | ~20 PLN | [Allegro](https://allegro.pl/oferta/adapter-do-esp32-30-pin-devkit-v1-z-zaciskami-ark-terminal-goldpiny-shield-17825539542) | **Not needed for POC** |

---

## Total Cost Summary

**Already Purchased/Owned:** ~495 PLN
- GeeekPi Arduino UNO R4 WiFi kit: ~408 PLN (includes Arduino, LCD, buttons, breadboard, wires)
- Pump: 25 PLN
- MOSFETs: 8 PLN
- Diodes: 3 PLN
- Sensor: 22 PLN
- Resistors: 17 PLN
- Power supply: ~12 PLN (estimated)

**Still Need:** ~5-10 PLN (just nail polish for sensor waterproofing!)

**Total POC Investment:** ~500-505 PLN (~$125-130 USD)

**Note:** Using the Arduino UNO R4 WiFi allows immediate project start without waiting for ESP32 delivery. The Arduino's 5V logic actually works better with the LCD and sensor than 3.3V, and it has more stable WiFi reconnection than ESP32. The Phase 2 production system will use ESP32 for lower per-unit cost when scaling to 6 plants.

---

## Sensor Waterproofing Supplies (Choose One Method)

| Method | Materials | Cost | Difficulty |
|--------|-----------|------|------------|
| Nail Polish | Clear nail polish, 2 coats | $2-4 | Easy |
| PlastiDip | PlastiDip spray can | $8-12 | Easy |
| Heat Shrink | 3:1 adhesive-lined heat shrink + heat gun | $10-15 | Medium |
| Hot Glue | Hot glue gun + glue sticks | $5-8 | Easy |

**Recommended:** Nail polish (cheapest, easy, effective for POC)

---

## Component Selection Notes

### Arduino UNO R4 WiFi (POC Microcontroller)
- **Why this board for POC:**
  - WiFi built-in (Renesas RA4M1 + ESP32-S3 for WiFi)
  - 5V logic - works perfectly with LCD and sensors without level shifting
  - More stable WiFi reconnection than standalone ESP32
  - Already owned (no additional purchase needed)
  - Easier programming with Arduino IDE (familiar environment)
  - **No ADC2/WiFi conflict** (unlike ESP32)
- **Specifications:**
  - 6 analog inputs (A0-A5) - sufficient for single plant POC
  - 14 digital I/O pins
  - 32KB SRAM, 256KB flash
  - USB-C connector
  - Built-in LED matrix (can display status)
- **Limitations vs ESP32:**
  - Higher power consumption (~100mA idle vs ESP32 ~80mA)
  - Fewer analog pins (6 vs 18)
  - Not ideal for battery operation
  - Higher cost per unit (good for POC, expensive for 6-plant production)

### Why ESP32 for Phase 2 (Production)
- **Lower cost:** ~$6-8 vs ~$25-30 for Arduino UNO R4
- **More ADC pins:** 18 analog channels (need 6 for sensors + 1 for water level)
- **Lower power:** Better for always-on operation
- **Smaller form factor:** Easier to fit in custom enclosure
- **Phase 2 only:** POC validates logic, then migrate firmware to ESP32

### IRLZ44N MOSFET
- **Logic-level:** Works with 3.3V GPIO from ESP32 (critical!)
- **Specs:** 55V, 47A - massively oversized for 12V pump, but ensures cool operation
- **Advantages over relay:** No clicking noise, faster switching, longer life, no coil current draw
- **Schottky diode (1N5819):** Better than 1N4007 for this application - lower forward voltage drop (0.4V vs 0.7V), faster switching
- **Circuit:**
  ```
  ESP32 GPIO → 10kΩ resistor → MOSFET Gate
  MOSFET Gate → 10kΩ pull-down resistor → GND (prevents floating gate)
  MOSFET Drain → Pump negative
  MOSFET Source → GND
  Pump positive → 12V+
  1N5819 diode: Cathode to 12V+, Anode to Pump negative (flyback protection)
  ```

### Cytron Capacitive Soil Moisture Sensor (MCP6004-based)
- **Why this sensor is superior:**
  - MCP6004 quad op-amp (proper analog signal processing, not cheap NE555 timer)
  - Works from 1.8V-6V (perfect for both 3.3V ESP32 and 5V systems)
  - Rail-to-rail precision analog output
  - Low power consumption (~170µA)
  - More stable readings, less drift than generic sensors
  - Expected lifespan: 1-2+ years when properly waterproofed
- **Waterproofing is mandatory** - coat electronics with nail polish, PlastiDip, or heat-shrink
- **Power from GPIO:** Use ESP32 GPIO pin to power sensor only during readings (extends sensor life)
- **Cytron brand:** Reputable Malaysian manufacturer (better QC than generic Chinese sensors)

### LCD Display
- **20x4 preferred** for more menu information display
- **I2C interface** (PCF8574) saves GPIO pins vs parallel connection
- **Alternative:** 16x2 I2C (cheaper) works but cramped for complex menus
- **Voltage:** Most I2C LCDs work with both 5V and 3.3V logic (verify before purchase)

### Peristaltic Pump (12V from Allegro)
- **Self-priming:** No need to fill tubing manually
- **Can run dry:** Safe if water reservoir empties (unlike submersible pumps)
- **Flow rate:** Typical ~100mL/min - check actual specs
- **Tubing:** Should come with silicone tubing, verify inner diameter matches
- **Power:** 12V recommended over 5V for better torque and reliability

### Power Supply
- **12V 2A:** Powers pump with headroom
- **ESP32 power:** USB cable during development, or add buck converter (12V → 5V → USB port)
- **Shared ground:** **Critical** - 12V GND and ESP32 GND must be connected for MOSFET to work
- **Separate supplies:** Keep ESP32 on USB power during POC for easy programming/debugging

---

## Optional Components (Nice to Have)

| Component | Purpose | Price |
|-----------|---------|-------|
| Potentiometer 10kΩ | LCD contrast adjustment | $0.50 |
| LEDs (3mm or 5mm) | Status indicators | $0.20 each |
| 220Ω Resistors | LED current limiting | $0.10 each |
| Buzzer (5V active) | Audio alerts | $1-2 |
| Project enclosure | Protect electronics | $8-15 |

---

## Tools Needed

- Soldering iron + solder (if not using pre-assembled modules)
- Wire strippers
- Small screwdriver set
- Multimeter (helpful for debugging)
- Heat gun (if using heat-shrink waterproofing)

---

## Instructions for Filling Out This Document

**Please provide links and actual prices for the following components from your preferred suppliers:**

1. Arduino Uno R4 WiFi
2. Capacitive soil moisture sensor (specify brand/model if possible)
3. LCD Display (20x4 I2C or 16x2 I2C - your preference)
4. Peristaltic pump (specify voltage and flow rate)
5. Relay module
6. Other components as available

**Preferred suppliers:**
- [ ] Amazon
- [ ] AliExpress
- [ ] Local electronics store
- [ ] Other: ___________

Once links are added, I'll calculate the exact total cost and create a shopping list.
