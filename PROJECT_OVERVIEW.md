# Plant Watering System - Project Overview

## Project Phases

### Phase 1: Proof of Concept (POC)
**Hardware:**
- **Arduino Uno R4 WiFi** (Renesas RA4M1, WiFi built-in, 5V logic) - ✅ Already owned
- 1x Cytron capacitive soil moisture sensor (MCP6004-based, waterproofed) - ✅ Already owned
- 1x 12V peristaltic pump - ✅ Already owned
- 1x 16×2 I2C LCD display (from GeeekPi kit) - ✅ Already owned
- 4x navigation buttons: Up, Down, Select, Back (from GeeekPi kit) - ✅ Already owned
- IRLZ44N logic-level MOSFET (pump control, works with 5V) - ✅ Already owned
- 1N5819 Schottky diode (flyback protection) - ✅ Already owned
- Breadboard and jumper wires (from GeeekPi kit) - ✅ Already owned
- 12V 2A power supply (for pump) - ✅ Already owned
- 10kΩ resistors (for MOSFET gate circuit, from resistor kit) - ✅ Already owned

**Features:**
- Single plant monitoring and watering
- LCD menu-based configuration interface
- Configurable parameters:
  - Desired humidity level (target %)
  - Minimum humidity threshold (dry cycle minimum %)
  - Watering amount (mL or pump duration)
  - Check interval (hours)
  - Soak time (minutes - wait after watering for moisture to propagate)
  - Dry cycle enable/disable
- Manual watering trigger
- Real-time moisture display
- Sensor calibration interface

**Goals:**
- Test and validate watering logic
- Develop reliable menu navigation system
- Test sensor accuracy and pump control
- Validate dry cycle algorithm
- Establish baseline power requirements

---

### Phase 2: Final Product (Production)
**Hardware:**
- ESP32 (or Arduino Nano - cost comparison needed)
- 6x Capacitive soil moisture sensors (waterproofed)
- 6x Peristaltic pumps (5-12V)
- 1x Water level sensor (ultrasonic HC-SR04 or laser ToF VL53L0X)
- 3D printed enclosure and reservoir
- 6x Relay modules or MOSFET array
- Power supply system (12V with buck converter to 5V/3.3V)
- 6x 1N4007 flyback diodes
- Optional: Status LEDs, buzzer for local alerts

**Features:**
- 6 independent plant monitoring and control
- Cloud connectivity (Arduino IoT Cloud or similar)
- Mobile app with multi-device support:
  - House/device grouping concept
  - Per-plant naming and configuration
  - Remote monitoring of all devices
  - Push notifications for low water alerts
  - Per-plant configuration:
    - Desired humidity level
    - Minimum humidity threshold (dry cycle)
    - Watering amount
    - Check interval
    - Soak time (moisture propagation wait period)
- Dry cycle logic per plant
- Water level monitoring with automatic pump disable
- Failsafe logic:
  - Stop watering if moisture doesn't increase
  - Alert on sensor failures
  - Alert on low water level
- WiFi reconnection logic
- Deep sleep mode for power efficiency (if battery-powered)

**Goals:**
- Reliable multi-plant management
- Remote monitoring and control
- Scalable to multiple devices per household
- Production-ready enclosure
- Low maintenance operation

---

## Key Features Explained

### Dry Cycle Logic
Purpose: Prevent constant watering and allow natural soil drying cycle

**Default Configuration:**
- Target humidity (upper limit): 80%
- Minimum threshold (lower limit): 10%
- Water amount per iteration: 50 mL
- Check interval (during dry cycle): 6 hours
- Soak time (capillarity wait): 5 minutes

**Behavior:**
1. **Watering Phase:** When moisture ≤ 10%, water in 50mL increments with 5-min soak until 80% reached
2. **Dry Cycle Phase:** When moisture > 10%, check every 6 hours but don't water
3. System re-enters watering phase only when moisture drops to ≤10%
4. This mimics natural rainfall patterns and prevents root rot

See [WATERING_LOGIC.md](WATERING_LOGIC.md) for detailed explanation with examples.

**Implementation:**
```cpp
if (currentMoisture <= minThreshold) {
    waterUntilTarget(targetHumidity);
} else {
    // Dry cycle active - do not water
}
```

### Watering Logic with Moisture Propagation

After watering, the system waits for the configured **soak time** before taking another moisture reading. This is critical because:

1. **Water distribution takes time**: When water is added to soil, it doesn't instantly reach all parts of the soil volume
2. **Capillary action**: Water moves through soil via capillary action, which can take several minutes
3. **Sensor stabilization**: The capacitive sensor needs time to read the actual moisture level after water is added
4. **Prevents over-watering**: Without a soak time, the system might think the soil is still dry and add more water

**Recommended soak time values:**
- **Fast-draining soil** (sandy): 3-5 minutes
- **Medium soil** (loam): 5-10 minutes
- **Slow-draining soil** (clay): 10-15 minutes
- **Default**: 5 minutes (good for most potting mixes)

**Watering Algorithm:**
```cpp
int readMoisture() {
    // Power on sensor from GPIO (extends sensor life dramatically)
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(200);  // Warmup time for stable reading

    // Average 20 samples to reduce noise (ESP32 ADC is noisy)
    long sum = 0;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(SENSOR_ADC_PIN);  // Must use ADC1 pin (GPIO32-39)!
        delay(10);
    }
    int avgReading = sum / numSamples;

    // Power off sensor
    digitalWrite(SENSOR_POWER_PIN, LOW);

    // Convert to percentage using calibration values
    int moisturePercent = map(avgReading, sensorDry, sensorWet, 0, 100);
    return constrain(moisturePercent, 0, 100);
}

void waterPlant() {
    int initialMoisture = readMoisture();

    // Water in small increments
    while (currentMoisture < targetHumidity) {
        pumpWater(waterAmount);  // Pump configured amount (mL)

        // Wait for moisture to propagate through soil
        delay(soakTime * 60 * 1000);  // Convert minutes to milliseconds

        currentMoisture = readMoisture();

        // Failsafe: if moisture didn't increase, stop watering
        if (currentMoisture <= initialMoisture) {
            setError("Water not absorbing - check sensor/pump");
            break;
        }

        initialMoisture = currentMoisture;
    }
}
```

**Benefits of soak time:**
- Prevents flooding from rapid repeated watering cycles
- Allows accurate moisture readings after watering
- Reduces water waste
- Prevents root damage from over-watering
- Works with different soil types by being configurable

### Moisture Sensor Best Practices (from research doc)
- Use capacitive sensors only (no resistive)
- Waterproof with nail polish, PlastiDip, or heat-shrink
- Power from GPIO pin, not constant power
- Take averaged readings (10-64 samples)
- Calibrate in actual soil, not just air/water
- Check for TLC555 chip (not NE555 on 3.3V systems)

### Pump Control Best Practices
- Peristaltic pumps preferred (can run dry, precise dosing)
- Always check water level before pumping
- Add flyback diode protection
- Implement post-watering sensor verification
- Use separate power supply from microcontroller

### ESP32-Specific Considerations

**Critical ADC Constraint:**
- **ADC2 pins (GPIO0, 2, 4, 12-15, 25-27) cannot be used while WiFi is active**
- Sensor readings will fail or return garbage if using ADC2 with WiFi enabled
- **Always use ADC1 pins (GPIO32-39) for moisture sensors**

**WiFi Reconnection Logic:**
ESP32 does NOT automatically reconnect to WiFi after signal loss or router reboots. Without explicit reconnection logic, the system will stay offline until manually reset.

```cpp
// Setup WiFi with stability improvements
void setupWiFi() {
    WiFi.persistent(false);  // Prevent flash wear from repeated WiFi.begin()
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, password);

    // Use static IP to avoid DHCP issues
    IPAddress local_IP(192, 168, 1, 100);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(local_IP, gateway, subnet);
}

// Check and reconnect in main loop
void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost - reconnecting...");
        WiFi.reconnect();
        delay(5000);
    }
    // ... rest of loop
}
```

**Additional WiFi Stability Tips:**
- Set router to 20MHz channel width (not 40MHz/auto)
- Keep signal strength better than -70 dBm
- Use static IP instead of DHCP (prevents lease expiration disconnects)
- Extend DHCP lease time on router if using DHCP

**Deep Sleep Considerations (for battery operation):**
- ESP32 has a known bug: after random wake cycles, may sleep and never wake
- Fix: add 500ms delay after waking before reading RTC memory
- Use `ULL` suffix for long sleep durations to prevent overflow:
  ```cpp
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 1000000ULL);
  ```
- Different boards have vastly different deep sleep current (19mA vs 0.01mA)

### Mechanical and Tubing Best Practices

**Reservoir Placement:**
- **Keep water reservoir BELOW plant level** to prevent siphoning
- If reservoir is above plants, water continues draining after pump stops

**Tubing Selection:**
- **Use opaque tubing** - transparent tubing grows algae rapidly when exposed to light
- **5-6mm inner diameter** recommended - balances flow rate vs clogging resistance
- Narrow tubing (1/4") clogs faster from mineral buildup
- Flush with plain water periodically; clean with vinegar when clogged

**Distribution Method:**
- **Closed-end hose with drilled holes** works better than Y-splitters
- Splitters cause uneven distribution (some plants get all water, others get air)
- Single hose with 1mm holes at each plant location creates equal pressure

---

## Configuration Parameters

| Parameter | POC (Single Plant) | Final (Per Plant) | Range/Options |
|-----------|-------------------|-------------------|---------------|
| Target Humidity | Yes | Yes | 0-100% |
| Min Threshold (Dry Cycle) | Yes | Yes | 0-100% |
| Watering Amount | Yes | Yes | mL or seconds |
| Check Interval | Yes | Yes | 1-24 hours |
| Soak Time | Yes | Yes | 1-30 minutes |
| Dry Cycle Enable | Yes | Yes | On/Off |
| Plant Name | No | Yes | String (app only) |
| Pump Calibration | Yes | Yes | mL per second |
| Sensor Calibration | Yes | Yes | ADC values for 0%/100% |

## Maintenance Schedule

Based on community experience with long-running systems:

**Weekly:**
- Check reservoir water level
- Verify pump operation with manual test
- Review moisture readings for anomalies or sensor drift

**Monthly:**
- Inspect tubing for algae/mineral buildup
- Verify sensor readings against manual soil check (finger test)
- Test notification system (if applicable)
- Check electrical connections for corrosion

**Quarterly (Every 3 months):**
- Inspect sensors for corrosion or waterproofing degradation
- Recalibrate sensors if readings have drifted
- Clean reservoir with vinegar solution
- Check flyback diodes and MOSFET connections

**Annually (Every 12 months):**
- **Replace peristaltic pump tubing** (wears out from compression)
- Deep clean water reservoir and tubing with vinegar
- Replace sensors showing significant drift or corrosion
- Update firmware with bug fixes and improvements
- Test failsafe modes (simulate sensor failure, empty reservoir)

**Signs That Immediate Maintenance Is Needed:**
- Moisture readings don't match manual soil check
- Pump runs but water doesn't flow
- WiFi disconnects frequently
- Unexpected watering patterns
- Visible corrosion on sensor or wiring

---

## Development Roadmap

### POC Phase Milestones
1. ✓ Hardware assembly and basic pump control
2. ✓ Sensor reading and calibration
3. ✓ LCD menu system development
4. ✓ Configuration storage (EEPROM)
5. ✓ Dry cycle logic implementation
6. ✓ Testing and refinement

### Final Product Milestones
1. Hardware selection and cost analysis
2. Multi-sensor and multi-pump control
3. Water level sensor integration
4. Cloud platform setup (Arduino IoT Cloud)
5. Mobile app development
6. Multi-device architecture
7. 3D enclosure design
8. Production assembly and testing

---

## Next Steps
1. Gather component links and pricing for POC BOM
2. Design POC menu structure
3. Begin Arduino Uno R4 WiFi code development
4. Test sensor waterproofing methods
5. Validate pump flow rate calibration
