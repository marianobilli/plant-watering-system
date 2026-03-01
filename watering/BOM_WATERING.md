# Bill of Materials - Plant Watering System Phase 2

Complete parts list for building a single auto-watering plant system with soil monitoring, automatic pump control, and menu-driven configuration.

## Overview
- **Arduino UNO R4 WiFi** + **I2C LCD** for user interface
- **Capacitive soil moisture sensor** with DIS pin (low-power control)
- **12V peristaltic pump** controlled via **IRLZ44N MOSFET** (logic-level gate driver)
- **Dual power supply:** USB for Arduino, 12V for pump (shared ground)
- **4× pushbuttons** for menu navigation

---

## Microcontroller & Communication

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| Arduino microcontroller | 1 | Arduino UNO R4 WiFi | 32-pin variant, 14-bit ADC, 5V logic | $35–45 |
| LCD Display | 1 | 16x2 I2C LCD (address 0x27 or 0x3F) | Blue or green backlight, 5V compatible | $8–12 |
| USB Cable | 1 | USB-A to USB-C or USB-B (check your Arduino) | For programming and power | $3–5 |
| Breadboard | 1 | 830-point solderless breadboard | For prototype wiring | $5–10 |
| Jumper wires | 1 pack | Pre-cut breadboard jumper wires (M–M, 22 AWG) | Assorted lengths | $5–8 |

**Subtotal: ~$60–80**

---

## Sensors

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| Soil moisture sensor | 1 | Capacitive moisture sensor with DIS pin (e.g., DF-Robot SEN0193, DFRobot Gravity) | 3.3–5.5V logic, analog 0–3.3V output | $15–25 |
| Sensor cable | 1 | Shielded 3-wire cable (VCC, GND, AOUT, DIS pin) | Extends from sensor to breadboard | $2–5 |

**Subtotal: ~$20–30**

---

## Pump & Water Delivery

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| Peristaltic pump | 1 | 12V DC peristaltic pump (e.g., S-3Z model, ~1.67 mL/s) | Rated for continuous operation, ~100 mL/min flow | $20–35 |
| Silicone tubing | 1 roll | 3mm ID × 5mm OD food-grade silicone tubing | 1–2 meters for water intake/outlet | $5–8 |
| Water connectors | 2 | Barbed connectors (3mm ID, brass or plastic) | For tubing connection to pump | $1–2 |
| Check valve (optional) | 1 | Inline check valve (3mm barb fittings) | Prevents water backflow from pot | $2–5 |
| Plant pot connector | 1 | Drip irrigation stake or T-connector for pot | Distributes water into soil | $1–3 |

**Subtotal: ~$30–55**

---

## Power Electronics (MOSFET Pump Control Circuit)

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| MOSFET | 1 | IRLZ44N N-Channel Logic-Level MOSFET (TO-220 package) | Gate threshold ~1.2V (compatible with Arduino 5V logic) | $1–2 |
| Flyback diode | 1 | 1N5819 Schottky Diode (1A, 40V) | Protects MOSFET from pump coil back-EMF | $0.50–1 |
| Resistor 10kΩ | 2 | 10kΩ ¼W carbon film resistors | Gate current limiter + pull-down | $0.50 |
| Heatsink (optional) | 1 | TO-220 aluminum heatsink + 2 plastic insulating pads | Thermal management for continuous 2A pump current (may not be needed for short watering cycles) | $2–5 |
| Perfboard | 1 | Small perfboard (2×3 inches) or scrap | For soldering MOSFET circuit (optional, can use breadboard) | $1–3 |

**Subtotal: ~$6–12**

---

## Power Supply

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| 12V Power Adapter | 1 | 12V 2A DC power supply (barrel connector) | For pump + tubing (~2A peak during pump run) | $10–20 |
| USB Power Adapter (optional) | 1 | Standard 5V USB power adapter | If not using computer USB for Arduino power | $5–10 |
| Barrel connector to breadboard | 1 | Male/female barrel connector breakout or scrap wire | To connect 12V adapter to breadboard GND/+12V rails | $2–3 |

**Notes:**
- **Option 1 (Dual supply):** USB power for Arduino (from computer) + separate 12V adapter for pump (shares common GND)
- **Option 2 (Single 12V):** Single 12V adapter → buck converter (12V to 5V) → Arduino VIN, AND 12V directly to pump MOSFET

**Subtotal: ~$15–35**

---

## User Interface

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| Pushbutton | 4 | 6×6×5mm tactile pushbuttons (SPST, NO) | For UP/DOWN/SELECT/BACK menu navigation | $1–2 |
| Resistor array (optional) | 1 | Pack of ¼W resistors (assorted values) | For any additional pull-up requirements (Arduino has internal pull-ups) | $1–2 |

**Subtotal: ~$2–4**

---

## Miscellaneous

| Item | Qty | Part | Notes | Est. Cost |
|------|-----|------|-------|-----------|
| Mounting hardware | 1 set | M3 plastic/brass spacers, screws, DIN rail clips | For secure Arduino + LCD mounting | $3–5 |
| Enclosure (optional) | 1 | 3D-printed or plastic box | Protects electronics from moisture/dust | $10–25 |
| Solder + flux (optional) | — | Lead-free solder, rosin core | For permanent MOSFET circuit (breadboard version works fine) | $3–5 |
| Heat-shrink tubing (optional) | 1 pack | Assorted sizes | For cable insulation | $2–3 |
| Zip ties (optional) | 1 pack | Cable ties or velcro straps | For cable management | $1–2 |

**Subtotal: ~$10–40**

---

## Grand Total

| Category | Est. Cost |
|----------|-----------|
| Microcontroller + communication | $60–80 |
| Sensors | $20–30 |
| Pump + water delivery | $30–55 |
| Power electronics | $6–12 |
| Power supply | $15–35 |
| User interface | $2–4 |
| Miscellaneous | $10–40 |
| **TOTAL** | **$143–256** |

**Typical build: ~$180–200** (with mid-range components, dual USB+12V power, no enclosure)

---

## Shopping List (Quick Reference)

### Essential (Must-Have)
1. Arduino UNO R4 WiFi ($40)
2. 16x2 I2C LCD ($10)
3. Capacitive soil moisture sensor with DIS pin ($20)
4. 12V peristaltic pump ($30)
5. IRLZ44N MOSFET ($2)
6. 1N5819 Schottky diode ($1)
7. 12V 2A power adapter ($15)
8. Breadboard + jumper wires ($10)
9. 4× pushbuttons ($2)
10. Resistors (10kΩ, can salvage) (~$0.50)

**Essential Subtotal: ~$130–145**

### Recommended (Nice-to-Have)
- Silicone tubing + connectors ($10)
- USB cable ($5)
- Mounting hardware + spacers ($5)
- Heat-shrink tubing + zip ties ($5)

### Optional (For Permanent Build)
- 3D-printed enclosure ($15–25)
- Soldering iron + solder ($30 if don't have)
- Perfboard for MOSFET circuit ($3)
- Heatsink for MOSFET ($5)

---

## Sourcing Tips

### Recommended Suppliers
- **Arduino:** Arduino.cc, Adafruit, Sparkfun, Amazon
- **Sensors/LCD:** DFRobot, Waveshare, Amazon, local electronics shops
- **MOSFET/diode:** SparkFun, Adafruit, EBay, Digi-Key (in bulk)
- **Pump:** Amazon, eBay, AliExpress (check ratings carefully)
- **Breadboard/jumpers:** Amazon, Adafruit, local electronics shops

### Cost-Saving Options
- Buy Arduino Arduino UNO R3 instead of R4 WiFi (save $10–15, but no 14-bit ADC — would need external ADC chip)
- Use basic LED/buzzer instead of LCD for status (save $8, lose menu interface)
- Manual water scheduling instead of automatic pump (save $30, add plant observation)
- Reuse MOSFET from other electronics projects (electronics junk drawer)

---

## Verification Checklist

- [ ] All components received and inspected
- [ ] Arduino programs correctly (USB cable works)
- [ ] LCD displays text at I2C address 0x27 or 0x3F (scan with I2C scanner sketch)
- [ ] Sensor reads ADC 10000–12000 in air, ~6000 in water (after `analogReadResolution(14)`)
- [ ] MOSFET gate control works (briefly connect D3 to 5V, pump should activate)
- [ ] 12V power supply outputs 12V (check with multimeter)
- [ ] Common ground connected between Arduino and 12V supply
- [ ] All wiring double-checked against `build.md` before powering on

---

## Future Expansion (Bill of Materials for Multi-Plant)

To extend to 2 plants, add:
- 1× Additional peristaltic pump ($30)
- 1× Additional IRLZ44N MOSFET ($2)
- 1× Additional 1N5819 diode ($1)
- 1× Additional soil moisture sensor ($20)
- Arduino pin D8 and D9 for second pump control
- Second 12V supply for second pump (or use same 12V with higher current capacity, e.g., 4A)

**Cost to add second plant: ~$55–70**

---

*Phase 2 v1.0 BOM — Plant Watering System*
*Generated 2026-02-28*
