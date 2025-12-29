/*
 * Plant Watering System - POC (Phase 1)
 *
 * Hardware: Arduino UNO R4 WiFi
 * Display: 16x2 I2C LCD
 *
 * Features:
 * - Single plant monitoring and watering
 * - Two operating modes: Watering (automatic) and Monitor (data logging)
 * - LCD menu-based configuration
 * - GPIO-powered sensor (extends lifespan)
 * - Dry cycle logic with configurable thresholds
 * - Soak time for moisture propagation
 * - Monitor mode: logs moisture to SD card every 5 minutes (CSV format)
 * - Sensor and pump calibration wizards
 * - EEPROM configuration storage
 * - Failsafe protection
 *
 * Pin Assignments:
 * - A0: Soil Moisture Sensor (ADC)
 * - D2: Sensor Power Control
 * - D3: Pump MOSFET Control
 * - SDA (A4): LCD SDA
 * - SCL (A5): LCD SCL
 * - D4: Button UP
 * - D5: Button DOWN
 * - D6: Button SELECT
 * - D7: Button BACK
 * - D10: SD Card CS (Chip Select)
 * - D11: SD Card MOSI (SPI)
 * - D12: SD Card MISO (SPI)
 * - D13: SD Card SCK (SPI)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

#define SENSOR_ADC_PIN      A0    // Soil moisture sensor analog input
#define SENSOR_POWER_PIN    2     // GPIO power control for sensor
#define PUMP_PIN            3     // MOSFET gate control for pump
#define BTN_UP              4     // Navigation button: UP
#define BTN_DOWN            5     // Navigation button: DOWN
#define BTN_SELECT          6     // Navigation button: SELECT
#define BTN_BACK            7     // Navigation button: BACK
#define SD_CS_PIN           10    // SD card chip select (SPI)

// ============================================================================
// LCD CONFIGURATION
// ============================================================================

// Try 0x27 first, if that doesn't work try 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================================
// EEPROM MEMORY MAP (18 bytes total)
// ============================================================================

#define EEPROM_MAGIC_ADDR       0    // 2 bytes: Magic number (0xA5C3)
#define EEPROM_TARGET_ADDR      2    // 1 byte: Target humidity (0-100)
#define EEPROM_MIN_ADDR         3    // 1 byte: Min threshold (0-100)
#define EEPROM_AMOUNT_ADDR      4    // 2 bytes: Water amount (mL × 10)
#define EEPROM_INTERVAL_ADDR    6    // 1 byte: Check interval (hours)
#define EEPROM_SOAK_TIME_ADDR   7    // 1 byte: Soak time (minutes)
#define EEPROM_MODE_ADDR        8    // 1 byte: Operating mode (0=Watering, 1=Monitor)
#define EEPROM_CAL_DRY_ADDR     9    // 2 bytes: Sensor dry ADC value
#define EEPROM_CAL_WET_ADDR     11   // 2 bytes: Sensor wet ADC value
#define EEPROM_PUMP_RATE_ADDR   13   // 4 bytes: Pump rate (mL/s as float)
#define EEPROM_CHECKSUM_ADDR    17   // 1 byte: Simple checksum

#define EEPROM_MAGIC_NUMBER 0xA5C3   // Magic number for valid config

// ============================================================================
// OPERATING MODES
// ============================================================================

#define MODE_WATERING 0  // Automatic watering mode (default)
#define MODE_MONITOR  1  // Passive monitoring mode (logs to SD card)

// ============================================================================
// CONFIGURATION STRUCTURE
// ============================================================================

struct Config {
    uint8_t targetHumidity;      // Target moisture % (0-100)
    uint8_t minThreshold;        // Minimum threshold for dry cycle (0-100)
    uint16_t waterAmount;        // Water amount in mL × 10 (for 0.1 mL precision)
    uint8_t checkInterval;       // Check interval in hours (1-24)
    uint8_t soakTime;            // Soak time in minutes (1-30)
    uint8_t operatingMode;       // Operating mode (0=Watering, 1=Monitor)
    uint16_t sensorDry;          // ADC value when sensor is in air (14-bit: 0-16383)
    uint16_t sensorWet;          // ADC value when sensor is in water (14-bit: 0-16383)
    float pumpRate;              // Pump flow rate in mL/s
};

// Note: Dry cycle is always enabled - it's core to the watering logic

// Default configuration
Config config = {
    .targetHumidity = 80,        // Target 80% (upper limit - stop watering)
    .minThreshold = 10,          // Water when below 10% (lower limit - start watering)
    .waterAmount = 500,          // 50.0 mL per watering (stored as 500 = 50.0 * 10)
    .checkInterval = 6,          // Check every 6 hours (during dry cycle in watering mode)
    .soakTime = 5,               // Wait 5 minutes for moisture propagation (capillarity)
    .operatingMode = MODE_WATERING,  // Default: automatic watering mode
    .sensorDry = 12400,          // Typical 14-bit ADC value in air
    .sensorWet = 6000,           // Typical 14-bit ADC value in water
    .pumpRate = 1.67             // Typical peristaltic pump ~100mL/min = 1.67mL/s
};

// ============================================================================
// MENU STATE MACHINE
// ============================================================================

enum MenuState {
    MAIN_MENU,
    STATUS_SCREEN,
    SETTINGS_MENU,
    SETTING_MODE,
    SETTING_TARGET,
    SETTING_MIN,
    SETTING_AMOUNT,
    SETTING_INTERVAL,
    SETTING_SOAK_TIME,
    CALIBRATE_MENU,
    CAL_SENSOR_START,
    CAL_SENSOR_AIR,
    CAL_SENSOR_WATER,
    CAL_SENSOR_DONE,
    CAL_PUMP_START,
    CAL_PUMP_RUNNING,
    CAL_PUMP_ENTER_ML,
    CAL_PUMP_DONE,
    MANUAL_WATER_CONFIRM,
    MANUAL_WATER_RUNNING,
    RESET_CONFIRM
};

MenuState currentState = MAIN_MENU;
uint8_t menuIndex = 0;           // Current menu selection
uint8_t settingsIndex = 0;       // Settings menu item index
uint8_t calibrateIndex = 0;      // Calibrate menu item index

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

int currentMoisture = 0;         // Current moisture reading (%)
unsigned long lastCheckTime = 0; // Last moisture check timestamp
unsigned long lastWateringTime = 0; // Last watering timestamp
bool isWatering = false;         // Currently watering flag
unsigned long wateringStartTime = 0; // Watering start timestamp for pump calibration

// Calibration temporary variables
uint16_t tempSensorDry = 0;
uint16_t tempSensorWet = 0;
unsigned long pumpCalibrationStart = 0;
uint16_t pumpCalibrationML = 0;  // Water amount in mL × 10

// Button debouncing
unsigned long lastButtonPress = 0;
#define DEBOUNCE_DELAY 200       // 200ms debounce

// Monitor mode variables
bool sdCardAvailable = false;    // SD card initialized successfully
unsigned long lastLogTime = 0;   // Last data log timestamp
#define MONITOR_LOG_INTERVAL 300000UL  // 5 minutes in milliseconds (5 * 60 * 1000)
String currentLogFile = "";      // Current CSV log file name

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println(F("Plant Watering System - POC"));

    // Initialize pins
    pinMode(SENSOR_POWER_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    // Ensure pump is off
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(SENSOR_POWER_PIN, LOW);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Plant Watering"));
    lcd.setCursor(0, 1);
    lcd.print(F("System v1.0"));
    delay(2000);

    // Load configuration from EEPROM
    loadConfig();

    // Initialize SD card (for monitor mode)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Init SD card..."));

    if (SD.begin(SD_CS_PIN)) {
        sdCardAvailable = true;
        Serial.println(F("SD card initialized"));
        lcd.setCursor(0, 1);
        lcd.print(F("SD: OK"));
        delay(1000);
    } else {
        sdCardAvailable = false;
        Serial.println(F("SD card init failed - monitor mode disabled"));
        lcd.setCursor(0, 1);
        lcd.print(F("SD: FAILED"));
        delay(2000);

        // If in monitor mode but SD failed, switch to watering mode
        if (config.operatingMode == MODE_MONITOR) {
            config.operatingMode = MODE_WATERING;
            saveConfig();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Switched to"));
            lcd.setCursor(0, 1);
            lcd.print(F("Watering mode"));
            delay(2000);
        }
    }

    // Take initial moisture reading
    currentMoisture = readMoisture();

    // Display main menu
    displayMenu();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Handle button input
    handleButtons();

    // Operating mode logic (only when not in menus)
    if (currentState == STATUS_SCREEN || currentState == MAIN_MENU) {
        unsigned long currentTime = millis();

        if (config.operatingMode == MODE_WATERING) {
            // WATERING MODE: Automatic watering with dry cycle logic
            unsigned long intervalMs = (unsigned long)config.checkInterval * 3600000UL; // Hours to ms

            if (currentTime - lastCheckTime >= intervalMs) {
                lastCheckTime = currentTime;
                currentMoisture = readMoisture();

                // Check if watering is needed (dry cycle logic)
                if (shouldWater()) {
                    waterPlant();
                }
            }
        } else if (config.operatingMode == MODE_MONITOR) {
            // MONITOR MODE: Log moisture every 5 minutes, no watering
            if (currentTime - lastLogTime >= MONITOR_LOG_INTERVAL) {
                lastLogTime = currentTime;
                currentMoisture = readMoisture();
                logMoistureToSD();

                // Return to status screen to show updated moisture
                if (currentState == MAIN_MENU) {
                    currentState = STATUS_SCREEN;
                }
                displayMenu();
            }
        }
    }

    delay(50); // Small delay to reduce CPU usage
}

// ============================================================================
// SENSOR READING
// ============================================================================

int readMoisture() {
    // Power on sensor from GPIO (extends sensor life dramatically)
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(200); // Warmup time for stable reading

    // Average 20 samples to reduce noise (Arduino UNO R4 has 14-bit ADC)
    long sum = 0;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(SENSOR_ADC_PIN);
        delay(10);
    }
    int avgReading = sum / numSamples;

    // Power off sensor
    digitalWrite(SENSOR_POWER_PIN, LOW);

    // Convert to percentage using calibration values
    // Arduino UNO R4: ADC is 14-bit (0-16383)
    int moisturePercent = map(avgReading, config.sensorDry, config.sensorWet, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.print(F("ADC: "));
    Serial.print(avgReading);
    Serial.print(F(" -> "));
    Serial.print(moisturePercent);
    Serial.println(F("%"));

    return moisturePercent;
}

int readMoistureRaw() {
    // Read raw ADC value (for calibration)
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(200);

    long sum = 0;
    const int numSamples = 20;
    for (int i = 0; i < numSamples; i++) {
        sum += analogRead(SENSOR_ADC_PIN);
        delay(10);
    }
    int avgReading = sum / numSamples;

    digitalWrite(SENSOR_POWER_PIN, LOW);

    return avgReading;
}

// ============================================================================
// WATERING LOGIC
// ============================================================================

bool shouldWater() {
    // Dry cycle logic (always enabled): only water if at or below minimum threshold
    return currentMoisture <= config.minThreshold;
}

void waterPlant() {
    Serial.println(F("Starting watering cycle..."));

    int initialMoisture = currentMoisture;
    int iterationCount = 0;
    const int maxIterations = 10; // Safety limit

    // Water in small increments until target is reached
    while (currentMoisture < config.targetHumidity && iterationCount < maxIterations) {
        iterationCount++;

        // Pump configured amount (mL)
        float amountML = config.waterAmount / 10.0; // Convert from stored value
        pumpWater(amountML);

        // Wait for moisture to propagate through soil (soak time)
        Serial.print(F("Waiting "));
        Serial.print(config.soakTime);
        Serial.println(F(" minutes for moisture propagation..."));

        // Display watering status during soak time
        unsigned long soakTimeMs = (unsigned long)config.soakTime * 60000UL;
        unsigned long soakStart = millis();
        while (millis() - soakStart < soakTimeMs) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Soaking..."));
            lcd.setCursor(0, 1);
            unsigned long remaining = (soakTimeMs - (millis() - soakStart)) / 1000;
            lcd.print(remaining);
            lcd.print(F("s left"));
            delay(1000);
        }

        // Re-check moisture
        currentMoisture = readMoisture();

        // Failsafe: if moisture didn't increase, stop watering
        if (currentMoisture <= initialMoisture) {
            Serial.println(F("ERROR: Moisture not increasing - stopping!"));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("ERROR!"));
            lcd.setCursor(0, 1);
            lcd.print(F("Check sensor/pump"));
            delay(5000);
            break;
        }

        initialMoisture = currentMoisture;
    }

    if (iterationCount >= maxIterations) {
        Serial.println(F("WARNING: Max watering iterations reached"));
    }

    Serial.println(F("Watering cycle complete"));
    lastWateringTime = millis();
}

void pumpWater(float mL) {
    // Calculate pump run time based on flow rate
    float seconds = mL / config.pumpRate;
    unsigned long pumpTimeMs = (unsigned long)(seconds * 1000);

    Serial.print(F("Pumping "));
    Serial.print(mL);
    Serial.print(F("mL ("));
    Serial.print(seconds);
    Serial.println(F("s)"));

    // Turn on pump
    digitalWrite(PUMP_PIN, HIGH);

    // Display pumping status
    unsigned long pumpStart = millis();
    while (millis() - pumpStart < pumpTimeMs) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Pumping..."));
        lcd.setCursor(0, 1);
        unsigned long remaining = (pumpTimeMs - (millis() - pumpStart)) / 1000;
        lcd.print(remaining);
        lcd.print(F("s left"));
        delay(500);
    }

    // Turn off pump
    digitalWrite(PUMP_PIN, LOW);
}

// ============================================================================
// SD CARD LOGGING (MONITOR MODE)
// ============================================================================

void logMoistureToSD() {
    if (!sdCardAvailable) {
        Serial.println(F("SD card not available - cannot log"));
        return;
    }

    // Generate filename based on a simple day counter (since we don't have RTC)
    // Format: moisture_log.csv
    String filename = "moisture.csv";

    // Check if file exists, if not create with header
    bool fileExists = SD.exists(filename.c_str());

    // Open file for appending
    File logFile = SD.open(filename.c_str(), FILE_WRITE);

    if (!logFile) {
        Serial.println(F("Failed to open log file"));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("SD Write Error"));
        delay(2000);
        displayMenu();
        return;
    }

    // Write CSV header if new file
    if (!fileExists) {
        logFile.println(F("Timestamp(ms),Moisture(%),RawADC"));
        Serial.println(F("Created new log file with header"));
    }

    // Read current moisture with raw ADC value
    int rawADC = readMoistureRaw();
    int moisture = map(rawADC, config.sensorDry, config.sensorWet, 0, 100);
    moisture = constrain(moisture, 0, 100);

    // Write data: timestamp, moisture%, raw ADC
    logFile.print(millis());
    logFile.print(F(","));
    logFile.print(moisture);
    logFile.print(F(","));
    logFile.println(rawADC);

    logFile.close();

    Serial.print(F("Logged: "));
    Serial.print(millis());
    Serial.print(F("ms, "));
    Serial.print(moisture);
    Serial.print(F("%, ADC="));
    Serial.println(rawADC);

    // Brief confirmation on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Logged: "));
    lcd.print(moisture);
    lcd.print(F("%"));
    lcd.setCursor(0, 1);
    unsigned long nextLog = (MONITOR_LOG_INTERVAL - (millis() - lastLogTime)) / 60000;
    lcd.print(F("Next: "));
    lcd.print(nextLog);
    lcd.print(F("min"));
    delay(2000);
}

// ============================================================================
// EEPROM FUNCTIONS
// ============================================================================

void loadConfig() {
    // Check magic number
    uint16_t magic = (EEPROM.read(EEPROM_MAGIC_ADDR) << 8) | EEPROM.read(EEPROM_MAGIC_ADDR + 1);

    if (magic != EEPROM_MAGIC_NUMBER) {
        Serial.println(F("No valid config found, using defaults"));
        saveConfig(); // Write defaults to EEPROM
        return;
    }

    // Verify checksum
    uint8_t storedChecksum = EEPROM.read(EEPROM_CHECKSUM_ADDR);
    uint8_t calculatedChecksum = calculateChecksum();

    if (storedChecksum != calculatedChecksum) {
        Serial.println(F("Config checksum mismatch, using defaults"));
        saveConfig();
        return;
    }

    // Load configuration
    config.targetHumidity = EEPROM.read(EEPROM_TARGET_ADDR);
    config.minThreshold = EEPROM.read(EEPROM_MIN_ADDR);
    config.waterAmount = (EEPROM.read(EEPROM_AMOUNT_ADDR) << 8) | EEPROM.read(EEPROM_AMOUNT_ADDR + 1);
    config.checkInterval = EEPROM.read(EEPROM_INTERVAL_ADDR);
    config.soakTime = EEPROM.read(EEPROM_SOAK_TIME_ADDR);
    config.operatingMode = EEPROM.read(EEPROM_MODE_ADDR);
    config.sensorDry = (EEPROM.read(EEPROM_CAL_DRY_ADDR) << 8) | EEPROM.read(EEPROM_CAL_DRY_ADDR + 1);
    config.sensorWet = (EEPROM.read(EEPROM_CAL_WET_ADDR) << 8) | EEPROM.read(EEPROM_CAL_WET_ADDR + 1);

    // Load pump rate (float, 4 bytes)
    uint8_t* pumpRateBytes = (uint8_t*)&config.pumpRate;
    for (int i = 0; i < 4; i++) {
        pumpRateBytes[i] = EEPROM.read(EEPROM_PUMP_RATE_ADDR + i);
    }

    Serial.println(F("Config loaded from EEPROM"));
}

void saveConfig() {
    // Write magic number
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_NUMBER >> 8);
    EEPROM.write(EEPROM_MAGIC_ADDR + 1, EEPROM_MAGIC_NUMBER & 0xFF);

    // Write configuration
    EEPROM.write(EEPROM_TARGET_ADDR, config.targetHumidity);
    EEPROM.write(EEPROM_MIN_ADDR, config.minThreshold);
    EEPROM.write(EEPROM_AMOUNT_ADDR, config.waterAmount >> 8);
    EEPROM.write(EEPROM_AMOUNT_ADDR + 1, config.waterAmount & 0xFF);
    EEPROM.write(EEPROM_INTERVAL_ADDR, config.checkInterval);
    EEPROM.write(EEPROM_SOAK_TIME_ADDR, config.soakTime);
    EEPROM.write(EEPROM_MODE_ADDR, config.operatingMode);
    EEPROM.write(EEPROM_CAL_DRY_ADDR, config.sensorDry >> 8);
    EEPROM.write(EEPROM_CAL_DRY_ADDR + 1, config.sensorDry & 0xFF);
    EEPROM.write(EEPROM_CAL_WET_ADDR, config.sensorWet >> 8);
    EEPROM.write(EEPROM_CAL_WET_ADDR + 1, config.sensorWet & 0xFF);

    // Write pump rate (float, 4 bytes)
    uint8_t* pumpRateBytes = (uint8_t*)&config.pumpRate;
    for (int i = 0; i < 4; i++) {
        EEPROM.write(EEPROM_PUMP_RATE_ADDR + i, pumpRateBytes[i]);
    }

    // Calculate and write checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.write(EEPROM_CHECKSUM_ADDR, checksum);

    Serial.println(F("Config saved to EEPROM"));
}

uint8_t calculateChecksum() {
    uint8_t checksum = 0;
    for (int i = 0; i < EEPROM_CHECKSUM_ADDR; i++) {
        checksum ^= EEPROM.read(i);
    }
    return checksum;
}

void resetToDefaults() {
    config.targetHumidity = 80;
    config.minThreshold = 10;
    config.waterAmount = 500;
    config.checkInterval = 6;
    config.soakTime = 5;
    config.operatingMode = MODE_WATERING;
    config.sensorDry = 12400;
    config.sensorWet = 6000;
    config.pumpRate = 1.67;

    saveConfig();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Reset complete"));
    delay(2000);
}

// ============================================================================
// BUTTON HANDLING
// ============================================================================

void handleButtons() {
    unsigned long currentTime = millis();

    // Debouncing
    if (currentTime - lastButtonPress < DEBOUNCE_DELAY) {
        return;
    }

    if (digitalRead(BTN_UP) == LOW) {
        lastButtonPress = currentTime;
        handleUpButton();
        while (digitalRead(BTN_UP) == LOW); // Wait for release
    }
    else if (digitalRead(BTN_DOWN) == LOW) {
        lastButtonPress = currentTime;
        handleDownButton();
        while (digitalRead(BTN_DOWN) == LOW);
    }
    else if (digitalRead(BTN_SELECT) == LOW) {
        lastButtonPress = currentTime;
        handleSelectButton();
        while (digitalRead(BTN_SELECT) == LOW);
    }
    else if (digitalRead(BTN_BACK) == LOW) {
        lastButtonPress = currentTime;
        handleBackButton();
        while (digitalRead(BTN_BACK) == LOW);
    }
}

void handleUpButton() {
    switch (currentState) {
        case MAIN_MENU:
            menuIndex = (menuIndex > 0) ? menuIndex - 1 : 3;
            displayMenu();
            break;

        case SETTINGS_MENU:
            settingsIndex = (settingsIndex > 0) ? settingsIndex - 1 : 5;
            displayMenu();
            break;

        case CALIBRATE_MENU:
            calibrateIndex = (calibrateIndex > 0) ? calibrateIndex - 1 : 1;
            displayMenu();
            break;

        case SETTING_MODE:
            config.operatingMode = (config.operatingMode == MODE_WATERING) ? MODE_MONITOR : MODE_WATERING;
            displayMenu();
            break;

        case SETTING_TARGET:
            config.targetHumidity = min(100, config.targetHumidity + 5);
            displayMenu();
            break;

        case SETTING_MIN:
            config.minThreshold = min(100, config.minThreshold + 5);
            displayMenu();
            break;

        case SETTING_AMOUNT:
            config.waterAmount = min(9999, config.waterAmount + 10); // +1.0 mL
            displayMenu();
            break;

        case SETTING_INTERVAL:
            config.checkInterval = min(24, config.checkInterval + 1);
            displayMenu();
            break;

        case SETTING_SOAK_TIME:
            config.soakTime = min(30, config.soakTime + 1);
            displayMenu();
            break;

        case CAL_PUMP_ENTER_ML:
            pumpCalibrationML = min(9999, pumpCalibrationML + 10); // +1.0 mL
            displayMenu();
            break;
    }
}

void handleDownButton() {
    switch (currentState) {
        case MAIN_MENU:
            menuIndex = (menuIndex < 3) ? menuIndex + 1 : 0;
            displayMenu();
            break;

        case SETTINGS_MENU:
            settingsIndex = (settingsIndex < 5) ? settingsIndex + 1 : 0;
            displayMenu();
            break;

        case CALIBRATE_MENU:
            calibrateIndex = (calibrateIndex < 1) ? calibrateIndex + 1 : 0;
            displayMenu();
            break;

        case SETTING_MODE:
            config.operatingMode = (config.operatingMode == MODE_WATERING) ? MODE_MONITOR : MODE_WATERING;
            displayMenu();
            break;

        case SETTING_TARGET:
            config.targetHumidity = max(0, config.targetHumidity - 5);
            displayMenu();
            break;

        case SETTING_MIN:
            config.minThreshold = max(0, config.minThreshold - 5);
            displayMenu();
            break;

        case SETTING_AMOUNT:
            config.waterAmount = max(10, config.waterAmount - 10); // -1.0 mL
            displayMenu();
            break;

        case SETTING_INTERVAL:
            config.checkInterval = max(1, config.checkInterval - 1);
            displayMenu();
            break;

        case SETTING_SOAK_TIME:
            config.soakTime = max(1, config.soakTime - 1);
            displayMenu();
            break;

        case CAL_PUMP_ENTER_ML:
            pumpCalibrationML = max(10, pumpCalibrationML - 10); // -1.0 mL
            displayMenu();
            break;
    }
}

void handleSelectButton() {
    switch (currentState) {
        case MAIN_MENU:
            switch (menuIndex) {
                case 0: // Status
                    currentState = STATUS_SCREEN;
                    currentMoisture = readMoisture(); // Update reading
                    displayMenu();
                    break;
                case 1: // Settings
                    currentState = SETTINGS_MENU;
                    settingsIndex = 0;
                    displayMenu();
                    break;
                case 2: // Calibrate
                    currentState = CALIBRATE_MENU;
                    calibrateIndex = 0;
                    displayMenu();
                    break;
                case 3: // Manual Water
                    currentState = MANUAL_WATER_CONFIRM;
                    displayMenu();
                    break;
            }
            break;

        case SETTINGS_MENU:
            switch (settingsIndex) {
                case 0:
                    currentState = SETTING_MODE;
                    displayMenu();
                    break;
                case 1:
                    currentState = SETTING_TARGET;
                    displayMenu();
                    break;
                case 2:
                    currentState = SETTING_MIN;
                    displayMenu();
                    break;
                case 3:
                    currentState = SETTING_AMOUNT;
                    displayMenu();
                    break;
                case 4:
                    currentState = SETTING_INTERVAL;
                    displayMenu();
                    break;
                case 5:
                    currentState = SETTING_SOAK_TIME;
                    displayMenu();
                    break;
            }
            break;

        case CALIBRATE_MENU:
            switch (calibrateIndex) {
                case 0: // Calibrate Sensor
                    currentState = CAL_SENSOR_START;
                    displayMenu();
                    break;
                case 1: // Calibrate Pump
                    currentState = CAL_PUMP_START;
                    displayMenu();
                    break;
            }
            break;

        case CAL_SENSOR_START:
            currentState = CAL_SENSOR_AIR;
            displayMenu();
            break;

        case CAL_SENSOR_AIR:
            tempSensorDry = readMoistureRaw();
            currentState = CAL_SENSOR_WATER;
            displayMenu();
            break;

        case CAL_SENSOR_WATER:
            tempSensorWet = readMoistureRaw();
            config.sensorDry = tempSensorDry;
            config.sensorWet = tempSensorWet;
            saveConfig();
            currentState = CAL_SENSOR_DONE;
            displayMenu();
            break;

        case CAL_SENSOR_DONE:
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;

        case CAL_PUMP_START:
            currentState = CAL_PUMP_RUNNING;
            pumpCalibrationStart = millis();
            digitalWrite(PUMP_PIN, HIGH); // Start pump
            displayMenu();
            break;

        case CAL_PUMP_RUNNING:
            digitalWrite(PUMP_PIN, LOW); // Stop pump
            unsigned long pumpDuration = millis() - pumpCalibrationStart;
            pumpCalibrationML = 1000; // Default 100.0 mL
            currentState = CAL_PUMP_ENTER_ML;
            displayMenu();
            break;

        case CAL_PUMP_ENTER_ML:
            {
                unsigned long pumpDuration = millis() - pumpCalibrationStart;
                float seconds = pumpDuration / 1000.0;
                float mL = pumpCalibrationML / 10.0;
                config.pumpRate = mL / seconds;
                saveConfig();
                currentState = CAL_PUMP_DONE;
                displayMenu();
            }
            break;

        case CAL_PUMP_DONE:
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;

        case MANUAL_WATER_CONFIRM:
            currentState = MANUAL_WATER_RUNNING;
            displayMenu();
            waterPlant();
            currentState = MAIN_MENU;
            displayMenu();
            break;

        case RESET_CONFIRM:
            resetToDefaults();
            currentState = MAIN_MENU;
            displayMenu();
            break;

        case SETTING_MODE:
        case SETTING_TARGET:
        case SETTING_MIN:
        case SETTING_AMOUNT:
        case SETTING_INTERVAL:
        case SETTING_SOAK_TIME:
            saveConfig();
            currentState = SETTINGS_MENU;
            displayMenu();
            break;
    }
}

void handleBackButton() {
    switch (currentState) {
        case STATUS_SCREEN:
        case SETTINGS_MENU:
        case CALIBRATE_MENU:
        case MANUAL_WATER_CONFIRM:
            currentState = MAIN_MENU;
            menuIndex = 0;
            displayMenu();
            break;

        case SETTING_MODE:
        case SETTING_TARGET:
        case SETTING_MIN:
        case SETTING_AMOUNT:
        case SETTING_INTERVAL:
        case SETTING_SOAK_TIME:
            currentState = SETTINGS_MENU;
            displayMenu();
            break;

        case CAL_SENSOR_START:
        case CAL_SENSOR_AIR:
        case CAL_SENSOR_WATER:
        case CAL_SENSOR_DONE:
        case CAL_PUMP_START:
        case CAL_PUMP_DONE:
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;

        case CAL_PUMP_RUNNING:
            digitalWrite(PUMP_PIN, LOW); // Stop pump
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;

        case CAL_PUMP_ENTER_ML:
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;
    }
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

void displayMenu() {
    lcd.clear();

    switch (currentState) {
        case MAIN_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("MAIN MENU"));
            lcd.setCursor(0, 1);
            switch (menuIndex) {
                case 0: lcd.print(F(">Status")); break;
                case 1: lcd.print(F(">Settings")); break;
                case 2: lcd.print(F(">Calibrate")); break;
                case 3: lcd.print(F(">Manual Water")); break;
            }
            break;

        case STATUS_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print(F("Moisture: "));
            lcd.print(currentMoisture);
            lcd.print(F("%"));
            lcd.setCursor(0, 1);

            if (config.operatingMode == MODE_MONITOR) {
                // Monitor mode: show logging status
                lcd.print(F("MONITOR mode"));
            } else {
                // Watering mode: show dry cycle status
                if (currentMoisture <= config.minThreshold) {
                    lcd.print(F("DRY-Need water"));
                } else {
                    lcd.print(F("OK-Dry cycle"));
                }
            }
            break;

        case SETTINGS_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("SETTINGS"));
            lcd.setCursor(0, 1);
            switch (settingsIndex) {
                case 0: lcd.print(F(">Mode")); break;
                case 1: lcd.print(F(">Target")); break;
                case 2: lcd.print(F(">Min Threshold")); break;
                case 3: lcd.print(F(">Water Amount")); break;
                case 4: lcd.print(F(">Check Interval")); break;
                case 5: lcd.print(F(">Soak Time")); break;
            }
            break;

        case SETTING_MODE:
            lcd.setCursor(0, 0);
            lcd.print(F("Operating Mode"));
            lcd.setCursor(0, 1);
            if (config.operatingMode == MODE_WATERING) {
                lcd.print(F("Watering"));
            } else {
                lcd.print(F("Monitor"));
            }
            lcd.print(F(" UP/DN"));
            break;

        case SETTING_TARGET:
            lcd.setCursor(0, 0);
            lcd.print(F("Target Humidity"));
            lcd.setCursor(0, 1);
            lcd.print(config.targetHumidity);
            lcd.print(F("%  UP/DN SEL"));
            break;

        case SETTING_MIN:
            lcd.setCursor(0, 0);
            lcd.print(F("Min Threshold"));
            lcd.setCursor(0, 1);
            lcd.print(config.minThreshold);
            lcd.print(F("%  UP/DN SEL"));
            break;

        case SETTING_AMOUNT:
            lcd.setCursor(0, 0);
            lcd.print(F("Water Amount"));
            lcd.setCursor(0, 1);
            lcd.print(config.waterAmount / 10.0, 1);
            lcd.print(F("mL UP/DN SEL"));
            break;

        case SETTING_INTERVAL:
            lcd.setCursor(0, 0);
            lcd.print(F("Check Interval"));
            lcd.setCursor(0, 1);
            lcd.print(config.checkInterval);
            lcd.print(F("h  UP/DN SEL"));
            break;

        case SETTING_SOAK_TIME:
            lcd.setCursor(0, 0);
            lcd.print(F("Soak Time"));
            lcd.setCursor(0, 1);
            lcd.print(config.soakTime);
            lcd.print(F("min UP/DN SEL"));
            break;

        case CALIBRATE_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("CALIBRATE"));
            lcd.setCursor(0, 1);
            switch (calibrateIndex) {
                case 0: lcd.print(F(">Sensor")); break;
                case 1: lcd.print(F(">Pump")); break;
            }
            break;

        case CAL_SENSOR_START:
            lcd.setCursor(0, 0);
            lcd.print(F("Sensor Cal"));
            lcd.setCursor(0, 1);
            lcd.print(F("Press SEL"));
            break;

        case CAL_SENSOR_AIR:
            lcd.setCursor(0, 0);
            lcd.print(F("Hold in AIR"));
            lcd.setCursor(0, 1);
            lcd.print(F("Press SEL"));
            break;

        case CAL_SENSOR_WATER:
            lcd.setCursor(0, 0);
            lcd.print(F("Put in WATER"));
            lcd.setCursor(0, 1);
            lcd.print(F("Press SEL"));
            break;

        case CAL_SENSOR_DONE:
            lcd.setCursor(0, 0);
            lcd.print(F("Cal Complete!"));
            lcd.setCursor(0, 1);
            lcd.print(F("Dry:"));
            lcd.print(config.sensorDry);
            lcd.print(F(" "));
            lcd.print(config.sensorWet);
            break;

        case CAL_PUMP_START:
            lcd.setCursor(0, 0);
            lcd.print(F("Pump Cal"));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Start pump"));
            break;

        case CAL_PUMP_RUNNING:
            lcd.setCursor(0, 0);
            lcd.print(F("Pumping..."));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Stop"));
            break;

        case CAL_PUMP_ENTER_ML:
            lcd.setCursor(0, 0);
            lcd.print(F("How much water?"));
            lcd.setCursor(0, 1);
            lcd.print(pumpCalibrationML / 10.0, 1);
            lcd.print(F("mL UP/DN SEL"));
            break;

        case CAL_PUMP_DONE:
            lcd.setCursor(0, 0);
            lcd.print(F("Cal Complete!"));
            lcd.setCursor(0, 1);
            lcd.print(config.pumpRate, 2);
            lcd.print(F("mL/s"));
            break;

        case MANUAL_WATER_CONFIRM:
            lcd.setCursor(0, 0);
            lcd.print(F("Manual Water?"));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Yes BACK=No"));
            break;

        case MANUAL_WATER_RUNNING:
            lcd.setCursor(0, 0);
            lcd.print(F("Watering..."));
            lcd.setCursor(0, 1);
            lcd.print(F("Please wait"));
            break;

        case RESET_CONFIRM:
            lcd.setCursor(0, 0);
            lcd.print(F("Reset to def?"));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Yes BACK=No"));
            break;
    }
}
