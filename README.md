# Arduino Automatic Plant Watering System

A modular automatic watering system with Arduino, featuring individual soil moisture monitoring, LCD menu interface, water level detection, and Arduino Cloud integration for remote monitoring and multi-device management.

## 📋 Table of Contents

- [Project Overview](#project-overview)
- [Development Stages](#development-stages)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [System Architecture](#system-architecture)
- [Wiring Diagram](#wiring-diagram)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Arduino Cloud Integration](#arduino-cloud-integration)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Future Improvements](#future-improvements)
- [License](#license)

## 🌱 Project Overview

This project evolves through three distinct stages, from proof of concept to a production-ready system with cloud connectivity and mobile app integration.

### Project Evolution

```
Proof of Concept → Prototype → Final Product
  (2 plants)      (4 plants)   (6 plants)
  Breadboard      UNO R4 WiFi   Arduino Nano
  Local Only      + Cloud       + 3D Printed
```

### Target Specifications

| Stage | Controller | Plants | Cloud | Enclosure | Status |
|-------|-----------|--------|-------|-----------|--------|
| **Stage 1: POC** | Arduino UNO R4 WiFi | 2 | No | Breadboard | In Development |
| **Stage 2: Prototype** | Arduino UNO R4 WiFi | 4 | Yes | Breadboard/Perfboard | Planned |
| **Stage 3: Final** | Arduino Nano | 6 | Yes | 3D Printed + PCB | Planned |

## 🎯 Development Stages

### Stage 1: Proof of Concept (Current)

**Goal:** Validate core functionality with minimal hardware

**Components:**
- Arduino UNO R4 WiFi
- Breadboard assembly
- 2 plants with capacitive soil sensors
- 16x2 LCD I2C display
- 3-button menu navigation
- Handmade water container
- HC-SR04 water level sensor
- 2× 12V water pumps
- 2-channel relay module

**Features:**
- Individual plant moisture monitoring
- Manual threshold configuration via LCD menu
- Automatic watering based on soil moisture
- Water tank level monitoring
- EEPROM settings persistence
- Basic pump control logic

**Success Criteria:**
- Accurate soil moisture readings
- Reliable pump activation
- Stable menu navigation
- Correct water level detection

### Stage 2: Prototype with Cloud Integration

**Goal:** Add remote monitoring and mobile app connectivity

**Additional Components:**
- Expand to 4 plants
- 4-channel relay module
- 2 additional soil sensors and pumps

**New Features:**
- **Arduino Cloud Integration:**
  - Real-time sensor data synchronization
  - Remote threshold adjustment
  - Historical data logging
- **Mobile App Features:**
  - Live moisture level viewing
  - Push notifications for low water
  - Manual watering triggers
  - Watering history and statistics
- **Multi-User Support:**
  - Multiple phones can connect to one "house"
  - A house can manage multiple watering systems
  - Shared access for family members

**Success Criteria:**
- Stable WiFi connectivity
- Real-time cloud synchronization
- Reliable push notifications
- Multi-device access working

### Stage 3: Final Production System

**Goal:** Professional, scalable solution ready for deployment

**Changes from Prototype:**
- **Controller:** Arduino Nano (smaller footprint, 6 analog pins)
- **Plants:** Scale to 6 plants
- **Enclosure:** Custom 3D-printed housing
- **Electronics:** Universal PCB board with soldered connections
- **Water Tank:** 3D-printed reservoir with integrated sensor mount
- **Assembly:** Modular design with mounting brackets for display, Arduino, sensors

**Production Features:**
- Compact, clean installation
- Easy maintenance access
- Professional appearance
- Scalable to multiple units per household
- Robust connections (no jumper wires)
- Integrated cable management

**Success Criteria:**
- Reliable operation for 30+ days
- Easy to replicate and install
- User-friendly for non-technical users
- Weatherproof/water-resistant enclosure

## ✨ Features

### Current Features (Stage 1-2)

#### Hardware Features
- **Independent Plant Control**: Each plant has:
  - Capacitive soil moisture sensor
  - Dedicated 12V water pump
  - Customizable humidity threshold (0-100%)
  - Individual watering schedule

- **User Interface:**
  - 16x2 LCD I2C display
  - 3-button menu navigation (SELECT, UP, DOWN)
  - Real-time moisture level display
  - Per-plant threshold configuration

- **Safety Features:**
  - Water tank level monitoring via ultrasonic sensor
  - Automatic pump shutoff when tank is empty
  - EEPROM storage for settings persistence
  - Relay isolation for Arduino protection

- **Water Management:**
  - Multi-level tank detection (FULL, OK, LOW, EMPTY)
  - Maximum 3.0m pumping height capability
  - 240L/h flow rate per pump

### Stage 2 Cloud Features

#### Arduino Cloud Integration
- **Real-Time Monitoring:**
  - Live soil moisture readings
  - Water tank level status
  - Pump activation history
  - System uptime tracking

- **Mobile App (Arduino IoT Cloud App):**
  - Dashboard with all plant status
  - Customizable widgets
  - Dark mode support
  - Offline data caching

- **Push Notifications:**
  - Low water level alerts
  - Plant needs watering (if moisture critically low)
  - System errors or connectivity issues
  - Daily summary reports (optional)

- **Multi-Device & Multi-System:**
  - **House Concept:** Group multiple watering systems under one "house"
  - **Shared Access:** Multiple family members can monitor/control
  - **Role Management:** Owner vs. viewer permissions
  - **Cross-Platform:** iOS and Android support

- **Data Analytics:**
  - Historical moisture trends
  - Water consumption statistics
  - Optimal watering time recommendations
  - Plant health scoring

#### Remote Control Features
- Adjust moisture thresholds from app
- Manual watering trigger
- Enable/disable individual plants
- Schedule maintenance mode

## 🔧 Hardware Requirements

### Stage 1: Proof of Concept (2 Plants)

| Component | Quantity | Specifications | Source | Est. Price (PLN) |
|-----------|----------|----------------|--------|------------------|
| Arduino UNO R4 WiFi | 1 | ABX00087 | [Botland](https://botland.com.pl/arduino-seria-podstawowa-oryginalne-plytki/23292-arduino-uno-r4-wifi-abx00087-7630049204591.html) | 129 |
| Capacitive Soil Sensor | 2 | v1.2 recommended | [Botland - Cytron](https://botland.com.pl/czujniki-wilgotnosci/21207-cytron-maker-soil-moisture-pojemnosciowy-czujnik-wilgotnosci-gleby-5904422383442.html) | 15-20 each |
| Water Pump AD20P-1230E | 2 | 12V, 300mA, 240L/h, 3m head | [Botland SKU 14873](https://botland.com.pl/pompy/14873-elektryczna-pompa-cieczy-ad20p-1230e-12v-240lh-5904422342739.html) | 20-25 each |
| 2-Channel Relay Module | 1 | 5V, 10A/250VAC, active LOW | [Botland](https://botland.com.pl/moduly-przekaznikow/1998-modul-przekaznikow-4-kanaly-styki-10a-250vac-cewka-5v.html) | 12-15 |
| LCD 16x2 I2C Display | 1 | I2C interface | Botland | 15-25 |
| HC-SR04 Ultrasonic Sensor | 1 | 2-400cm range | [Botland](https://botland.com.pl/ultradzwiekowe-czujniki-odleglosci/1420-ultradzwiekowy-czujnik-odleglosci-hc-sr04-2-200cm-justpi-5903351241366.html) | 8-12 |
| 12V 1A Power Supply | 1 | DC jack output | Botland | 20-30 |
| Tactile Push Buttons | 3 | 6x6mm | Botland | ~2 |
| 10kΩ Resistors | 3 | 1/4W THT | Botland | ~1 |
| Silicone Tubing | 2-3m | 4mm inner diameter | Botland | 3-4/m |
| Water Container | 1 | Handmade/repurposed 2-5L | - | Variable |
| Breadboard | 1 | 830 tie-points | Botland | 10-15 |
| Jumper Wires | 1 set | Male-Female, Male-Male | Botland | 5-10 |

**Stage 1 Total: ~180-220 PLN**

### Stage 2: Prototype (4 Plants)

**Additional Components:**

| Component | Quantity | Specifications | Est. Price (PLN) |
|-----------|----------|----------------|------------------|
| Capacitive Soil Sensor | +2 | v1.2 | 30-40 |
| Water Pump AD20P-1230E | +2 | 12V, 300mA | 40-50 |
| 4-Channel Relay Module | 1 | Replace 2-channel | +5 |
| 12V 2A Power Supply | 1 | Replace 1A version | +10 |
| Silicone Tubing | +2m | 4mm inner diameter | 6-8 |

**Stage 2 Additional: ~90-110 PLN**
**Stage 2 Total: ~270-330 PLN**

### Stage 3: Final Product (6 Plants)

**Component Changes:**

| Component | Quantity | Specifications | Est. Price (PLN) |
|-----------|----------|----------------|------------------|
| Arduino Nano | 1 | Replace UNO R4 WiFi | -80 (cost saving) |
| Capacitive Soil Sensor | +2 (6 total) | v1.2 | 30-40 |
| Water Pump AD20P-1230E | +2 (6 total) | 12V, 300mA | 40-50 |
| 8-Channel Relay Module | 1 | 5V (use 6 channels) | 25-30 |
| 12V 3A Power Supply | 1 | For 6 pumps (1.8A) | 35-45 |
| Universal PCB Board | 1 | 10×15cm | 8-12 |
| 3D Printed Parts | Set | Reservoir + enclosure | 50-100 (filament) |
| Mounting Hardware | Set | Standoffs, screws | 10-15 |
| Silicone Tubing | +2m (6m total) | 4mm inner diameter | 6-8 |

**Stage 3 Additional: ~130-220 PLN**
**Stage 3 Total: ~300-400 PLN**

### Pump Specifications (AD20P-1230E)

**VERIFIED SPECIFICATIONS:**
- Voltage: DC 12V
- Current: 300mA per pump
  - 2 pumps: 600mA (Stage 1)
  - 4 pumps: 1.2A (Stage 2)
  - 6 pumps: 1.8A (Stage 3)
- Flow Rate: 240L/h (4L/min)
- **Max Head: 300cm (3.0m)** ✅ Exceeds 2.5m requirement
- Cable: 160cm with 5.5x2.1mm DC jack
- IPX8 waterproof rating

## 🏗️ System Architecture

### Stage 1: Proof of Concept (2 Plants)

```
┌─────────────────────────────────────────────────────────┐
│                  ARDUINO UNO R4 WiFi                    │
│                     (WiFi unused in Stage 1)            │
│                                                         │
│  5V Rail  ─┬─ LCD VCC                                  │
│            ├─ Relay VCC                                 │
│            ├─ 3× Buttons                                │
│            └─ 2× Soil Sensors VCC                       │
│                                                         │
│  GND Rail ─┴─ Common Ground (incl. 12V PSU)            │
│                                                         │
│  I2C:                                                   │
│    A4 (SDA) ──── LCD SDA                               │
│    A5 (SCL) ──── LCD SCL                               │
│                                                         │
│  Analog Inputs:                                         │
│    A0 ──── Soil Sensor Plant 1                         │
│    A1 ──── Soil Sensor Plant 2                         │
│                                                         │
│  Digital Outputs (Relay Control):                      │
│    D3 ──── Relay IN1 (Pump 1)                         │
│    D4 ──── Relay IN2 (Pump 2)                         │
│                                                         │
│  Digital Inputs (Menu Buttons):                        │
│    D7 ──── Button SELECT                               │
│    D8 ──── Button UP                                    │
│    D9 ──── Button DOWN                                  │
│                                                         │
│  Digital I/O (Water Level):                            │
│    D10 ──── HC-SR04 TRIG                              │
│    D11 ──── HC-SR04 ECHO                              │
└─────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────┐
│              12V POWER DISTRIBUTION                      │
│                                                          │
│  12V PSU ─┬─ (+) ──── Relay COM1-2                     │
│           └─ (GND) ─┬─ Arduino GND (COMMON GROUND!)    │
│                     └─ Pump 1, 2 Negative Leads         │
│                                                          │
│  Relay NO1 ──── Pump 1 (+)                             │
│  Relay NO2 ──── Pump 2 (+)                             │
└──────────────────────────────────────────────────────────┘
```

### Stage 2: Prototype with Cloud (4 Plants)

```
┌─────────────────────────────────────────────────────────┐
│                  ARDUINO UNO R4 WiFi                    │
│                     (WiFi ACTIVE)                       │
│                                                         │
│  WiFi Module ────→ Arduino IoT Cloud                   │
│                    ├─ Dashboard                         │
│                    ├─ Mobile Apps                       │
│                    └─ Push Notifications                │
│                                                         │
│  [Same pin layout as Stage 1, plus:]                   │
│                                                         │
│  Analog Inputs:                                         │
│    A0 ──── Soil Sensor Plant 1                         │
│    A1 ──── Soil Sensor Plant 2                         │
│    A2 ──── Soil Sensor Plant 3                         │
│    A3 ──── Soil Sensor Plant 4                         │
│                                                         │
│  Digital Outputs (Relay Control):                      │
│    D3 ──── Relay IN1 (Pump 1)                         │
│    D4 ──── Relay IN2 (Pump 2)                         │
│    D5 ──── Relay IN3 (Pump 3)                         │
│    D6 ──── Relay IN4 (Pump 4)                         │
└─────────────────────────────────────────────────────────┘

                           ↕ WiFi

┌─────────────────────────────────────────────────────────┐
│              ARDUINO IoT CLOUD                          │
│                                                         │
│  House: "My Home"                                       │
│  ├─ System 1: Living Room Plants (4 plants)            │
│  └─ System 2: Balcony Plants (4 plants)                │
│                                                         │
│  Users:                                                 │
│  ├─ Owner (full control)                               │
│  ├─ Family Member 1 (view + control)                   │
│  └─ Family Member 2 (view only)                        │
└─────────────────────────────────────────────────────────┘

         ↓                ↓                ↓

    📱 Phone 1       📱 Phone 2       📱 Phone 3
    (iOS/Android)   (iOS/Android)   (iOS/Android)
```

### Stage 3: Final Product (6 Plants - Arduino Nano)

```
┌─────────────────────────────────────────────────────────┐
│                    ARDUINO NANO                         │
│                  (ESP8266/ESP32 WiFi module)            │
│                                                         │
│  UART ────→ ESP8266 WiFi Module ───→ Arduino Cloud     │
│                                                         │
│  Analog Inputs (6 plants):                             │
│    A0-A5 ──── Soil Sensors 1-6                         │
│                                                         │
│  Digital Outputs (6 relays):                           │
│    D2-D7 ──── Relay IN1-IN6 (Pumps 1-6)               │
│                                                         │
│  Digital I/O:                                          │
│    D8-D10 ──── Menu Buttons                            │
│    D11-D12 ──── HC-SR04 (TRIG, ECHO)                  │
│                                                         │
│  I2C (LCD):                                            │
│    A4 (SDA), A5 (SCL) ──── LCD Display                │
└─────────────────────────────────────────────────────────┘
```

**Note:** Arduino Nano doesn't have built-in WiFi. Options:
- Use ESP8266/ESP32 WiFi module via UART
- OR replace with Arduino Nano 33 IoT (built-in WiFi)
- OR use ESP32 directly (more analog pins)

### Button Wiring (All Stages)

```
Each button (identical wiring):

     5V ──── Button ──┬──── Arduino Pin (D7/D8/D9)
                      │
                    10kΩ Resistor
                      │
                     GND

NOT pressed = LOW (pulled to GND via resistor)
Pressed = HIGH (5V overpowers resistor)
```

## 🔌 Wiring Diagram

### Complete Pin Mapping (Stage 2 - 4 Plants)

#### Power Connections
```
Arduino 5V    → LCD VCC, Relay VCC, Button Rails, Soil Sensors VCC
Arduino GND   → LCD GND, Relay GND, Resistors, Sensors GND, 12V PSU GND
```

#### I2C (LCD Display)
```
A4 (SDA)      → LCD SDA
A5 (SCL)      → LCD SCL
```

#### Analog Inputs (Soil Sensors)
```
A0            → Plant 1 Soil Sensor AOUT
A1            → Plant 2 Soil Sensor AOUT
A2            → Plant 3 Soil Sensor AOUT
A3            → Plant 4 Soil Sensor AOUT
```

#### Digital Outputs (Pump Relays)
```
D3            → Relay Channel 1 (IN1) - Pump 1
D4            → Relay Channel 2 (IN2) - Pump 2
D5            → Relay Channel 3 (IN3) - Pump 3
D6            → Relay Channel 4 (IN4) - Pump 4
```

#### Digital Inputs (Menu Buttons)
```
D7            → SELECT Button (with 10kΩ pull-down)
D8            → UP Button (with 10kΩ pull-down)
D9            → DOWN Button (with 10kΩ pull-down)
```

#### Digital I/O (Water Level Sensor)
```
D10           → HC-SR04 TRIG
D11           → HC-SR04 ECHO
```

#### 12V Power System
```
12V PSU (+)   → Relay COM1, COM2, COM3, COM4
12V PSU (-)   → Arduino GND (COMMON GROUND CRITICAL!)
              → All pump negative leads

Relay NO1     → Pump 1 positive lead
Relay NO2     → Pump 2 positive lead
Relay NO3     → Pump 3 positive lead
Relay NO4     → Pump 4 positive lead
```

### Critical Safety Notes

⚠️ **COMMON GROUND IS MANDATORY**
The Arduino GND and 12V power supply GND MUST be connected together. Without this, the relay module will not function properly.

⚠️ **Relay Type**
Standard relay modules are **active LOW**:
- `digitalWrite(pin, LOW)` = Relay ON (pump runs)
- `digitalWrite(pin, HIGH)` = Relay OFF (pump stops)

⚠️ **Power Supply Sizing**
- Stage 1 (2 pumps): 2× 300mA = 600mA → Use 1A supply
- Stage 2 (4 pumps): 4× 300mA = 1.2A → Use 2A supply
- Stage 3 (6 pumps): 6× 300mA = 1.8A → Use 3A supply
- DO NOT power pumps from Arduino

## 💻 Software Requirements

### Arduino Libraries

Install via Arduino IDE Library Manager:

1. **LCDI2C_Multilingual** (adapted from LiquidCrystal_I2C)
   - GitHub: [locple/LCDI2C_Multilingual](https://github.com/locple/LCDI2C_Multilingual)
   - For European character support on LCD

2. **ArduinoIoTCloud** (Stage 2+)
   - Official Arduino IoT Cloud library
   - Install: Library Manager → "ArduinoIoTCloud"
   - Dependencies: Arduino_ConnectionHandler, Arduino_DebugUtils

3. **WiFiS3** (Stage 2+ for UNO R4 WiFi)
   - Built-in library for UNO R4 WiFi
   - For network connectivity

4. **EEPROM** (All Stages)
   - Built-in library
   - Already included with Arduino core

5. **DHT sensor library** by Adafruit (OPTIONAL)
   - For future temperature/humidity monitoring

### Development Environment

- **Arduino IDE 2.0+** or Arduino CLI
- **Arduino Web Editor** (alternative for cloud development)
- Board Support:
  - Stage 1-2: Arduino UNO R4 WiFi (renesas_uno)
  - Stage 3: Arduino Nano or Nano 33 IoT

### Arduino Cloud Account (Stage 2+)

1. Create account at [create.arduino.cc](https://create.arduino.cc)
2. Free tier includes:
   - Up to 2 Things (devices)
   - Up to 5 dashboards
   - 1-day data retention
3. Paid plans for extended features

## 📥 Installation

### Stage 1: Proof of Concept Setup

#### Step 1: Hardware Assembly

**Soil Sensor Placement (2 plants):**
```
- Depth: 5-10cm below soil surface (middle depth)
- Position: 5-8cm from pot edge at 45° angle toward center
- Location: Where most roots are (1/3 to 1/2 pot depth)
- Quantity: 1 sensor per plant (sufficient for pots up to 30cm diameter)
```

**IMPORTANT:** Only insert the **probe/fork** (bottom part) into soil. Keep electronics (IC, components, connector) ABOVE soil level!

**Water Level Sensor Mounting:**
```
- Mount HC-SR04 at top of water container looking down
- Sensor measures distance to water surface

Container Level Detection:
  5cm distance    = FULL
  10-15cm        = OK (pumps operational)
  20cm           = LOW warning
  25cm+          = EMPTY (STOP ALL PUMPS - safety critical)
```

**Pump Installation:**
```
- Submerge pumps in handmade water container
- Connect 4mm silicone tubing to pump outlets
- Route tubing to each plant location (max 2.5m height)
- Secure tubing to prevent movement
```

**Breadboard Assembly:**
- Follow wiring diagram for 2-plant configuration
- Use breadboard for all connections
- Keep wiring organized and labeled
- Test continuity with multimeter

#### Step 2: Software Upload (Stage 1)

```bash
# Clone repository
git clone https://github.com/YOUR_USERNAME/arduino-plant-watering-system.git
cd arduino-plant-watering-system

# Open in Arduino IDE
# File → Open → stage1_proof_of_concept/stage1_proof_of_concept.ino

# Select board
# Tools → Board → Arduino UNO R4 WiFi
# Tools → Port → [Select your Arduino port]

# Install required libraries
# Tools → Manage Libraries
# Search: "LCDI2C" → Install

# Compile and upload
# Sketch → Upload (Ctrl+U)
```

### Stage 2: Cloud Integration Setup

#### Step 1: Expand Hardware (2 → 4 Plants)

- Add 2 more soil sensors (connect to A2, A3)
- Add 2 more pumps and relay channels (D5, D6)
- Replace 2-channel relay with 4-channel
- Upgrade to 12V 2A power supply

#### Step 2: Arduino Cloud Configuration

**Create Thing:**
```
1. Login to create.arduino.cc
2. Go to IoT Cloud → Things
3. Click "Create Thing"
4. Name: "Living Room Plants" (or your location)
5. Select Device: Arduino UNO R4 WiFi
6. Associate your Arduino
```

**Add Cloud Variables:**
```cpp
// In Arduino Cloud Thing editor, add these variables:

int plant1_moisture;      // Read-only, update from device
int plant2_moisture;      // Read-only, update from device
int plant3_moisture;      // Read-only, update from device
int plant4_moisture;      // Read-only, update from device

int plant1_threshold;     // Read/Write, sync both ways
int plant2_threshold;     // Read/Write, sync both ways
int plant3_threshold;     // Read/Write, sync both ways
int plant4_threshold;     // Read/Write, sync both ways

int waterLevel;           // Read-only, % of tank capacity
bool lowWaterAlert;       // Read-only, triggers notification
String systemStatus;      // Read-only, status message

bool manualWater1;        // Read/Write, trigger from app
bool manualWater2;        // Read/Write, trigger from app
bool manualWater3;        // Read/Write, trigger from app
bool manualWater4;        // Read/Write, trigger from app
```

**Configure Network:**
```
1. In Thing settings → Network
2. Enter WiFi SSID
3. Enter WiFi Password
4. Save credentials (encrypted in Arduino)
```

**Setup Complete Code:**
```
1. Click "Sketch" tab in Thing editor
2. Copy your Stage 2 code
3. Modify with auto-generated cloud code
4. Upload from web editor OR download and use Arduino IDE
```

#### Step 3: Create Dashboard

```
1. IoT Cloud → Dashboards → Create
2. Add Widgets:
   - Gauge: Plant 1 Moisture (0-100%)
   - Gauge: Plant 2 Moisture
   - Gauge: Plant 3 Moisture
   - Gauge: Plant 4 Moisture
   - Slider: Plant 1 Threshold
   - Slider: Plant 2 Threshold
   - Slider: Plant 3 Threshold
   - Slider: Plant 4 Threshold
   - Value: Water Level
   - LED: Low Water Alert (red when low)
   - Switch: Manual Water buttons
3. Arrange layout
4. Save dashboard
```

#### Step 4: Setup Mobile App

```
1. Install "Arduino IoT Cloud" app (iOS/Android)
2. Login with your Arduino account
3. Dashboard automatically syncs
4. Enable push notifications in app settings
```

#### Step 5: Configure Notifications (Push Alerts)

**Trigger Setup:**
```
1. IoT Cloud → Things → Your Thing → Triggers
2. Create Trigger: "Low Water Alert"
   - Condition: lowWaterAlert == true
   - Action: Send notification
   - Message: "⚠️ Water tank is LOW! Refill soon."

3. Create Trigger: "Plant Critical" (optional)
   - Condition: plant1_moisture < 20
   - Action: Send notification
   - Message: "🌱 Plant 1 critically dry!"
```

#### Step 6: Multi-Device Setup

**Share Access:**
```
1. Dashboard → Share
2. Add email addresses of family members
3. Choose permission level:
   - Can Edit: Full control
   - Can View: Read-only
4. Recipients get email invitation
5. They login and see shared dashboard
```

**Multiple Watering Systems in One House:**
```
House Organization:
├── Living Room System (Thing 1)
│   ├── 4 plants
│   └── Dashboard 1
└── Balcony System (Thing 2)
    ├── 4 plants
    └── Dashboard 2

All visible in same Arduino Cloud account!
All users can access all systems if shared.
```

### Stage 3: Final Product Assembly

#### Hardware Changes:

**PCB Soldering:**
```
1. Design PCB layout or use universal perfboard
2. Solder Arduino Nano socket
3. Solder relay module connections
4. Solder button inputs with pull-down resistors
5. Solder screw terminals for:
   - Soil sensors (6× 3-pin)
   - Pumps (6× 2-pin via relay)
   - Power input (12V)
   - LCD connector (I2C)
   - Water sensor (4-pin)
6. Test all connections before final assembly
```

**3D Printing:**
```
Files needed:
1. water_reservoir.stl (5-10L capacity with sensor mount)
2. electronics_enclosure.stl (fits Nano + PCB + display)
3. pump_holder.stl (secure pumps in reservoir)
4. display_bezel.stl (front panel mount)
5. mounting_bracket.stl (wall/shelf mount)

Print settings:
- Material: PLA or PETG
- Layer height: 0.2mm
- Infill: 20-30%
- Supports: Yes (for overhangs)
- Water reservoir: 100% infill bottom layer (waterproof)
```

**Final Assembly:**
```
1. Mount Arduino Nano on PCB
2. Install PCB in enclosure with standoffs
3. Mount LCD in front bezel
4. Install buttons in enclosure front panel
5. Connect ribbon cables from PCB to LCD, buttons
6. Mount water sensor in reservoir lid
7. Install pumps in reservoir with holder
8. Route tubing through enclosure exit ports
9. Final wiring check and power-on test
```

## ⚙️ Configuration

### Initial Calibration

#### 1. Soil Sensor Calibration

Each sensor needs calibration for your specific soil type:

```cpp
// In code, measure values:
// 1. Sensor in dry soil    → Note value (e.g., 850)
// 2. Sensor in wet soil    → Note value (e.g., 400)
// 3. Calculate range and map to 0-100%

int moistureValue = analogRead(A0);
int moisturePercent = map(moistureValue, 850, 400, 0, 100);
moisturePercent = constrain(moisturePercent, 0, 100);
```

#### 2. Water Tank Calibration

```cpp
// Measure distances with empty and full tank:
// Empty tank: 25cm
// Full tank: 5cm

// Set thresholds in code:
#define TANK_EMPTY_CM 25
#define TANK_LOW_CM 20
#define TANK_OK_CM 15
#define TANK_FULL_CM 5
```

### Menu System Usage

**Navigation:**
- **UP Button**: Increase value / Move up menu
- **DOWN Button**: Decrease value / Move down menu
- **SELECT Button**: Confirm selection / Enter submenu

**Menu Structure (Stage 1):**
```
Main Menu
├── Plant 1 Settings
│   ├── View Current Moisture
│   ├── Set Threshold (0-100%)
│   └── Manual Water Test
├── Plant 2 Settings
├── Water Tank Status
└── System Info
```

**Menu Structure (Stage 2):**
```
Main Menu
├── Plant 1-4 Settings (same as above)
├── Water Tank Status
├── Cloud Status
│   ├── WiFi Connection
│   ├── Cloud Sync Status
│   └── Last Update Time
└── System Info
```

### EEPROM Memory Map

Settings are stored in EEPROM and survive power loss:

**Stage 1-2 (4 plants):**
```
Address 0: Plant 1 Threshold (0-100)
Address 1: Plant 2 Threshold (0-100)
Address 2: Plant 3 Threshold (0-100)
Address 3: Plant 4 Threshold (0-100)
Address 4: System Flags
Address 5-9: Reserved
```

**Stage 3 (6 plants):**
```
Address 0-5: Plant 1-6 Thresholds
Address 6: System Flags
Address 7-9: Reserved
```

## ☁️ Arduino Cloud Integration

### Cloud Variables Behavior

**Automatic Synchronization:**
- Sensor readings pushed every 30 seconds
- Threshold changes from app applied immediately
- Manual watering triggers activate within 1-2 seconds

**Offline Handling:**
- Local operation continues if WiFi lost
- Settings from EEPROM used
- Cloud syncs when connection restored

### Push Notification Examples

**Low Water Alert:**
```
Title: Water Tank Low
Message: Living Room Plants - Water level at 15%. Please refill.
Priority: High
Sound: Enabled
```

**Critical Plant Alert:**
```
Title: Plant Needs Water
Message: Plant 2 moisture at 18% (threshold 55%). Manual check recommended.
Priority: Normal
```

**Daily Summary (Optional):**
```
Title: Daily Plant Report
Message: All 4 plants watered 3 times today. Tank at 60%. System healthy.
Priority: Low
Time: 8:00 PM daily
```

### Multi-User Scenarios

**Scenario 1: Family Sharing**
```
Parents + 2 kids all have Arduino app
All can see plant status
Kids have view-only access
Parents can adjust thresholds
Any phone gets low water alerts
```

**Scenario 2: Multiple Systems**
```
One house has:
- Living room system (4 plants)
- Balcony system (4 plants)
- Bedroom system (2 plants)

All in same Arduino Cloud account
Each is separate "Thing"
Master dashboard shows all
Individual dashboards for each system
```

**Scenario 3: Remote Monitoring**
```
On vacation abroad
Check plants via mobile data
Adjust thresholds if needed
Get alerts if issues arise
Ask neighbor to refill if tank low
```

## 🚀 Usage

### Daily Operation

1. **System starts** → Reads thresholds from EEPROM
2. **WiFi connects** (Stage 2+) → Syncs with Arduino Cloud
3. **Main loop** checks every 30 seconds:
   - Read all soil sensors
   - Check water tank level
   - Compare moisture to thresholds
   - Activate pumps if needed (and tank not empty)
   - Update cloud variables
4. **LCD displays** current status
5. **App shows** live data on dashboard

### Watering Logic

```
FOR each plant:
    IF soil_moisture < threshold AND tank_level > EMPTY:
        Turn ON pump
        Run for 3 seconds
        Turn OFF pump
        Update cloud: last_watering_time
        Wait 60 seconds before next check
    ENDIF

    IF manual_water_button pressed (from app):
        Override threshold
        Water for 3 seconds
        Acknowledge to app
    ENDIF
ENDFOR
```

### LCD Display Screens

**Screen 1: Overview**
```
P1:45% T:60 P2:78%
P3:55% T:55 P4:62%
```

**Screen 2: Water Status**
```
Tank: OK (15cm)
WiFi: Connected
```

**Screen 3: Active Watering**
```
WATERING PLANT 2
Cloud updated ✓
```

### Mobile App Dashboard Layout

```
┌─────────────────────────────────┐
│  Living Room Plants      🔔 ON  │
├─────────────────────────────────┤
│                                 │
│  🌱 Plant 1      [====    ] 45% │
│     Threshold: ▓▓▓▓▓▓░░░░ 60%   │
│     [💧 Water Now]              │
│                                 │
│  🌱 Plant 2      [========] 78% │
│     Threshold: ▓▓▓▓▓░░░░░ 55%   │
│                                 │
│  🌱 Plant 3      [=====   ] 55% │
│  🌱 Plant 4      [======  ] 62% │
│                                 │
├─────────────────────────────────┤
│  💧 Water Tank:  [=======] 70%  │
│  🔗 Status: Connected            │
│  ⏱️ Updated: 5 seconds ago      │
└─────────────────────────────────┘
```

## 🔧 Troubleshooting

### Hardware Issues

#### Relay doesn't click when Arduino pin goes LOW

**Solution:**
- Check common ground connection between Arduino and 12V supply
- Verify relay module is 5V type (not 12V)
- Confirm relay input pins connected correctly

#### Pump doesn't run when relay activates

**Solution:**
- Verify 12V power supply is ON
- Check pump connections to relay NO (Normally Open) terminals
- Confirm all pump negative leads connected to 12V GND
- Test pump directly with 12V supply

#### Soil sensor readings erratic or stuck at max/min

**Solution:**
- Check sensor VCC and GND connections
- Verify analog pin connections
- Clean sensor surface with soft cloth
- Recalibrate for your soil type
- Replace sensor if corroded (capacitive sensors rarely corrode)

#### LCD shows garbled text or nothing

**Solution:**
- Check I2C address (common: 0x27 or 0x3F)
- Verify SDA → A4, SCL → A5 connections
- Run I2C scanner sketch to detect address
- Adjust LCD contrast potentiometer

#### Water tank reads EMPTY when full

**Solution:**
- Check HC-SR04 TRIG and ECHO pin connections
- Verify sensor faces directly down at water
- Adjust distance thresholds in code
- Clean ultrasonic sensor lenses

### Software Issues

#### Settings reset after power loss

**Solution:**
- Check EEPROM write function is called after changes
- Verify EEPROM library is included
- Add delay after EEPROM write (5-10ms)

### Cloud/Network Issues (Stage 2+)

#### WiFi won't connect

**Solution:**
```
1. Check SSID and password in Thing settings
2. Verify WiFi is 2.4GHz (not 5GHz)
3. Check router firewall settings
4. Test with phone hotspot to isolate issue
5. Serial monitor shows connection attempts
```

#### Cloud variables not updating

**Solution:**
```
1. Check Thing status: Green = connected
2. Verify variables are marked "Read and Write" or "Read Only"
3. Check variable types match (int, bool, String)
4. Ensure ArduinoCloud.update() in loop()
5. Check Arduino Cloud status page for outages
```

#### Push notifications not received

**Solution:**
```
1. Enable notifications in Arduino app settings
2. Check phone notification permissions
3. Verify trigger conditions are correct
4. Test trigger manually in cloud interface
5. Ensure app is running in background (iOS)
```

#### Multiple users can't see dashboard

**Solution:**
```
1. Verify dashboard is shared (not Thing)
2. Check recipient accepted email invitation
3. Confirm they logged in with invited email
4. Try re-sending invitation
5. Check shared user's permission level
```

#### High latency or slow updates

**Solution:**
```
1. Reduce cloud update frequency (30s minimum recommended)
2. Use WiFi closer to router
3. Upgrade to Arduino Cloud paid plan (faster sync)
4. Minimize number of variables synced
5. Check internet connection speed
```

## 🔮 Future Improvements

### Planned Features

- [ ] **Stage 1 Completion** (POC - 2 Plants)
  - Basic watering system functional
  - LCD menu working
  - EEPROM storage tested
  - Water level safety validated

- [ ] **Stage 2 Implementation** (Cloud - 4 Plants)
  - WiFi connectivity stable
  - Arduino Cloud integration
  - Mobile app dashboard
  - Push notifications working
  - Multi-user testing

- [ ] **Stage 3 Development** (Final Product - 6 Plants)
  - 3D printed enclosure designed
  - PCB layout finalized
  - Arduino Nano migration
  - Professional assembly
  - User manual created

### Advanced Features (Post-Stage 3)

- [ ] **Enhanced Scheduling**
  - Time-based watering schedules
  - Multiple sessions per day
  - Seasonal adjustment
  - Vacation mode

- [ ] **Additional Sensors**
  - DHT11/22 for air temperature/humidity
  - Light sensor for optimal watering times
  - Water quality monitoring (pH, EC)
  - Temperature compensation for moisture readings

- [ ] **Pump Control Refinements**
  - PWM speed control for gentle watering
  - Variable watering duration per plant
  - Drip mode vs. flood mode
  - Flow rate monitoring

- [ ] **Data Analytics**
  - Soil moisture trends over time
  - Water consumption statistics
  - Optimal watering time recommendations
  - Plant health scoring algorithm
  - Export data to CSV/Excel

- [ ] **Voice Assistant Integration**
  - Amazon Alexa skill
  - Google Home integration
  - Voice commands: "Water plant 1"
  - Status queries: "How is plant 2?"

- [ ] **Advanced Alerts**
  - Predictive alerts (will need water in X hours)
  - Pump failure detection
  - Unusual moisture drop alerts
  - Tank refill reminders based on usage

## 📊 Scalability Options

### Expanding Beyond 6 Plants

| Method | Max Plants | Hardware | Cost | Complexity |
|--------|-----------|----------|------|-----------|
| **Current (Nano)** | 6 | Arduino Nano | Baseline | Simple |
| **Arduino Mega** | 16 | Mega 2560 | +$15-30 | Simple |
| **Analog Multiplexer** | 32+ | CD74HC4067 | +$2 each | Medium |
| **I2C Soil Sensors** | Unlimited | I2C sensors | $8-12/sensor | Easy |
| **Multiple Systems** | Unlimited | Duplicate units | Full cost | Complex |

**Recommendation for Large Installations:**
- 1-6 plants: Arduino Nano (Stage 3 design)
- 7-16 plants: Arduino Mega 2560
- 17+ plants: Multiple systems in Arduino Cloud "House"

## 📝 Code Structure

```
plant-watering-system/
├── README.md                          # This file
├── stage1_proof_of_concept/
│   ├── stage1_proof_of_concept.ino   # Main sketch (2 plants, no WiFi)
│   ├── config.h                       # Pin definitions
│   ├── sensors.h                      # Sensor functions
│   ├── pumps.h                        # Pump control
│   ├── lcd_menu.h                     # Menu system
│   ├── eeprom_manager.h              # Settings storage
│   └── water_tank.h                  # Tank monitoring
│
├── stage2_cloud_prototype/
│   ├── stage2_cloud_prototype.ino    # Main sketch (4 plants, WiFi)
│   ├── thingProperties.h             # Auto-generated by Arduino Cloud
│   ├── arduino_secrets.h             # WiFi credentials (gitignored)
│   ├── cloud_functions.h             # Cloud sync logic
│   └── [same .h files as Stage 1]
│
├── stage3_final_product/
│   ├── stage3_final_product.ino      # Production code (6 plants)
│   ├── [WiFi module code for Nano]
│   └── [optimized versions of Stage 2 files]
│
├── hardware/
│   ├── pcb/
│   │   ├── schematic.pdf
│   │   └── pcb_layout.pdf
│   ├── 3d_models/
│   │   ├── water_reservoir.stl
│   │   ├── electronics_enclosure.stl
│   │   ├── pump_holder.stl
│   │   ├── display_bezel.stl
│   │   └── mounting_bracket.stl
│   └── wiring_diagrams/
│       ├── stage1_breadboard.png
│       ├── stage2_prototype.png
│       └── stage3_pcb.png
│
├── docs/
│   ├── assembly_guide.md
│   ├── calibration_guide.md
│   ├── cloud_setup.md
│   └── troubleshooting.md
│
└── tools/
    ├── i2c_scanner/                  # Find LCD address
    ├── sensor_calibration/           # Calibration helper
    └── eeprom_reset/                 # Factory reset
```

## 📄 License

MIT License - Feel free to modify and distribute

## 👤 Author

Created for Arduino plant watering automation project.

**Project Timeline:**
- Stage 1 (POC): Development started December 2024
- Stage 2 (Cloud): Planned Q1 2025
- Stage 3 (Final): Planned Q2 2025

## 🙏 Acknowledgments

- Arduino community for extensive documentation
- Arduino IoT Cloud team for excellent platform
- Botland.pl for component availability in Poland
- LCDI2C_Multilingual library maintainers
- All contributors to this project

## 📞 Support

For issues, questions, or contributions:
- Open an issue on GitHub
- Submit pull requests for improvements
- Share your build photos and modifications!
- Tag @YOUR_HANDLE on social media

## 🔗 Useful Links

- [Arduino UNO R4 WiFi Documentation](https://docs.arduino.cc/hardware/uno-r4-wifi)
- [Arduino IoT Cloud Guide](https://docs.arduino.cc/arduino-cloud/)
- [Arduino Cloud Free vs Paid Plans](https://cloud.arduino.cc/plans)
- [Capacitive Soil Sensor Guide](https://how2electronics.com/capacitive-soil-moisture-sensor-arduino/)
- [Project Blog](#) (coming soon)

---

**Last Updated:** December 2024
**Current Stage:** Stage 1 (Proof of Concept)
**Arduino Version:** UNO R4 WiFi
**Project Status:** ✅ Documentation Complete | 🔄 Stage 1 Development In Progress

**Ready to build?** Start with Stage 1 and expand as you gain confidence!
