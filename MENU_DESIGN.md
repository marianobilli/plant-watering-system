# LCD Menu System Design - POC

## Button Layout
- **UP:** Navigate up through menu items
- **DOWN:** Navigate down through menu items
- **SELECT:** Enter submenu or confirm selection
- **BACK:** Return to previous menu or cancel

## Menu Structure

```
MAIN MENU
├── 1. Current Status
│   ├── Display: Moisture: XX%
│   ├── Display: Target: XX%
│   ├── Display: Status: [Dry Cycle/Ready/Watering]
│   └── [BACK to return]
│
├── 2. Manual Water
│   ├── Confirm: "Water now?"
│   ├── [SELECT] → Execute watering
│   └── [BACK] → Cancel
│
├── 3. Settings
│   ├── 3.1 Target Humidity
│   │   ├── Display current: "Target: XX%"
│   │   ├── [UP/DOWN] Adjust (0-100%)
│   │   ├── [SELECT] Save
│   │   └── [BACK] Cancel
│   │
│   ├── 3.2 Min Threshold
│   │   ├── Display current: "Min: XX%"
│   │   ├── [UP/DOWN] Adjust (0-100%)
│   │   ├── [SELECT] Save
│   │   └── [BACK] Cancel
│   │
│   ├── 3.3 Water Amount
│   │   ├── Display current: "Amount: X.Xs" or "XXmL"
│   │   ├── [UP/DOWN] Adjust
│   │   ├── [SELECT] Save
│   │   └── [BACK] Cancel
│   │
│   ├── 3.4 Check Interval
│   │   ├── Display current: "Check: XXh"
│   │   ├── [UP/DOWN] Adjust (1-24 hours)
│   │   ├── [SELECT] Save
│   │   └── [BACK] Cancel
│   │
│   ├── 3.5 Soak Time
│   │   ├── Display current: "Soak: XXm"
│   │   ├── [UP/DOWN] Adjust (1-30 minutes)
│   │   ├── [SELECT] Save
│   │   └── [BACK] Cancel
│   │
│   ├── 3.6 Dry Cycle
│   │   ├── Display current: "Dry Cycle: ON/OFF"
│   │   ├── [UP/DOWN] Toggle
│   │   ├── [SELECT] Save
│   │   └── [BACK] Cancel
│   │
│   └── 3.7 Back to Main
│
├── 4. Calibration
│   ├── 4.1 Calibrate Sensor
│   │   ├── Step 1: "Sensor in air"
│   │   │   ├── [SELECT] Read dry value
│   │   │   └── Display: "Dry: XXXX"
│   │   ├── Step 2: "Sensor in water"
│   │   │   ├── [SELECT] Read wet value
│   │   │   └── Display: "Wet: XXXX"
│   │   ├── [SELECT] Save calibration
│   │   └── [BACK] Cancel
│   │
│   ├── 4.2 Calibrate Pump
│   │   ├── Instructions: "Place tube in measuring cup"
│   │   ├── "Pump for 10s"
│   │   ├── [SELECT] Run pump
│   │   ├── Input: "Enter mL:"
│   │   │   ├── [UP/DOWN] Adjust value
│   │   │   └── [SELECT] Calculate mL/s
│   │   └── [BACK] Cancel
│   │
│   └── 4.3 Back to Main
│
├── 5. System Info
│   ├── Display: Firmware version
│   ├── Display: Uptime
│   ├── Display: WiFi status (future)
│   └── [BACK] Return
│
└── 6. Reset Settings
    ├── Confirm: "Reset to defaults?"
    ├── [SELECT] → Reset all
    └── [BACK] → Cancel
```

---

## Screen Layout Examples (16×2 LCD) - **YOUR DISPLAY**

**Note:** These layouts are optimized for your 16×2 I2C LCD from the GeeekPi kit.

### Main Menu - Idle Screen (Auto-cycling display)
The idle screen cycles through information every 3 seconds:

**Screen 1: Moisture Status**
```
┌────────────────┐
│Moist:75% T:80% │ ← Current moisture : Target
│Status:DRY CYCLE│ ← Current state
└────────────────┘
```

**Screen 2: Next Check Time**
```
┌────────────────┐
│Moist:75% T:80% │
│Next chk: 1h30m │ ← Time until next check
└────────────────┘
```

**Screen 3: Last Watering**
```
┌────────────────┐
│Moist:75% T:80% │
│Last:2h ago 5mL │ ← Time since last watering + amount
└────────────────┘
```

### Main Menu - Navigation
```
┌────────────────┐
│>Status  Manual │ ← Selected item has '>'
│ Config   Cal   │ ← Settings=Config, Calibration=Cal
└────────────────┘

Press UP/DOWN to move selection:
┌────────────────┐
│ Status >Manual │
│ Config   Cal   │
└────────────────┘

Continue scrolling:
┌────────────────┐
│ Manual >Config │
│ Cal    Info    │
└────────────────┘
```

### 1. Current Status - Detailed View
**Page 1:**
```
┌────────────────┐
│Current: 75%    │
│Target:  80%    │
└────────────────┘
```

**Page 2:** (auto-advance after 2 seconds)
```
┌────────────────┐
│Min Thr: 20%    │ ← Dry cycle minimum
│Dry Cyc: ON     │
└────────────────┘
```

**Page 3:**
```
┌────────────────┐
│Check:   2h     │ ← Check interval
│Amount:  10mL   │ ← Watering amount
└────────────────┘
```

### 2. Manual Water - Confirmation
```
┌────────────────┐
│Water now?      │
│SEL=Yes BCK=No  │
└────────────────┘
```

**During watering:**
```
┌────────────────┐
│WATERING...     │
│Time: 3.5s      │
└────────────────┘
```

**After watering:**
```
┌────────────────┐
│Done! 75%->82%  │ ← Before -> After
│Press any key   │
└────────────────┘
```

### 3. Settings Menu
**Main settings menu:**
```
┌────────────────┐
│>Target   Min   │ ← Target Humidity, Min Threshold
│ Amount  Check  │
└────────────────┘

Next page:
┌────────────────┐
│>Soak    DryCyc │ ← Soak Time, Dry Cycle
│ Back           │
└────────────────┘
```

### 3.1 Settings - Target Humidity
```
┌────────────────┐
│Target:    80%  │
│UP/DN SEL=Save  │
└────────────────┘

While adjusting (blinks):
┌────────────────┐
│Target:   [85%] │ ← Brackets show editing
│UP/DN SEL=Save  │
└────────────────┘
```

### 3.2 Settings - Min Threshold
```
┌────────────────┐
│Min Thr:   20%  │
│UP/DN SEL=Save  │
└────────────────┘
```

### 3.3 Settings - Water Amount
```
┌────────────────┐
│Amount:   10mL  │
│UP/DN SEL=Save  │
└────────────────┘
```

### 3.4 Settings - Check Interval
```
┌────────────────┐
│Check:     2h   │
│UP/DN SEL=Save  │
└────────────────┘
```

### 3.5 Settings - Soak Time
```
┌────────────────┐
│Soak:      5m   │
│UP/DN SEL=Save  │
└────────────────┘
```

### 3.6 Settings - Dry Cycle
```
┌────────────────┐
│Dry Cycle: ON   │
│UP/DN SEL=Save  │
└────────────────┘
```

### 4. Calibration Menu
```
┌────────────────┐
│>Sensor  Pump   │
│ Back           │
└────────────────┘
```

### 4.1 Calibrate Sensor - Step 1 (Dry)
```
┌────────────────┐
│Put sensor in   │
│AIR, press SEL  │
└────────────────┘

After reading:
┌────────────────┐
│Dry: 3100       │ ← Raw ADC value
│Press SEL       │
└────────────────┘
```

### 4.1 Calibrate Sensor - Step 2 (Wet)
```
┌────────────────┐
│Put sensor in   │
│WATER, press SEL│
└────────────────┘

After reading:
┌────────────────┐
│Wet: 1500       │
│SEL=Save BCK=X  │
└────────────────┘
```

### 4.2 Calibrate Pump - Step 1
```
┌────────────────┐
│Place tube in   │
│cup, press SEL  │
└────────────────┘
```

**During 10s pump run:**
```
┌────────────────┐
│PUMPING... 7s   │ ← Countdown
│                │
└────────────────┘
```

**Enter amount collected:**
```
┌────────────────┐
│Collected: 15mL │ ← Adjust with UP/DOWN
│SEL=Save BCK=X  │
└────────────────┘

After save:
┌────────────────┐
│Rate: 1.5mL/s   │ ← Calculated rate
│Saved!          │
└────────────────┘
```

### 5. System Info
**Page 1:**
```
┌────────────────┐
│FW: v1.0        │ ← Firmware version
│Uptime: 3d 5h   │
└────────────────┘
```

**Page 2:** (auto-advance)
```
┌────────────────┐
│WiFi: Connected │ ← Future feature
│IP:192.168.1.50 │
└────────────────┘
```

### 6. Reset Settings
```
┌────────────────┐
│Reset to        │
│defaults?SEL=YES│
└────────────────┘

After confirmation:
┌────────────────┐
│Resetting...    │
│Please wait     │
└────────────────┘

Done:
┌────────────────┐
│Reset complete! │
│Press any key   │
└────────────────┘
```

### Error Messages
**Sensor disconnected:**
```
┌────────────────┐
│ERROR: Sensor!  │
│Check wiring    │
└────────────────┘
```

**Water level low:**
```
┌────────────────┐
│ALERT: Low H2O! │
│Refill tank     │
└────────────────┘
```

**Watering failed:**
```
┌────────────────┐
│ERR: No change  │
│Check pump/sens │
└────────────────┘
```

---

## Screen Layout for 20×4 LCD (Reference Only - Not Your Display)

**Note:** These are kept for reference if you upgrade to 20×4 LCD later.

### Main Menu - Idle Screen
```
┌────────────────────┐
│Plant Water System  │
│Moisture:  75%      │
│Target:    80%      │
│Status: Dry Cycle   │
└────────────────────┘
```

---

## Navigation State Machine

### State Definitions:
```cpp
enum MenuState {
    MAIN_IDLE,          // Show current status
    MAIN_MENU,          // Menu selection
    MANUAL_WATER,       // Manual watering confirmation
    SETTINGS_MENU,      // Settings submenu
    SETTING_TARGET,     // Adjust target humidity
    SETTING_MIN,        // Adjust minimum threshold
    SETTING_AMOUNT,     // Adjust watering amount
    SETTING_INTERVAL,   // Adjust check interval
    SETTING_SOAK_TIME,  // Adjust soak time after watering
    SETTING_DRY_CYCLE,  // Toggle dry cycle
    CAL_MENU,           // Calibration submenu
    CAL_SENSOR,         // Sensor calibration wizard
    CAL_PUMP,           // Pump calibration wizard
    SYSTEM_INFO,        // Display system information
    RESET_CONFIRM       // Reset confirmation
};
```

### Button Actions by State:

| State | UP | DOWN | SELECT | BACK |
|-------|-----|------|--------|------|
| MAIN_IDLE | Enter menu | Enter menu | Enter menu | - |
| MAIN_MENU | Previous item | Next item | Enter submenu | Return to idle |
| MANUAL_WATER | - | - | Execute water | Cancel |
| SETTINGS_MENU | Previous item | Next item | Enter setting | Return to main |
| SETTING_TARGET | Increase value | Decrease value | Save & exit | Cancel & exit |
| SETTING_MIN | Increase value | Decrease value | Save & exit | Cancel & exit |
| SETTING_AMOUNT | Increase value | Decrease value | Save & exit | Cancel & exit |
| SETTING_INTERVAL | Increase value | Decrease value | Save & exit | Cancel & exit |
| SETTING_SOAK_TIME | Increase value | Decrease value | Save & exit | Cancel & exit |
| SETTING_DRY_CYCLE | Toggle value | Toggle value | Save & exit | Cancel & exit |
| CAL_MENU | Previous item | Next item | Enter calibration | Return to main |
| CAL_SENSOR | - | - | Next step | Cancel & exit |
| CAL_PUMP | Increase value | Decrease value | Save/next | Cancel & exit |

---

## Configuration Storage (EEPROM)

### Memory Map:
```cpp
// EEPROM addresses (ESP32 has EEPROM emulation in flash)
#define EEPROM_MAGIC_ADDR       0    // 2 bytes: Magic number (0xA5C3)
#define EEPROM_TARGET_ADDR      2    // 1 byte: Target humidity (0-100)
#define EEPROM_MIN_ADDR         3    // 1 byte: Min threshold (0-100)
#define EEPROM_AMOUNT_ADDR      4    // 2 bytes: Water amount (mL × 10)
#define EEPROM_INTERVAL_ADDR    6    // 1 byte: Check interval (hours)
#define EEPROM_SOAK_TIME_ADDR   7    // 1 byte: Soak time (minutes)
#define EEPROM_DRY_CYCLE_ADDR   8    // 1 byte: Dry cycle enable (0/1)
#define EEPROM_CAL_DRY_ADDR     9    // 2 bytes: Sensor dry ADC value
#define EEPROM_CAL_WET_ADDR     11   // 2 bytes: Sensor wet ADC value
#define EEPROM_PUMP_RATE_ADDR   13   // 4 bytes: Pump rate (mL/s as float)
#define EEPROM_CHECKSUM_ADDR    17   // 1 byte: Simple checksum

// Total used: 18 bytes
```

### Default Values:
```cpp
struct Config {
    uint8_t targetHumidity = 80;      // 80%
    uint8_t minThreshold = 20;        // 20%
    uint16_t waterAmount = 100;       // 10.0mL (stored as mL × 10)
    uint8_t checkInterval = 2;        // 2 hours
    uint8_t soakTime = 5;             // 5 minutes (moisture propagation time)
    bool dryCycleEnabled = true;      // ON
    uint16_t sensorDry = 3100;        // Typical air reading
    uint16_t sensorWet = 1500;        // Typical water reading
    float pumpRate = 1.0;             // 1.0 mL/second
};
```

---

## Value Adjustment Behavior

### Increment/Decrement Steps:
| Parameter | Step Size | Min | Max | Unit |
|-----------|-----------|-----|-----|------|
| Target Humidity | 5% | 0 | 100 | % |
| Min Threshold | 5% | 0 | 100 | % |
| Water Amount | 0.5 (5 in storage) | 0 | 99.9 | mL |
| Check Interval | 1 | 1 | 24 | hours |
| Soak Time | 1 | 1 | 30 | minutes |
| Dry Cycle | toggle | OFF | ON | bool |
| Calibration values | Manual input | - | - | ADC/mL |

### Long Press Behavior (Optional Enhancement):
- Hold UP/DOWN for >1 second → Fast increment (10× step size)
- Useful for large adjustments

---

## User Feedback

### Visual Feedback:
- **Cursor position:** `>` symbol for selected menu item
- **Edit mode:** `[ value ]` brackets around editable value
- **Progress:** Animated dots during watering `...`
- **Alerts:** `!` symbol for warnings (low water, sensor error)

### Audio Feedback (Optional with Buzzer):
- Button press: Short beep (50ms)
- Save setting: Double beep
- Error: Long beep (500ms)
- Watering complete: Success jingle

---

## Error States and Messages

| Error Condition | Display Message | Action |
|----------------|-----------------|--------|
| Sensor disconnected | "ERR: Sensor!" | Stop watering, alert |
| Water not increasing | "ERR: No water?" | Enter failsafe mode |
| Invalid EEPROM | "Loading defaults" | Reset to defaults |
| Calibration invalid | "CAL: Invalid!" | Prompt recalibration |
| Moisture > 100% | "CHK: Sensor" | Display warning |

---

## Timing Considerations

### Button Debouncing:
- Debounce delay: 50ms
- Long press threshold: 1000ms (for fast adjust)

### Screen Update Rate:
- Idle screen: Update moisture every 2-5 seconds
- Menu navigation: Update immediately on button press
- Value adjustment: Update immediately on button press
- Watering progress: Update every 100ms

### Sensor Reading During Menu:
- Continue periodic sensor reads in background
- Update idle screen when returned to main
- Allow manual watering from any menu state (emergency)

---

## Implementation Notes

### Libraries Needed:
```cpp
#include <LiquidCrystal_I2C.h>  // I2C LCD control
#include <EEPROM.h>              // Configuration storage
```

### Key Functions:
```cpp
void updateDisplay();            // Refresh LCD based on current state
void handleButton(Button btn);   // Process button press
void saveConfig();               // Write config to EEPROM
void loadConfig();               // Read config from EEPROM
void resetConfig();              // Load defaults
void calibrateSensor();          // Sensor calibration wizard
void calibratePump();            // Pump calibration wizard
int readMoisture();              // Read and convert sensor to %
void waterPlant(int targetPercent); // Execute watering
```

---

## Future Enhancements (Phase 2)

When transitioning to cloud-connected final product:
- Menu system becomes secondary (mobile app primary)
- LCD shows minimal info: WiFi status, current moisture, last watering
- Buttons could be reduced to: Manual water, WiFi reset
- Configuration moves to mobile app
- LCD optional (cost savings) - LEDs only for status

**Recommendation:** Keep full menu system in POC for standalone testing and validation of all logic before adding cloud complexity.
