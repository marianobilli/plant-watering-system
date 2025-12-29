# Bill of Materials - Final Product (Phase 2)

## Core Components

| Qty | Component | Specifications | Est. Price (USD) | Link | Notes |
|-----|-----------|----------------|------------------|------|-------|
| 1 | Microcontroller | ESP32-DevKitC or Arduino Nano ESP32 | $6-12 | [Add link] | **Cost comparison needed** |
| 6 | Capacitive Soil Moisture Sensors | 3.3-5V, waterproofed | $18-30 | [Add link] | TLC555 chip, sealed |
| 6 | Peristaltic Pumps | 12V, ~100mL/min | $48-72 | [Add link] | Self-priming, tubing included |
| 1 | Water Level Sensor | HC-SR04 ultrasonic or VL53L0X ToF | $3-8 | [Add link] | Mount on reservoir lid |
| 1 | 6-Channel Relay Module | 5V/12V, optocoupler isolation | $8-12 | [Add link] | Or 6x individual relay modules |
| 6 | 1N4007 Diodes | 1A, 1000V | $0.60 | [Add link] | Flyback protection per pump |
| 1 | 12V Power Supply | 12V 5A (60W) minimum | $12-18 | [Add link] | Powers all pumps + buck converter |
| 1 | Buck Converter | 12V to 5V, 3A | $3-5 | [Add link] | Powers ESP32 from 12V supply |
| 1 | Prototyping PCB | 10x15cm or larger | $3-5 | [Add link] | Permanent assembly |
| - | Silicone Tubing | 5-6mm ID, 10-15 meters | $10-15 | [Add link] | For 6 pumps + distribution |
| - | Wire Kit | 22-24 AWG, various colors | $8-12 | [Add link] | Permanent wiring |
| 1 | 3D Printed Enclosure | PLA/PETG filament | $5-15 | [Add link] | STL files to be designed |
| 1 | 3D Printed Reservoir | PETG (food-safe), ~3-5L | $8-20 | [Add link] | Watertight design |

---

## Optional Components (Recommended)

| Qty | Component | Purpose | Est. Price (USD) |
|-----|-----------|---------|------------------|
| 6 | Status LEDs | Per-pump operation indicator | $1-2 |
| 6 | 220Ω Resistors | LED current limiting | $0.60 |
| 1 | Buzzer (5V) | Local audio alerts | $1-2 |
| 1 | LCD Display (16x2 I2C) | Local status display (optional) | $4-6 |
| 1 | Reset Button | Manual reset | $0.50 |
| 1 | Leak Sensor | Water detection | $3-5 |
| - | Cable Glands | IP65 rated for enclosure | $5-10 |
| - | Desiccant Packets | Moisture control in enclosure | $3-5 |
| - | Heat Shrink Tubing Kit | Wire protection and labeling | $8-12 |

---

## Total Estimated Cost: $140-240 USD per device

**Note:** Cost varies significantly based on:
- ESP32 vs Arduino Nano ESP32 choice
- Sensor quality (generic vs branded)
- 3D printing vs purchased enclosure
- Optional features included

---

## Microcontroller Comparison (ESP32 vs Arduino Nano ESP32)

| Feature | ESP32-DevKitC | Arduino Nano ESP32 | Winner |
|---------|---------------|-------------------|--------|
| Price | $6-8 | $10-12 | ESP32 |
| GPIO Pins | 34 (enough for 6 sensors + pumps) | 14 digital + 8 analog | ESP32 |
| ADC Channels | 18 (use ADC1 for WiFi compatibility) | 8 | ESP32 |
| WiFi/BLE | Yes (dual-mode) | Yes (dual-mode) | Tie |
| Power Consumption | 160mA active, 10µA deep sleep | Similar | Tie |
| Programming | Arduino IDE, ESP-IDF | Arduino IDE | Nano (easier) |
| Form Factor | Larger (~55x28mm) | Smaller (~45x18mm) | Nano |
| Community Support | Massive | Growing | ESP32 |
| Arduino IoT Cloud | Supported via ESP32 core | Native support | Nano (easier) |

**Recommendation:**
- **ESP32-DevKitC** for cost-effectiveness and flexibility
- **Arduino Nano ESP32** if Arduino IoT Cloud integration is priority and budget allows

---

## Water Level Sensor Comparison

| Sensor Type | HC-SR04 Ultrasonic | VL53L0X Laser ToF | Float Switch |
|-------------|-------------------|-------------------|--------------|
| Price | $2-3 | $5-8 | $3-5 |
| Accuracy | ±3mm | ±1mm | On/Off only |
| Range | 2cm - 4m | 3cm - 2m | Fixed level |
| Waterproofing | Moderate (top-mount) | Good (top-mount) | Excellent |
| False Readings | Can occur with splashing | Rare | None |
| Power | 15mA active | 10mA active | Passive |
| **Recommendation** | Good for POC | Best for production | Backup only |

**Recommended:** VL53L0X for accurate reservoir monitoring, HC-SR04 as budget alternative

---

## Power Supply Sizing

### Power Requirements Calculation:
- ESP32: 160mA average (WiFi active), 240mA peak
- 6x Peristaltic Pumps: 150mA each @ 12V = 900mA peak (if all run simultaneously)
- 6x Relays: 70mA each @ 5V = 420mA peak
- Sensors: ~50mA total
- LEDs/Buzzer: ~100mA
- **Total peak draw:** ~1.5A @ 12V + 0.8A @ 5V

### Recommended Supply:
- **12V 5A (60W)** power supply with overhead
- **Buck converter:** 12V → 5V @ 3A (for ESP32 + relays + sensors)
- **Alternative:** Separate 5V 3A supply + 12V 2A supply

### Deep Sleep Mode (Battery Operation - Optional):
- ESP32 deep sleep: 10-15µA
- Sensors powered off: 0µA
- Wake every 2 hours to check moisture
- **Estimated battery life:** 6-12 months on 3x 18650 (3.7V 3000mAh) with proper power management

---

## 3D Printing Requirements

### Enclosure Design Goals:
- IP65-rated (dust/splash proof)
- Ventilation for heat dissipation
- Cable gland entry points
- Mounting holes for PCB standoffs
- LCD window (if using display)
- LED light pipes for status indicators

### Reservoir Design Goals:
- 3-5L capacity (holds ~500mL per plant × 6 plants × 2 waterings)
- Opaque walls (prevent algae growth)
- Mounting point for water level sensor on lid
- 6x outlet ports (1 per pump intake)
- Removable lid for easy refilling
- Drain plug at bottom for cleaning

### Filament Requirements:
- **Enclosure:** PLA (easiest) or PETG (better durability)
- **Reservoir:** PETG (food-safe, watertight) or ABS with sealant
- **Estimated filament:** 500g-1kg depending on design

**Note:** STL files to be designed after POC validation

---

## Cloud Platform Comparison

| Platform | Arduino IoT Cloud | Blynk | ThingSpeak | AWS IoT Core |
|----------|------------------|-------|------------|--------------|
| Cost | Free tier: 2 devices | Free tier limited | Free (non-commercial) | Pay-per-use |
| Mobile App | Official app included | Custom app builder | Web dashboard only | Custom development |
| Multi-device | Yes | Yes | Yes | Yes |
| Push Notifications | Yes | Yes | Limited | Via SNS |
| Ease of Setup | High (native Arduino) | High | Medium | Low |
| **Recommendation** | **Best for this project** | Good alternative | Data logging focus | Overkill |

**Recommended:** Arduino IoT Cloud for easiest integration with multi-device support

---

## Component Wiring Summary (6 Plants)

### ESP32 Pin Allocation:
| Function | GPIO Pins | Qty | Notes |
|----------|-----------|-----|-------|
| Moisture Sensors (ADC) | GPIO32-37 (ADC1) | 6 | Avoid ADC2 with WiFi |
| Sensor Power Control | GPIO13-18 | 6 | Or use analog mux |
| Pump Relays | GPIO19, 21, 22, 23, 25, 26 | 6 | 5V relay control |
| Water Level Sensor | GPIO4 (Trig), GPIO5 (Echo) | 2 | HC-SR04 |
| I2C (optional LCD) | GPIO21 (SDA), GPIO22 (SCL) | 2 | Shared bus |
| Status LEDs | GPIO12, 14, 27, 33, 15, 2 | 6 | Optional |

**Total pins used:** ~28 GPIO (ESP32-DevKitC has 34 usable GPIO)

---

## Instructions for Filling Out This Document

**Please confirm your preferences:**

1. **Microcontroller:** ESP32-DevKitC or Arduino Nano ESP32?
2. **Water Level Sensor:** HC-SR04 or VL53L0X?
3. **Optional LCD Display:** Include in final product? (Yes/No)
4. **3D Printing:** Have access to 3D printer? (Yes/No)
5. **Cloud Platform:** Arduino IoT Cloud or alternative preference?

**Provide component links and prices from preferred suppliers once POC is validated and ready for final build.**
