/*
 * Soil Humidity Monitor - Monitoring Variant
 * Version: 1.4
 *
 * Hardware: Arduino UNO R4 WiFi
 * Display: 16x2 I2C LCD
 *
 * Features:
 * - Continuous soil moisture monitoring with live ADC display
 * - EEPROM circular buffer logging (14 days @ 15-min intervals, 4 bytes/entry)
 * - CSV data download via Serial (median ADC only, min/max in live serial)
 * - Sensor calibration wizard with live ADC feedback
 * - Manual calibration value editing
 * - Configurable parameters (log interval, sensor timing, backlight timeout)
 * - DIS-pin controlled sensor (extends lifespan)
 * - No watering functionality (monitoring only)
 *
 * Pin Assignments:
 * - A0: Soil Moisture Sensor (ADC)
 * - D2: Sensor Disable Control (DIS pin)
 * - D3: UNUSED (pump pin in POC)
 * - SDA (A4): LCD SDA
 * - SCL (A5): LCD SCL
 * - D4: Button UP
 * - D5: Button DOWN
 * - D6: Button SELECT
 * - D7: Button BACK
 * - D10-D13: UNUSED (SD card in POC)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// ============================================================================
// VERSION INFORMATION
// ============================================================================

#define VERSION_MAJOR       1
#define VERSION_MINOR       4
#define VERSION_STRING      "1.4"
#define FIRMWARE_VERSION    ((VERSION_MAJOR << 8) | VERSION_MINOR)  // 0x0104

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

#define SENSOR_ADC_PIN      A0    // Soil moisture sensor analog input
#define SENSOR_DISABLE_PIN  2     // DIS pin control (HIGH=disabled, LOW=enabled)
#define BTN_UP              4     // Navigation button: UP
#define BTN_DOWN            5     // Navigation button: DOWN
#define BTN_SELECT          6     // Navigation button: SELECT
#define BTN_BACK            7     // Navigation button: BACK

// ============================================================================
// LCD CONFIGURATION
// ============================================================================

// Try 0x27 first, if that doesn't work try 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================================
// EEPROM MEMORY MAP
// ============================================================================

// Header section (32 bytes: 0-31)
#define EEPROM_MAGIC_ADDR           0    // 2 bytes: Magic number (0xA5C3)
#define EEPROM_VERSION_ADDR         2    // 2 bytes: Firmware version
#define EEPROM_LOG_INTERVAL_ADDR    4    // 2 bytes: Log interval (minutes)
#define EEPROM_CAL_DRY_ADDR         6    // 2 bytes: Sensor dry ADC value
#define EEPROM_CAL_WET_ADDR         8    // 2 bytes: Sensor wet ADC value
#define EEPROM_WRITE_PTR_ADDR       10   // 4 bytes: Circular buffer write pointer
#define EEPROM_TOTAL_ENTRIES_ADDR   14   // 4 bytes: Total entries written
#define EEPROM_FIRST_TS_ADDR        18   // 2 bytes: First entry timestamp (hours)
#define EEPROM_FLAGS_ADDR           20   // 1 byte: Buffer flags
// New configuration parameters (v1.2)
#define EEPROM_BACKLIGHT_TIMEOUT_ADDR  21  // 2 bytes: Backlight timeout (minutes)
#define EEPROM_SENSOR_WARMUP_ADDR      23  // 2 bytes: Sensor warmup (milliseconds)
#define EEPROM_NUM_MEASUREMENTS_ADDR   25  // 1 byte: Number of measurements (5-50)
#define EEPROM_MEASUREMENT_DELAY_ADDR  26  // 2 bytes: Delay between measurements (ms)
// Bytes 28-30: Reserved for future expansion
#define EEPROM_HEADER_CHECKSUM_ADDR 31   // 1 byte: Header checksum

// Data section (32-5407: 5376 bytes = 1344 entries × 4 bytes)
#define EEPROM_DATA_START_ADDR      32
#define EEPROM_DATA_END_ADDR        5407
#define EEPROM_DATA_SIZE            5376
#define ENTRY_SIZE                  4
#define MAX_ENTRIES                 1344

#define EEPROM_MAGIC_NUMBER         0xA5C3

// Entry flags
#define FLAG_SENSOR_ERROR    0x10  // bit 4: Sensor error detected
#define FLAG_OUT_OF_RANGE    0x20  // bit 5: Reading out of range

// Buffer flags
#define FLAG_BUFFER_WRAPPED  0x01  // bit 0: Circular buffer has wrapped

// ============================================================================
// CONFIGURATION STRUCTURE
// ============================================================================

struct Config {
    uint16_t logInterval;           // Log interval in minutes (1-60)
    uint16_t sensorDry;             // ADC value when sensor is in air (14-bit: 0-16383)
    uint16_t sensorWet;             // ADC value when sensor is in water (14-bit: 0-16383)
    uint32_t writePointer;          // Current write position in circular buffer
    uint32_t totalEntries;          // Total entries written (lifetime)
    uint16_t firstTimestamp;        // Hours since boot for first entry
    uint8_t flags;                  // Buffer status flags

    // New in v1.2
    uint16_t backlightTimeout;      // Backlight timeout (minutes, 1-60)
    uint16_t sensorWarmup;          // Sensor warmup delay (milliseconds, 100-2000)
    uint8_t numMeasurements;        // Number of samples for median (5-50)
    uint16_t measurementDelay;      // Delay between samples (milliseconds, 10-500)
};

// Default configuration
Config config = {
    .logInterval = 15,              // 15 minutes (14 days @ 4 bytes/entry)
    .sensorDry = 12400,             // Typical 14-bit ADC value in air
    .sensorWet = 6000,              // Typical 14-bit ADC value in water
    .writePointer = 0,              // Start of buffer
    .totalEntries = 0,              // No entries yet
    .firstTimestamp = 0,            // 0 hours (boot time)
    .flags = 0,                     // No flags set

    // v1.2 defaults
    .backlightTimeout = 1,          // 1 minute
    .sensorWarmup = 500,            // 500ms (v1.4: DIS control, VCC stable)
    .numMeasurements = 10,          // 10 samples
    .measurementDelay = 100         // 100ms between samples
};

// ============================================================================
// LOG ENTRY STRUCTURE
// ============================================================================

struct LogEntry {
    uint8_t moisturePercent;        // 0-100%
    uint16_t rawADC;                // Median ADC value (14-bit: 0-16383)
    uint8_t flags;                  // Status flags
};

// ============================================================================
// MOISTURE STATISTICS STRUCTURE
// ============================================================================

// Structure to hold moisture statistics (median + min/max)
struct MoistureStats {
    int median;
    int minADC;
    int maxADC;
};

// ============================================================================
// MENU STATE MACHINE
// ============================================================================

enum MenuState {
    STATUS_SCREEN,                  // Default: Live moisture display
    MAIN_MENU,                     // Menu selection
    SETTINGS_MENU,                 // Settings submenu
    SETTING_LOG_INTERVAL,          // Adjust log interval
    SETTING_BACKLIGHT_TIMEOUT,     // Adjust LCD sleep timeout (v1.2)
    SETTING_SENSOR_WARMUP,         // Adjust sensor warmup delay (v1.2)
    SETTING_NUM_MEASUREMENTS,      // Adjust number of samples (v1.2)
    SETTING_MEASUREMENT_DELAY,     // Adjust delay between samples (v1.2)
    CALIBRATE_MENU,                // Calibration submenu (Dry/Wet selection)
    CAL_DRY_MENU,          // Dry value submenu (Measure/Edit)
    CAL_WET_MENU,          // Wet value submenu (Measure/Edit)
    CAL_DRY_MEASURE,       // Measure dry value (in air)
    CAL_WET_MEASURE,       // Measure wet value (in water)
    CAL_DRY_EDIT,          // Edit dry value manually
    CAL_WET_EDIT,          // Edit wet value manually
    CAL_MEASURE_DONE,      // Measurement complete (show result)
    DOWNLOAD_MENU,         // Download data menu
    DOWNLOAD_CONFIRM,      // Confirm download
    DOWNLOAD_PROGRESS,     // Downloading...
    DOWNLOAD_COMPLETE,     // Download done
    CLEAR_DATA_CONFIRM,    // Confirm clear data
    SYSTEM_INFO,           // System information
    RESET_CONFIRM          // Confirm reset to defaults
};

MenuState currentState = STATUS_SCREEN;
uint8_t menuIndex = 0;           // Current menu selection
uint8_t settingsIndex = 0;       // Settings menu item index
uint8_t calibrateIndex = 0;      // Calibrate menu item index (0=Dry, 1=Wet)
uint8_t calDryIndex = 0;         // Dry calibration submenu index (0=Measure, 1=Edit)
uint8_t calWetIndex = 0;         // Wet calibration submenu index (0=Measure, 1=Edit)

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

int currentMoisture = 0;         // Current moisture reading (%)
int currentRawADC = 0;           // Current raw ADC value
unsigned long lastLogTime = 0;   // Last data log timestamp
unsigned long lastDisplayUpdate = 0; // Last display update timestamp
bool backlightOn = true;         // LCD backlight state
unsigned long lastActivity = 0;  // Last user activity timestamp

// Button debouncing
unsigned long lastButtonPress = 0;
#define DEBOUNCE_DELAY 200       // 200ms debounce

// Display update intervals
#define DISPLAY_UPDATE_INTERVAL 2000    // 2 seconds
// #define BACKLIGHT_TIMEOUT 60000      // REMOVED - now configurable via Settings menu

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize serial for debugging and CSV download
    Serial.begin(115200);
    Serial.print(F("Soil Humidity Monitor v"));
    Serial.println(F(VERSION_STRING));

    // Configure ADC for 14-bit resolution (0-16383)
    // CRITICAL: Without this, Arduino defaults to 10-bit (0-1023) for compatibility
    analogReadResolution(14);

    // Initialize pins
    pinMode(SENSOR_DISABLE_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    // Ensure sensor is disabled initially
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Soil Monitor"));
    lcd.setCursor(0, 1);
    lcd.print(F("v"));
    lcd.print(F(VERSION_STRING));
    delay(2000);

    // Load configuration from EEPROM
    loadConfig();

    // Take initial moisture reading
    currentMoisture = readMoisture();

    // Display status screen
    displayMenu();

    Serial.println(F("Setup complete"));
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    unsigned long currentTime = millis();

    // Handle button input (always responsive)
    handleButtons();

    // Update display if on status screen AND backlight is on
    if (currentState == STATUS_SCREEN && backlightOn &&
        currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        currentMoisture = readMoisture();
        displayMenu();
        lastDisplayUpdate = millis();  // Reset timer AFTER sensor reading completes
    }

    // Update display during calibration to show live ADC values
    if ((currentState == CAL_DRY_MEASURE || currentState == CAL_WET_MEASURE) && backlightOn &&
        currentTime - lastDisplayUpdate >= 500) {  // Update every 500ms for faster feedback
        currentRawADC = readMoistureRaw();
        displayMenu();
        lastDisplayUpdate = millis();  // Reset timer AFTER sensor reading completes
    }

    // Log moisture reading at configured interval
    unsigned long logIntervalMs = (unsigned long)config.logInterval * 60000UL;
    if (currentTime - lastLogTime >= logIntervalMs) {
        lastLogTime = currentTime;
        logMoistureReading();
    }

    // Update backlight (auto-sleep)
    updateBacklight();

    delay(50);  // Small delay to reduce CPU usage
}

// ============================================================================
// SENSOR READING
// ============================================================================

int readMoisture() {
    digitalWrite(SENSOR_DISABLE_PIN, LOW);  // Enable sensor (DIS LOW = active)
    delay(config.sensorWarmup);  // Use configurable warmup time

    // Allocate sample array dynamically based on config
    int* samples = new int[config.numMeasurements];
    int minADC = 16383;  // Track minimum
    int maxADC = 0;      // Track maximum

    // Collect samples (NO DISCARDING - warmup handles stabilization)
    for (int i = 0; i < config.numMeasurements; i++) {
        int reading = analogRead(SENSOR_ADC_PIN);
        samples[i] = reading;

        // Track min/max
        if (reading < minADC) minADC = reading;
        if (reading > maxADC) maxADC = reading;

        if (i < config.numMeasurements - 1) {
            delay(config.measurementDelay);  // Configurable delay
        }
    }

    // Sort for median (bubble sort)
    for (int i = 0; i < config.numMeasurements - 1; i++) {
        for (int j = 0; j < config.numMeasurements - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                int temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    // Calculate median (handles both odd and even sample counts)
    int medianReading;
    if (config.numMeasurements % 2 == 0) {
        int mid = config.numMeasurements / 2;
        medianReading = (samples[mid - 1] + samples[mid]) / 2;
    } else {
        medianReading = samples[config.numMeasurements / 2];
    }

    delete[] samples;  // Free memory

    currentRawADC = medianReading;
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);  // Disable sensor (DIS HIGH = sleep)

    // Convert to percentage using calibration values
    int moisturePercent = map(medianReading, config.sensorDry, config.sensorWet, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.print(F("ADC: "));
    Serial.print(medianReading);
    Serial.print(F(" ("));
    Serial.print(minADC);
    Serial.print(F("-"));
    Serial.print(maxADC);
    Serial.print(F(") -> "));
    Serial.print(moisturePercent);
    Serial.println(F("%"));

    return moisturePercent;
}

int readMoistureRaw() {
    // Read raw ADC value (for calibration - returns median only)
    digitalWrite(SENSOR_DISABLE_PIN, LOW);  // Enable sensor (DIS LOW = active)
    delay(config.sensorWarmup);  // Configurable

    int* samples = new int[config.numMeasurements];

    for (int i = 0; i < config.numMeasurements; i++) {
        samples[i] = analogRead(SENSOR_ADC_PIN);
        if (i < config.numMeasurements - 1) {
            delay(config.measurementDelay);
        }
    }

    // Sort for median
    for (int i = 0; i < config.numMeasurements - 1; i++) {
        for (int j = 0; j < config.numMeasurements - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                int temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    int median;
    if (config.numMeasurements % 2 == 0) {
        int mid = config.numMeasurements / 2;
        median = (samples[mid - 1] + samples[mid]) / 2;
    } else {
        median = samples[config.numMeasurements / 2];
    }

    delete[] samples;
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);  // Disable sensor (DIS HIGH = sleep)

    return median;
}

MoistureStats readMoistureWithStats() {
    // Read moisture with full statistics (median + min + max) for logging
    digitalWrite(SENSOR_DISABLE_PIN, LOW);  // Enable sensor (DIS LOW = active)
    delay(config.sensorWarmup);

    int* samples = new int[config.numMeasurements];
    int minADC = 16383;
    int maxADC = 0;

    for (int i = 0; i < config.numMeasurements; i++) {
        int reading = analogRead(SENSOR_ADC_PIN);
        samples[i] = reading;
        if (reading < minADC) minADC = reading;
        if (reading > maxADC) maxADC = reading;
        if (i < config.numMeasurements - 1) {
            delay(config.measurementDelay);
        }
    }

    // Sort for median
    for (int i = 0; i < config.numMeasurements - 1; i++) {
        for (int j = 0; j < config.numMeasurements - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                int temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    int median;
    if (config.numMeasurements % 2 == 0) {
        int mid = config.numMeasurements / 2;
        median = (samples[mid - 1] + samples[mid]) / 2;
    } else {
        median = samples[config.numMeasurements / 2];
    }

    delete[] samples;
    digitalWrite(SENSOR_DISABLE_PIN, HIGH);  // Disable sensor (DIS HIGH = sleep)

    MoistureStats stats = {median, minADC, maxADC};
    return stats;
}

uint8_t detectSensorErrors(int rawADC) {
    uint8_t flags = 0;

    // Check if sensor is disconnected (reading at extremes)
    if (rawADC < 100 || rawADC > 16000) {
        flags |= FLAG_SENSOR_ERROR;
    }

    // Check if reading is outside calibrated range
    if (rawADC < config.sensorWet - 500 || rawADC > config.sensorDry + 500) {
        flags |= FLAG_OUT_OF_RANGE;
    }

    return flags;
}

// ============================================================================
// DATA LOGGING (CIRCULAR BUFFER)
// ============================================================================

void logMoistureReading() {
    MoistureStats stats = readMoistureWithStats();

    int moisture = map(stats.median, config.sensorDry, config.sensorWet, 0, 100);
    moisture = constrain(moisture, 0, 100);

    uint8_t flags = detectSensorErrors(stats.median);

    writeLogEntry(moisture, stats.median, flags);

    Serial.print(F("Logged entry #"));
    Serial.print(config.totalEntries);
    Serial.print(F(": "));
    Serial.print(moisture);
    Serial.print(F("% (ADC="));
    Serial.print(stats.median);
    Serial.print(F(", min="));
    Serial.print(stats.minADC);
    Serial.print(F(", max="));
    Serial.print(stats.maxADC);
    Serial.println(F(")"));

    // Update display if on status screen
    if (currentState == STATUS_SCREEN) {
        displayMenu();
    }
}

void writeLogEntry(uint8_t moisture, uint16_t medianADC, uint8_t flags) {
    // Calculate EEPROM address in circular buffer
    uint16_t entryOffset = (config.writePointer % MAX_ENTRIES) * ENTRY_SIZE;
    uint16_t addr = EEPROM_DATA_START_ADDR + entryOffset;

    // Write 4-byte entry
    EEPROM.write(addr + 0, moisture);
    EEPROM.write(addr + 1, medianADC >> 8);      // Median high byte
    EEPROM.write(addr + 2, medianADC & 0xFF);    // Median low byte
    EEPROM.write(addr + 3, flags);

    // Update write pointer and total count
    config.writePointer++;
    config.totalEntries++;

    // Set buffer wrapped flag if we've filled the buffer
    if (config.writePointer >= MAX_ENTRIES) {
        config.flags |= FLAG_BUFFER_WRAPPED;
    }

    // Save metadata (write pointer, total entries, flags)
    saveMetadata();
}

LogEntry readLogEntry(uint32_t index) {
    LogEntry entry;

    // Calculate actual position accounting for wrapping
    uint32_t actualIndex;
    if (config.flags & FLAG_BUFFER_WRAPPED) {
        // Buffer has wrapped - oldest entry is at writePointer position
        actualIndex = (config.writePointer + index) % MAX_ENTRIES;
    } else {
        // Buffer hasn't wrapped yet - read from start
        actualIndex = index;
    }

    uint16_t addr = EEPROM_DATA_START_ADDR + (actualIndex * ENTRY_SIZE);

    entry.moisturePercent = EEPROM.read(addr + 0);
    entry.rawADC = (EEPROM.read(addr + 1) << 8) | EEPROM.read(addr + 2);
    entry.flags = EEPROM.read(addr + 3);

    return entry;
}

uint32_t getEntryCount() {
    if (config.flags & FLAG_BUFFER_WRAPPED) {
        return MAX_ENTRIES;  // Buffer full
    } else {
        return config.writePointer;  // Partial buffer
    }
}

void clearAllData() {
    config.writePointer = 0;
    config.totalEntries = 0;
    config.firstTimestamp = millis() / 3600000UL;  // Hours since boot
    config.flags = 0;  // Clear wrapped flag

    saveConfig();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Data cleared!"));
    delay(2000);
}

// ============================================================================
// SERIAL CSV DOWNLOAD
// ============================================================================

void downloadDataToSerial() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Downloading..."));

    uint32_t entryCount = getEntryCount();

    // Send CSV header
    Serial.println(F("# Soil Humidity Monitor Data Export"));
    Serial.print(F("# Firmware Version: "));
    Serial.println(F(VERSION_STRING));
    Serial.print(F("# Total Entries: "));
    Serial.println(entryCount);
    Serial.print(F("# Log Interval: "));
    Serial.print(config.logInterval);
    Serial.println(F(" minutes"));
    Serial.print(F("# Sensor Calibration: Dry="));
    Serial.print(config.sensorDry);
    Serial.print(F(", Wet="));
    Serial.println(config.sensorWet);
    Serial.print(F("# Buffer Status: "));
    Serial.println((config.flags & FLAG_BUFFER_WRAPPED) ? F("WRAPPED") : F("NOT_WRAPPED"));
    Serial.println(F("#"));
    Serial.println(F("Entry,Timestamp_Hours,Timestamp_Minutes,Moisture_%,Median_ADC,Flags"));

    // Send data rows
    for (uint32_t i = 0; i < entryCount; i++) {
        LogEntry entry = readLogEntry(i);

        // Calculate timestamp
        float hours = config.firstTimestamp + (i * config.logInterval / 60.0);
        uint32_t minutes = i * config.logInterval;

        // CSV row: Entry,Hours,Minutes,Moisture,Median,Flags
        Serial.print(i + 1);
        Serial.print(',');
        Serial.print(hours, 2);
        Serial.print(',');
        Serial.print(minutes);
        Serial.print(',');
        Serial.print(entry.moisturePercent);
        Serial.print(',');
        Serial.print(entry.rawADC);      // Median
        Serial.print(',');
        Serial.print(F("0x"));
        Serial.println(entry.flags, HEX);

        // Update progress display every 50 entries
        if (i % 50 == 0) {
            updateProgressBar(i, entryCount);
        }

        delay(10);  // Small delay to prevent serial buffer overflow
    }

    // Footer
    Serial.println(F("#"));
    Serial.println(F("# End of data"));

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Download done!"));
    lcd.setCursor(0, 1);
    lcd.print(entryCount);
    lcd.print(F(" entries"));
    delay(3000);
}

void updateProgressBar(uint32_t current, uint32_t total) {
    uint8_t percent = (current * 100) / total;
    uint8_t barLength = (current * 10) / total;  // 10 chars max

    lcd.setCursor(0, 1);
    lcd.print('[');
    for (uint8_t i = 0; i < 10; i++) {
        lcd.print(i < barLength ? '#' : ' ');
    }
    lcd.print(']');
    lcd.print(percent);
    lcd.print('%');
}

// ============================================================================
// EEPROM CONFIGURATION FUNCTIONS
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
    uint8_t storedChecksum = EEPROM.read(EEPROM_HEADER_CHECKSUM_ADDR);
    uint8_t calculatedChecksum = calculateChecksum();

    if (storedChecksum != calculatedChecksum) {
        Serial.println(F("Config checksum mismatch, using defaults"));
        saveConfig();
        return;
    }

    // Load configuration
    config.logInterval = (EEPROM.read(EEPROM_LOG_INTERVAL_ADDR) << 8) | EEPROM.read(EEPROM_LOG_INTERVAL_ADDR + 1);
    config.sensorDry = (EEPROM.read(EEPROM_CAL_DRY_ADDR) << 8) | EEPROM.read(EEPROM_CAL_DRY_ADDR + 1);
    config.sensorWet = (EEPROM.read(EEPROM_CAL_WET_ADDR) << 8) | EEPROM.read(EEPROM_CAL_WET_ADDR + 1);

    // Load buffer pointers (4 bytes each)
    config.writePointer = ((uint32_t)EEPROM.read(EEPROM_WRITE_PTR_ADDR) << 24) |
                          ((uint32_t)EEPROM.read(EEPROM_WRITE_PTR_ADDR + 1) << 16) |
                          ((uint32_t)EEPROM.read(EEPROM_WRITE_PTR_ADDR + 2) << 8) |
                          (uint32_t)EEPROM.read(EEPROM_WRITE_PTR_ADDR + 3);

    config.totalEntries = ((uint32_t)EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR) << 24) |
                          ((uint32_t)EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 1) << 16) |
                          ((uint32_t)EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 2) << 8) |
                          (uint32_t)EEPROM.read(EEPROM_TOTAL_ENTRIES_ADDR + 3);

    config.firstTimestamp = (EEPROM.read(EEPROM_FIRST_TS_ADDR) << 8) | EEPROM.read(EEPROM_FIRST_TS_ADDR + 1);
    config.flags = EEPROM.read(EEPROM_FLAGS_ADDR);

    // Load v1.2 parameters (with defaults if not present or invalid)
    config.backlightTimeout = (EEPROM.read(EEPROM_BACKLIGHT_TIMEOUT_ADDR) << 8) |
                              EEPROM.read(EEPROM_BACKLIGHT_TIMEOUT_ADDR + 1);
    if (config.backlightTimeout < 1 || config.backlightTimeout > 60) {
        config.backlightTimeout = 1;  // Default 1 minute
    }

    config.sensorWarmup = (EEPROM.read(EEPROM_SENSOR_WARMUP_ADDR) << 8) |
                          EEPROM.read(EEPROM_SENSOR_WARMUP_ADDR + 1);
    if (config.sensorWarmup < 100 || config.sensorWarmup > 2000) {
        config.sensorWarmup = 1000;  // Default 1000ms
    }

    config.numMeasurements = EEPROM.read(EEPROM_NUM_MEASUREMENTS_ADDR);
    if (config.numMeasurements < 5 || config.numMeasurements > 50) {
        config.numMeasurements = 10;  // Default 10 samples
    }

    config.measurementDelay = (EEPROM.read(EEPROM_MEASUREMENT_DELAY_ADDR) << 8) |
                              EEPROM.read(EEPROM_MEASUREMENT_DELAY_ADDR + 1);
    if (config.measurementDelay < 10 || config.measurementDelay > 500) {
        config.measurementDelay = 100;  // Default 100ms
    }

    Serial.println(F("Config loaded from EEPROM"));
}

void saveConfig() {
    // Write magic number
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_NUMBER >> 8);
    EEPROM.write(EEPROM_MAGIC_ADDR + 1, EEPROM_MAGIC_NUMBER & 0xFF);

    // Write version
    EEPROM.write(EEPROM_VERSION_ADDR, FIRMWARE_VERSION >> 8);
    EEPROM.write(EEPROM_VERSION_ADDR + 1, FIRMWARE_VERSION & 0xFF);

    // Write configuration
    EEPROM.write(EEPROM_LOG_INTERVAL_ADDR, config.logInterval >> 8);
    EEPROM.write(EEPROM_LOG_INTERVAL_ADDR + 1, config.logInterval & 0xFF);
    EEPROM.write(EEPROM_CAL_DRY_ADDR, config.sensorDry >> 8);
    EEPROM.write(EEPROM_CAL_DRY_ADDR + 1, config.sensorDry & 0xFF);
    EEPROM.write(EEPROM_CAL_WET_ADDR, config.sensorWet >> 8);
    EEPROM.write(EEPROM_CAL_WET_ADDR + 1, config.sensorWet & 0xFF);

    // Write v1.2 parameters
    EEPROM.write(EEPROM_BACKLIGHT_TIMEOUT_ADDR, config.backlightTimeout >> 8);
    EEPROM.write(EEPROM_BACKLIGHT_TIMEOUT_ADDR + 1, config.backlightTimeout & 0xFF);

    EEPROM.write(EEPROM_SENSOR_WARMUP_ADDR, config.sensorWarmup >> 8);
    EEPROM.write(EEPROM_SENSOR_WARMUP_ADDR + 1, config.sensorWarmup & 0xFF);

    EEPROM.write(EEPROM_NUM_MEASUREMENTS_ADDR, config.numMeasurements);

    EEPROM.write(EEPROM_MEASUREMENT_DELAY_ADDR, config.measurementDelay >> 8);
    EEPROM.write(EEPROM_MEASUREMENT_DELAY_ADDR + 1, config.measurementDelay & 0xFF);

    // Write buffer pointers and metadata
    saveMetadata();

    // Calculate and write checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.write(EEPROM_HEADER_CHECKSUM_ADDR, checksum);

    Serial.println(F("Config saved to EEPROM"));
}

void saveMetadata() {
    // Write write pointer (4 bytes)
    EEPROM.write(EEPROM_WRITE_PTR_ADDR, (config.writePointer >> 24) & 0xFF);
    EEPROM.write(EEPROM_WRITE_PTR_ADDR + 1, (config.writePointer >> 16) & 0xFF);
    EEPROM.write(EEPROM_WRITE_PTR_ADDR + 2, (config.writePointer >> 8) & 0xFF);
    EEPROM.write(EEPROM_WRITE_PTR_ADDR + 3, config.writePointer & 0xFF);

    // Write total entries (4 bytes)
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR, (config.totalEntries >> 24) & 0xFF);
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR + 1, (config.totalEntries >> 16) & 0xFF);
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR + 2, (config.totalEntries >> 8) & 0xFF);
    EEPROM.write(EEPROM_TOTAL_ENTRIES_ADDR + 3, config.totalEntries & 0xFF);

    // Write first timestamp (2 bytes)
    EEPROM.write(EEPROM_FIRST_TS_ADDR, config.firstTimestamp >> 8);
    EEPROM.write(EEPROM_FIRST_TS_ADDR + 1, config.firstTimestamp & 0xFF);

    // Write flags
    EEPROM.write(EEPROM_FLAGS_ADDR, config.flags);

    // Update checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.write(EEPROM_HEADER_CHECKSUM_ADDR, checksum);
}

uint8_t calculateChecksum() {
    uint8_t checksum = 0;
    for (int i = 0; i < EEPROM_HEADER_CHECKSUM_ADDR; i++) {
        checksum ^= EEPROM.read(i);
    }
    return checksum;
}

void resetToDefaults() {
    config.logInterval = 15;
    config.sensorDry = 12400;
    config.sensorWet = 6000;
    config.writePointer = 0;
    config.totalEntries = 0;
    config.firstTimestamp = millis() / 3600000UL;
    config.flags = 0;

    // v1.2 defaults
    config.backlightTimeout = 1;        // 1 minute
    config.sensorWarmup = 1000;         // 1000ms
    config.numMeasurements = 10;        // 10 samples
    config.measurementDelay = 100;      // 100ms

    saveConfig();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Reset complete"));
    delay(2000);
}

// ============================================================================
// BACKLIGHT MANAGEMENT
// ============================================================================

void updateBacklight() {
    unsigned long timeoutMs = (unsigned long)config.backlightTimeout * 60000UL;
    if (backlightOn && (millis() - lastActivity > timeoutMs)) {
        lcd.noBacklight();
        backlightOn = false;
        Serial.println(F("Backlight off (timeout)"));
    }
}

void wakeBacklight() {
    lastActivity = millis();
    if (!backlightOn) {
        lcd.backlight();
        backlightOn = true;
        // Take fresh reading when waking up on status screen
        if (currentState == STATUS_SCREEN) {
            currentMoisture = readMoisture();
            displayMenu();
        }
    }
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
        wakeBacklight();
        handleUpButton();
        while (digitalRead(BTN_UP) == LOW); // Wait for release
    }
    else if (digitalRead(BTN_DOWN) == LOW) {
        lastButtonPress = currentTime;
        wakeBacklight();
        handleDownButton();
        while (digitalRead(BTN_DOWN) == LOW);
    }
    else if (digitalRead(BTN_SELECT) == LOW) {
        lastButtonPress = currentTime;
        wakeBacklight();
        handleSelectButton();
        while (digitalRead(BTN_SELECT) == LOW);
    }
    else if (digitalRead(BTN_BACK) == LOW) {
        lastButtonPress = currentTime;
        wakeBacklight();
        handleBackButton();
        while (digitalRead(BTN_BACK) == LOW);
    }
}

void handleUpButton() {
    switch (currentState) {
        case MAIN_MENU:
            menuIndex = (menuIndex > 0) ? menuIndex - 1 : 5;
            displayMenu();
            break;

        case SETTINGS_MENU:
            settingsIndex = (settingsIndex > 0) ? settingsIndex - 1 : 4;
            displayMenu();
            break;

        case CALIBRATE_MENU:
            calibrateIndex = (calibrateIndex > 0) ? calibrateIndex - 1 : 1;
            displayMenu();
            break;

        case CAL_DRY_MENU:
            calDryIndex = (calDryIndex > 0) ? calDryIndex - 1 : 1;
            displayMenu();
            break;

        case CAL_WET_MENU:
            calWetIndex = (calWetIndex > 0) ? calWetIndex - 1 : 1;
            displayMenu();
            break;

        case CAL_DRY_EDIT:
            // Round to nearest 50, then add 50
            config.sensorDry = ((config.sensorDry + 25) / 50) * 50 + 50;
            config.sensorDry = min(16383, config.sensorDry);
            displayMenu();
            break;

        case CAL_WET_EDIT:
            // Round to nearest 50, then add 50
            config.sensorWet = ((config.sensorWet + 25) / 50) * 50 + 50;
            config.sensorWet = min(16383, config.sensorWet);
            displayMenu();
            break;

        case SETTING_LOG_INTERVAL:
            config.logInterval = min(60, config.logInterval + 1);
            displayMenu();
            break;

        case SETTING_BACKLIGHT_TIMEOUT:
            config.backlightTimeout = min(60, config.backlightTimeout + 1);
            displayMenu();
            break;

        case SETTING_SENSOR_WARMUP:
            config.sensorWarmup = min(2000, config.sensorWarmup + 100);
            displayMenu();
            break;

        case SETTING_NUM_MEASUREMENTS:
            config.numMeasurements = min(50, config.numMeasurements + 1);
            displayMenu();
            break;

        case SETTING_MEASUREMENT_DELAY:
            config.measurementDelay = min(500, config.measurementDelay + 10);
            displayMenu();
            break;
    }
}

void handleDownButton() {
    switch (currentState) {
        case MAIN_MENU:
            menuIndex = (menuIndex < 5) ? menuIndex + 1 : 0;
            displayMenu();
            break;

        case SETTINGS_MENU:
            settingsIndex = (settingsIndex < 4) ? settingsIndex + 1 : 0;
            displayMenu();
            break;

        case CALIBRATE_MENU:
            calibrateIndex = (calibrateIndex < 1) ? calibrateIndex + 1 : 0;
            displayMenu();
            break;

        case CAL_DRY_MENU:
            calDryIndex = (calDryIndex < 1) ? calDryIndex + 1 : 0;
            displayMenu();
            break;

        case CAL_WET_MENU:
            calWetIndex = (calWetIndex < 1) ? calWetIndex + 1 : 0;
            displayMenu();
            break;

        case CAL_DRY_EDIT:
            // Round to nearest 50, then subtract 50
            config.sensorDry = ((config.sensorDry + 25) / 50) * 50 - 50;
            config.sensorDry = max(0, config.sensorDry);
            displayMenu();
            break;

        case CAL_WET_EDIT:
            // Round to nearest 50, then subtract 50
            config.sensorWet = ((config.sensorWet + 25) / 50) * 50 - 50;
            config.sensorWet = max(0, config.sensorWet);
            displayMenu();
            break;

        case SETTING_LOG_INTERVAL:
            config.logInterval = max(1, config.logInterval - 1);
            displayMenu();
            break;

        case SETTING_BACKLIGHT_TIMEOUT:
            config.backlightTimeout = max(1, config.backlightTimeout - 1);
            displayMenu();
            break;

        case SETTING_SENSOR_WARMUP:
            config.sensorWarmup = max(100, config.sensorWarmup - 100);
            displayMenu();
            break;

        case SETTING_NUM_MEASUREMENTS:
            config.numMeasurements = max(5, config.numMeasurements - 1);
            displayMenu();
            break;

        case SETTING_MEASUREMENT_DELAY:
            config.measurementDelay = max(10, config.measurementDelay - 10);
            displayMenu();
            break;
    }
}

void handleSelectButton() {
    switch (currentState) {
        case STATUS_SCREEN:
            // First SELECT press: show main menu
            currentState = MAIN_MENU;
            menuIndex = 0;
            displayMenu();
            break;

        case MAIN_MENU:
            // Second SELECT press: enter the selected submenu
            switch (menuIndex) {
                case 0: // Settings
                    currentState = SETTINGS_MENU;
                    settingsIndex = 0;
                    displayMenu();
                    break;
                case 1: // Calibrate
                    currentState = CALIBRATE_MENU;
                    calibrateIndex = 0;
                    displayMenu();
                    break;
                case 2: // Download Data
                    currentState = DOWNLOAD_CONFIRM;
                    displayMenu();
                    break;
                case 3: // Clear Data
                    currentState = CLEAR_DATA_CONFIRM;
                    displayMenu();
                    break;
                case 4: // System Info
                    currentState = SYSTEM_INFO;
                    displayMenu();
                    break;
                case 5: // Reset to Defaults
                    currentState = RESET_CONFIRM;
                    displayMenu();
                    break;
            }
            break;

        case SETTINGS_MENU:
            switch (settingsIndex) {
                case 0: currentState = SETTING_LOG_INTERVAL; break;
                case 1: currentState = SETTING_BACKLIGHT_TIMEOUT; break;
                case 2: currentState = SETTING_SENSOR_WARMUP; break;
                case 3: currentState = SETTING_NUM_MEASUREMENTS; break;
                case 4: currentState = SETTING_MEASUREMENT_DELAY; break;
            }
            displayMenu();
            break;

        case SETTING_LOG_INTERVAL:
        case SETTING_BACKLIGHT_TIMEOUT:
        case SETTING_SENSOR_WARMUP:
        case SETTING_NUM_MEASUREMENTS:
        case SETTING_MEASUREMENT_DELAY:
            saveConfig();
            currentState = SETTINGS_MENU;
            displayMenu();
            break;

        case CALIBRATE_MENU:
            if (calibrateIndex == 0) {
                // Selected "Dry Value"
                currentState = CAL_DRY_MENU;
                calDryIndex = 0;
                displayMenu();
            } else if (calibrateIndex == 1) {
                // Selected "Wet Value"
                currentState = CAL_WET_MENU;
                calWetIndex = 0;
                displayMenu();
            }
            break;

        case CAL_DRY_MENU:
            if (calDryIndex == 0) {
                // Selected "Measure Now" for dry
                currentState = CAL_DRY_MEASURE;
                currentRawADC = readMoistureRaw();  // Initial reading for display
                displayMenu();
            } else if (calDryIndex == 1) {
                // Selected "Edit Manually" for dry - round to nearest 50
                config.sensorDry = ((config.sensorDry + 25) / 50) * 50;
                currentState = CAL_DRY_EDIT;
                displayMenu();
            }
            break;

        case CAL_WET_MENU:
            if (calWetIndex == 0) {
                // Selected "Measure Now" for wet
                currentState = CAL_WET_MEASURE;
                currentRawADC = readMoistureRaw();  // Initial reading for display
                displayMenu();
            } else if (calWetIndex == 1) {
                // Selected "Edit Manually" for wet - round to nearest 50
                config.sensorWet = ((config.sensorWet + 25) / 50) * 50;
                currentState = CAL_WET_EDIT;
                displayMenu();
            }
            break;

        case CAL_DRY_MEASURE:
            // User pressed SELECT to capture the dry value
            config.sensorDry = currentRawADC;  // Use the live value being displayed
            saveConfig();
            currentState = CAL_MEASURE_DONE;
            displayMenu();
            break;

        case CAL_WET_MEASURE:
            // User pressed SELECT to capture the wet value
            config.sensorWet = currentRawADC;  // Use the live value being displayed
            saveConfig();
            currentState = CAL_MEASURE_DONE;
            displayMenu();
            break;

        case CAL_DRY_EDIT:
            saveConfig();
            currentState = CAL_MEASURE_DONE;
            displayMenu();
            break;

        case CAL_WET_EDIT:
            saveConfig();
            currentState = CAL_MEASURE_DONE;
            displayMenu();
            break;

        case CAL_MEASURE_DONE:
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;

        case DOWNLOAD_CONFIRM:
            currentState = DOWNLOAD_PROGRESS;
            displayMenu();
            downloadDataToSerial();
            currentState = DOWNLOAD_COMPLETE;
            displayMenu();
            break;

        case DOWNLOAD_COMPLETE:
            currentState = MAIN_MENU;
            displayMenu();
            break;

        case CLEAR_DATA_CONFIRM:
            clearAllData();
            currentState = MAIN_MENU;
            displayMenu();
            break;

        case RESET_CONFIRM:
            resetToDefaults();
            currentState = MAIN_MENU;
            displayMenu();
            break;
    }
}

void handleBackButton() {
    switch (currentState) {
        case MAIN_MENU:
        case SETTINGS_MENU:
        case CALIBRATE_MENU:
        case DOWNLOAD_CONFIRM:
        case CLEAR_DATA_CONFIRM:
        case SYSTEM_INFO:
        case RESET_CONFIRM:
        case DOWNLOAD_COMPLETE:
            currentState = STATUS_SCREEN;
            menuIndex = 0;
            displayMenu();
            break;

        case SETTING_LOG_INTERVAL:
        case SETTING_BACKLIGHT_TIMEOUT:
        case SETTING_SENSOR_WARMUP:
        case SETTING_NUM_MEASUREMENTS:
        case SETTING_MEASUREMENT_DELAY:
            currentState = SETTINGS_MENU;
            displayMenu();
            break;

        case CAL_DRY_MENU:
        case CAL_WET_MENU:
            currentState = CALIBRATE_MENU;
            displayMenu();
            break;

        case CAL_DRY_MEASURE:
        case CAL_DRY_EDIT:
            currentState = CAL_DRY_MENU;
            displayMenu();
            break;

        case CAL_WET_MEASURE:
        case CAL_WET_EDIT:
            currentState = CAL_WET_MENU;
            displayMenu();
            break;

        case CAL_MEASURE_DONE:
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
        case STATUS_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print(F("M:"));
            lcd.print(currentMoisture);
            lcd.print(F("% ADC:"));
            lcd.print(currentRawADC);
            lcd.setCursor(0, 1);
            lcd.print(F("Log:"));
            lcd.print(getEntryCount());
            lcd.print(F("/"));
            lcd.print(MAX_ENTRIES);

            // Warning if buffer is >90% full
            if (getEntryCount() > (MAX_ENTRIES * 0.9)) {
                lcd.setCursor(14, 1);
                lcd.print(F("!"));
            }
            break;

        case MAIN_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("MAIN MENU"));
            lcd.setCursor(0, 1);
            switch (menuIndex) {
                case 0: lcd.print(F(">Settings")); break;
                case 1: lcd.print(F(">Calibrate")); break;
                case 2: lcd.print(F(">Download")); break;
                case 3: lcd.print(F(">Clear Data")); break;
                case 4: lcd.print(F(">System Info")); break;
                case 5: lcd.print(F(">Reset")); break;
            }
            break;

        case SETTINGS_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("SETTINGS ("));
            lcd.print(settingsIndex + 1);
            lcd.print(F("/5)"));
            lcd.setCursor(0, 1);
            switch (settingsIndex) {
                case 0: lcd.print(F(">Log Interval")); break;
                case 1: lcd.print(F(">LCD Sleep")); break;
                case 2: lcd.print(F(">Sensor Warmup")); break;
                case 3: lcd.print(F(">Num Samples")); break;
                case 4: lcd.print(F(">Sample Delay")); break;
            }
            break;

        case SETTING_LOG_INTERVAL:
            lcd.setCursor(0, 0);
            lcd.print(F("Log Interval"));
            lcd.setCursor(0, 1);
            lcd.print(config.logInterval);
            lcd.print(F("min UP/DN SEL"));
            break;

        case SETTING_BACKLIGHT_TIMEOUT:
            lcd.setCursor(0, 0);
            lcd.print(F("LCD Sleep"));
            lcd.setCursor(0, 1);
            lcd.print(config.backlightTimeout);
            lcd.print(F("min UP/DN SEL"));
            break;

        case SETTING_SENSOR_WARMUP:
            lcd.setCursor(0, 0);
            lcd.print(F("Sensor Warmup"));
            lcd.setCursor(0, 1);
            lcd.print(config.sensorWarmup);
            lcd.print(F("ms UP/DN SEL"));
            break;

        case SETTING_NUM_MEASUREMENTS:
            lcd.setCursor(0, 0);
            lcd.print(F("Num Samples"));
            lcd.setCursor(0, 1);
            lcd.print(config.numMeasurements);
            lcd.print(F(" UP/DN SEL"));
            break;

        case SETTING_MEASUREMENT_DELAY:
            lcd.setCursor(0, 0);
            lcd.print(F("Sample Delay"));
            lcd.setCursor(0, 1);
            lcd.print(config.measurementDelay);
            lcd.print(F("ms UP/DN SEL"));
            break;

        case CALIBRATE_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("CALIBRATE"));
            lcd.setCursor(0, 1);
            if (calibrateIndex == 0) {
                lcd.print(F(">Dry Value"));
            } else {
                lcd.print(F(">Wet Value"));
            }
            break;

        case CAL_DRY_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("DRY CALIBRATION"));
            lcd.setCursor(0, 1);
            if (calDryIndex == 0) {
                lcd.print(F(">Measure Now"));
            } else {
                lcd.print(F(">Edit Manually"));
            }
            break;

        case CAL_WET_MENU:
            lcd.setCursor(0, 0);
            lcd.print(F("WET CALIBRATION"));
            lcd.setCursor(0, 1);
            if (calWetIndex == 0) {
                lcd.print(F(">Measure Now"));
            } else {
                lcd.print(F(">Edit Manually"));
            }
            break;

        case CAL_DRY_MEASURE:
            lcd.setCursor(0, 0);
            lcd.print(F("Hold in AIR"));
            lcd.setCursor(0, 1);
            lcd.print(F("ADC:"));
            lcd.print(currentRawADC);
            lcd.print(F(" SEL=OK"));
            break;

        case CAL_WET_MEASURE:
            lcd.setCursor(0, 0);
            lcd.print(F("Wet soil/water"));
            lcd.setCursor(0, 1);
            lcd.print(F("ADC:"));
            lcd.print(currentRawADC);
            lcd.print(F(" SEL=OK"));
            break;

        case CAL_DRY_EDIT:
            lcd.setCursor(0, 0);
            lcd.print(F("Dry (air) ADC"));
            lcd.setCursor(0, 1);
            lcd.print(config.sensorDry);
            lcd.print(F(" UP/DN SEL"));
            break;

        case CAL_WET_EDIT:
            lcd.setCursor(0, 0);
            lcd.print(F("Wet (water) ADC"));
            lcd.setCursor(0, 1);
            lcd.print(config.sensorWet);
            lcd.print(F(" UP/DN SEL"));
            break;

        case CAL_MEASURE_DONE:
            lcd.setCursor(0, 0);
            lcd.print(F("Saved! D:"));
            lcd.print(config.sensorDry);
            lcd.setCursor(0, 1);
            lcd.print(F("W:"));
            lcd.print(config.sensorWet);
            lcd.print(F(" SEL=OK"));
            break;

        case DOWNLOAD_CONFIRM:
            lcd.setCursor(0, 0);
            lcd.print(F("Download CSV?"));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Yes BCK=No"));
            break;

        case DOWNLOAD_PROGRESS:
            lcd.setCursor(0, 0);
            lcd.print(F("Downloading..."));
            // Progress bar updated by downloadDataToSerial()
            break;

        case DOWNLOAD_COMPLETE:
            // Displayed by downloadDataToSerial()
            break;

        case CLEAR_DATA_CONFIRM:
            lcd.setCursor(0, 0);
            lcd.print(F("Clear all data?"));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Yes BCK=No"));
            break;

        case SYSTEM_INFO:
            lcd.setCursor(0, 0);
            lcd.print(F("Cal D:"));
            lcd.print(config.sensorDry);
            lcd.setCursor(0, 1);
            lcd.print(F("W:"));
            lcd.print(config.sensorWet);
            lcd.print(F(" Int:"));
            lcd.print(config.logInterval);
            lcd.print(F("m"));
            break;

        case RESET_CONFIRM:
            lcd.setCursor(0, 0);
            lcd.print(F("Reset config?"));
            lcd.setCursor(0, 1);
            lcd.print(F("SEL=Yes BCK=No"));
            break;
    }
}
