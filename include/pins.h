#pragma once
#include <stdint.h>

// Pin map for the 6-plant board (hardware/kicad/plant_watering.kicad_sch).
// Sensors must stay on ADC1 (IO1-IO10): ADC2 is unusable while WiFi is active.

constexpr uint8_t NUM_PLANTS = 6;

constexpr uint8_t SENSOR_PINS[NUM_PLANTS] = {1, 2, 7, 8, 9, 10};      // S1..S6 (ADC1)
constexpr uint8_t PUMP_PINS[NUM_PLANTS]   = {15, 17, 39, 12, 16, 18}; // PUMP1..PUMP6 (MOSFET gates)

constexpr uint8_t PIN_SENSOR_DIS = 11; // shared sensor DIS line: LOW = sensors on, HIGH = asleep
constexpr uint8_t PIN_FLOAT_SW   = 47; // optional float switch, INPUT_PULLUP: LOW = reservoir low

// UI module (SH1106 OLED + rotary encoder + CON/BAK buttons), powered from 3V3
constexpr uint8_t PIN_SDA = 14;
constexpr uint8_t PIN_SCL = 13;
constexpr uint8_t PIN_CON = 21; // confirm button
constexpr uint8_t PIN_PSH = 6;  // encoder push
constexpr uint8_t PIN_TRA = 5;  // encoder A
constexpr uint8_t PIN_TRB = 4;  // encoder B
constexpr uint8_t PIN_BAK = 38; // back button
