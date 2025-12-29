# POC Firmware - Arduino UNO R4 WiFi

This directory contains the complete firmware for the Phase 1 POC (Proof of Concept) plant watering system.

## Files

- `plant_watering_poc.ino` - Main Arduino sketch (complete implementation)

## Hardware Requirements

See [../../WIRING_POC.md](../../WIRING_POC.md) for complete wiring guide.

**Quick Reference:**
- **Arduino UNO R4 WiFi**
- 16×2 I2C LCD (address 0x27 or 0x3F)
- Cytron capacitive soil moisture sensor (MCP6004-based)
- 12V peristaltic pump
- IRLZ44N logic-level MOSFET
- 1N5819 Schottky diode
- 10kΩ resistors (2×)
- 4× push buttons
- 12V 2A power supply
- microSD card module (SPI)
- microSD card (4GB-32GB, FAT32)

## Dependencies

Install these libraries via Arduino IDE Library Manager:

1. **LiquidCrystal I2C** by Frank de Brabander
   - Menu: Sketch → Include Library → Manage Libraries
   - Search: "LiquidCrystal I2C"
   - Install version 1.1.2 or later

2. **Wire** (built-in with Arduino IDE)
3. **EEPROM** (built-in with Arduino IDE)
4. **SPI** (built-in with Arduino IDE)
5. **SD** (built-in with Arduino IDE)

## Upload Instructions

1. **Install Arduino IDE** (version 2.0 or later recommended)
   - Download from: https://www.arduino.cc/en/software

2. **Install Arduino UNO R4 WiFi board support**
   - Open Arduino IDE
   - Go to: Tools → Board → Boards Manager
   - Search: "Arduino UNO R4"
   - Install: "Arduino UNO R4 Boards" by Arduino

3. **Install required libraries**
   - Go to: Sketch → Include Library → Manage Libraries
   - Search and install: "LiquidCrystal I2C" by Frank de Brabander

4. **Open the sketch**
   - File → Open → `plant_watering_poc.ino`

5. **Configure board settings**
   - Tools → Board → Arduino UNO R4 Boards → Arduino UNO R4 WiFi
   - Tools → Port → Select your Arduino's port (e.g., COM3, /dev/ttyUSB0)

6. **Upload**
   - Click Upload button (→) or press Ctrl+U
   - Wait for "Done uploading" message

## First-Time Setup

After uploading the firmware for the first time:

### 1. Check LCD I2C Address

If the LCD doesn't display anything:

1. Upload an I2C scanner sketch to find the LCD address
2. Edit line 36 in `plant_watering_poc.ino`:
   ```cpp
   LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change 0x27 to 0x3F if needed
   ```
3. Re-upload the sketch

### 2. Calibrate Sensor

**CRITICAL:** You must calibrate the sensor for accurate readings!

1. Navigate: Main Menu → Calibrate → Sensor
2. Follow on-screen instructions:
   - Hold sensor in **air** → Press SELECT
   - Put sensor in **water** (cup of water) → Press SELECT
3. Calibration values are saved to EEPROM

**Note:** The default calibration values (12400 dry, 6000 wet) are typical for Arduino UNO R4's 14-bit ADC but may vary by sensor.

### 3. Calibrate Pump

For accurate water dosing:

1. Navigate: Main Menu → Calibrate → Pump
2. Prepare a measuring cup
3. Press SELECT to start pump
4. Let it run until you have measurable water (e.g., 100mL)
5. Press SELECT to stop pump
6. Use UP/DOWN to enter the amount of water collected
7. Press SELECT to save

**Typical pump rate:** ~1.67 mL/s (100 mL/min)

### 4. Configure Settings

Navigate: Main Menu → Settings

Default values (optimized for most plants):
- **Operating Mode:** Watering (toggle between Watering/Monitor)
- **Target Humidity:** 80% (upper limit - stop watering)
- **Min Threshold:** 10% (lower limit - start watering)
- **Water Amount:** 50.0 mL (amount per watering iteration)
- **Check Interval:** 6 hours (how often to check during dry cycle in watering mode)
- **Soak Time:** 5 minutes (wait for capillarity/moisture propagation)

**Note:** Dry cycle is always enabled in watering mode - it's core to healthy plant watering.

Press SELECT on each setting to adjust, then SELECT again to save.

## Usage

### Operating Modes

The system has two distinct operating modes that can be selected via Settings → Mode:

#### 1. Watering Mode (Default)

Automatic watering mode with intelligent dry cycle logic.

**How it works:**
- Monitors soil moisture every 6 hours (configurable)
- Waters plant when moisture drops to or below minimum threshold (10%)
- Waters in 50mL increments with 5-minute soak time
- Continues watering until upper limit (80%) is reached
- Prevents over-watering with failsafe protection

**Use case:** Normal automated plant care - set it and forget it!

#### 2. Monitor Mode

Passive monitoring mode that logs moisture data to SD card without watering.

**How it works:**
- Reads soil moisture every 5 minutes
- Logs data to `moisture.csv` on SD card
- CSV format: `Timestamp(ms), Moisture(%), RawADC`
- No automatic watering occurs
- Displays "MONITOR mode" on status screen

**Use case:** Learn your plant's natural watering cycles before automating!

**Monitor Mode Workflow:**
1. Water your plant manually as you normally would
2. Switch system to Monitor Mode (Settings → Mode → Monitor)
3. Leave it running for 1-2 weeks
4. Remove SD card and plot the data (Excel, Python, etc.)
5. Observe how moisture drops between waterings
6. Use this data to configure optimal Target/Min thresholds
7. Switch back to Watering Mode for automation

**Example CSV output:**
```
Timestamp(ms),Moisture(%),RawADC
300000,82,6234
600000,78,6521
900000,73,6892
1200000,68,7245
...
```

**Note:** SD card must be installed and working for Monitor Mode. If SD card fails during boot, system automatically switches to Watering Mode.

### Main Menu Navigation

- **UP/DOWN:** Navigate menu items
- **SELECT:** Enter submenu or confirm
- **BACK:** Return to previous menu or cancel

### Menu Structure

```
MAIN MENU
├── Status (view current moisture and system state)
├── Settings
│   ├── Mode (Watering / Monitor)
│   ├── Target Humidity (0-100%)
│   ├── Min Threshold (0-100%)
│   ├── Water Amount (mL)
│   ├── Check Interval (hours)
│   └── Soak Time (minutes)
├── Calibrate
│   ├── Sensor (air + water calibration)
│   └── Pump (flow rate calibration)
└── Manual Water (trigger watering manually)
```

**Note:** Dry cycle is always ON in watering mode and not separately configurable - it's essential for plant health.

### How Automatic Watering Works

The system uses a **two-phase cycle**:

**Phase 1: Watering Phase** (moisture ≤ Min Threshold)
1. Water configured `Water Amount` (50mL)
2. Wait `Soak Time` (5 min) for capillarity
3. Re-measure moisture
4. If below `Target` (80%) → Repeat steps 1-3
5. If reached `Target` → Enter Dry Cycle Phase

**Phase 2: Dry Cycle Phase** (moisture > Min Threshold)
1. Check moisture every `Check Interval` (6 hours)
2. Don't water unless moisture drops to `Min Threshold` (10%)
3. When ≤10% → Re-enter Watering Phase

**Failsafe Protection:**
- Stops if moisture doesn't increase after watering
- Prevents flooding with maximum iteration limit
- See [WATERING_LOGIC.md](../../WATERING_LOGIC.md) for detailed explanation

### Status Screen

Displays:
- **Current moisture** (0-100%)
- **System state:**
  - "DRY - Need water" (below minimum threshold)
  - "OK - Dry cycle" (above minimum, dry cycle active)
  - "Below target" (dry cycle OFF, below target)
  - "OK" (at or above target)

## Serial Monitor Debugging

Open Serial Monitor (Tools → Serial Monitor) at **115200 baud** to see:
- ADC readings and moisture calculations
- Watering cycle status
- Pump operation details
- Configuration load/save messages
- Error messages

**Example output:**
```
Plant Watering System - POC
Config loaded from EEPROM
ADC: 9234 -> 52%
Starting watering cycle...
Pumping 10.0mL (5.99s)
Waiting 5 minutes for moisture propagation...
ADC: 7845 -> 67%
Watering cycle complete
```

## Troubleshooting

### LCD shows nothing
- Check I2C address (try 0x3F instead of 0x27)
- Check wiring: SDA → A4, SCL → A5
- Adjust contrast potentiometer on LCD backpack

### Sensor reads 0% or 100% constantly
- **Run sensor calibration** (Calibrate → Sensor)
- Check wiring: A0 for analog, D2 for power
- Verify sensor is waterproofed (nail polish coating)

### Pump doesn't run
- Check common ground connection (Arduino GND → 12V GND)
- Verify MOSFET wiring (see [WIRING_POC.md](../../WIRING_POC.md))
- Check 12V power supply is connected
- Test MOSFET: should be IRLZ44N (logic-level), not IRF44N

### Moisture doesn't increase after watering
- Check pump is actually pumping water
- Verify tubing connections
- Ensure sensor is in the soil (not in air)
- Check water reservoir has water

### System doesn't water automatically
- Check Settings → Mode (must be in Watering mode, not Monitor mode)
- Check Settings → Check Interval (may be set too high)
- Verify moisture is at or below Min Threshold
- Check Serial Monitor for errors

### SD card not working
- Verify SD card is formatted as FAT32 (not exFAT)
- Check wiring: CS→D10, MOSI→D11, MISO→D12, SCK→D13
- Try a different SD card (some cards have compatibility issues)
- Check Serial Monitor for "SD card initialized" message
- If SD fails, system automatically switches to Watering mode

### Monitor mode not logging
- Check Settings → Mode is set to "Monitor"
- Verify SD card is installed and initialized (check at boot)
- Check SD card has free space
- Serial Monitor shows "Logged: X%" every 5 minutes when working
- Remove SD card and check if `moisture.csv` file exists

## Safety Notes

⚠️ **Important:**
- Never connect pump directly to Arduino pins (always use MOSFET!)
- Always connect common ground between Arduino and 12V supply
- Waterproof sensor before use (nail polish coating)
- Don't power Arduino from 12V directly (use USB or buck converter)

## Configuration Storage

All settings are stored in EEPROM and persist across power cycles:
- Operating mode (Watering / Monitor)
- Target humidity, min threshold, water amount
- Check interval, soak time
- Sensor calibration (dry/wet ADC values)
- Pump flow rate

**Factory Reset:** Not currently implemented in menu (coming soon)
- To reset manually: Upload sketch with default values or use Serial Monitor to trigger `resetToDefaults()`

## Next Steps

After the POC is working:
1. Monitor for 1-2 weeks to verify watering logic
2. Adjust settings based on plant needs and soil type
3. Document sensor lifespan and accuracy drift
4. Plan Phase 2: ESP32-based 6-plant system with cloud connectivity

## Contributing

Found a bug or want to improve the code?
- Open an issue on GitHub
- Submit a pull request
- Document your findings in project notes

## License

MIT License - See repository root for details
