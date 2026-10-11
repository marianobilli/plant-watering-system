#include <unity.h>
#include "PlantLogic.h"

// Defaults: dry=3100, wet=1500, min=10%, target=80%, soak=5 min, check=6 h.
// raw(pct) gives the ADC value that maps to a given moisture percentage.
static int raw(int pct) { return 3100 - pct * 16; }

static const uint32_t MIN = 60000UL;
static const uint32_t HOUR = 3600000UL;

static PlantController plant;

void setUp(void) { plant = PlantController(); }
void tearDown(void) {}

void test_moisture_percent_maps_and_clamps(void) {
    TEST_ASSERT_EQUAL(0, moisturePercent(3100, 3100, 1500));
    TEST_ASSERT_EQUAL(100, moisturePercent(1500, 3100, 1500));
    TEST_ASSERT_EQUAL(50, moisturePercent(2300, 3100, 1500));
    TEST_ASSERT_EQUAL(0, moisturePercent(3500, 3100, 1500));
    TEST_ASSERT_EQUAL(100, moisturePercent(1000, 3100, 1500));
}

void test_pump_duration(void) {
    TEST_ASSERT_EQUAL_UINT32(25000, pumpDurationMs(50, 2.0f));
    TEST_ASSERT_EQUAL_UINT32(0, pumpDurationMs(50, 0.0f));
}

void test_first_reading_is_immediate(void) {
    TEST_ASSERT_TRUE(plant.wantsReading(0));
}

void test_wet_soil_stays_in_dry_cycle(void) {
    plant.onReading(0, raw(50));
    TEST_ASSERT_EQUAL(50, plant.moisture());
    TEST_ASSERT_EQUAL((int)Phase::Dry, (int)plant.phase());
    TEST_ASSERT_FALSE(plant.wantsReading(5 * HOUR));
    TEST_ASSERT_TRUE(plant.wantsReading(6 * HOUR));
}

void test_dry_soil_requests_pump(void) {
    plant.onReading(0, raw(8));
    TEST_ASSERT_TRUE(plant.wantsPump());
}

void test_does_not_water_above_min_threshold(void) {
    plant.onReading(0, raw(11));
    TEST_ASSERT_FALSE(plant.wantsPump());
}

void test_full_cycle_waters_until_target(void) {
    plant.onReading(0, raw(8));
    TEST_ASSERT_TRUE(plant.wantsPump());

    plant.onPumpStarted();
    plant.onPumpFinished(30000);
    TEST_ASSERT_EQUAL((int)Phase::Soaking, (int)plant.phase());
    TEST_ASSERT_FALSE(plant.wantsReading(30000 + 4 * MIN));
    TEST_ASSERT_TRUE(plant.wantsReading(30000 + 5 * MIN));

    plant.onReading(30000 + 5 * MIN, raw(45)); // still below target: dose again
    TEST_ASSERT_TRUE(plant.wantsPump());

    plant.onPumpStarted();
    plant.onPumpFinished(10 * MIN);
    plant.onReading(15 * MIN, raw(82)); // target reached
    TEST_ASSERT_EQUAL((int)Phase::Dry, (int)plant.phase());
    TEST_ASSERT_FALSE(plant.wantsPump());
}

void test_no_moisture_rise_latches_fault(void) {
    plant.onReading(0, raw(8));
    plant.onPumpStarted();
    plant.onPumpFinished(30000);
    plant.onReading(30000 + 5 * MIN, raw(8)); // no change after a dose
    TEST_ASSERT_EQUAL((int)Phase::Fault, (int)plant.phase());
    TEST_ASSERT_EQUAL((int)FaultCode::NoMoistureRise, (int)plant.fault());
    TEST_ASSERT_FALSE(plant.wantsPump());
    TEST_ASSERT_FALSE(plant.wantsReading(100 * HOUR));
}

void test_clear_fault_rechecks_immediately(void) {
    plant.onReading(0, raw(8));
    plant.onPumpStarted();
    plant.onPumpFinished(0);
    plant.onReading(5 * MIN, raw(8));
    plant.clearFault(6 * MIN);
    TEST_ASSERT_EQUAL((int)Phase::Dry, (int)plant.phase());
    TEST_ASSERT_TRUE(plant.wantsReading(6 * MIN));
}

void test_max_doses_latches_fault(void) {
    uint32_t t = 0;
    int pct = 5;
    plant.onReading(t, raw(pct));
    for (int dose = 0; dose < PLANT_MAX_DOSES; dose++) {
        TEST_ASSERT_TRUE(plant.wantsPump());
        plant.onPumpStarted();
        plant.onPumpFinished(t);
        t += 5 * MIN;
        pct += 1; // rises, but never reaches target
        plant.onReading(t, raw(pct));
    }
    TEST_ASSERT_EQUAL((int)Phase::Fault, (int)plant.phase());
    TEST_ASSERT_EQUAL((int)FaultCode::MaxDoses, (int)plant.fault());
}

void test_aborted_pump_is_retried_without_counting(void) {
    plant.onReading(0, raw(8));
    plant.onPumpStarted();
    plant.onPumpAborted();
    TEST_ASSERT_TRUE(plant.wantsPump());
}

void test_disconnected_sensor_faults_instead_of_watering(void) {
    plant.onReading(0, 4095);
    TEST_ASSERT_EQUAL((int)Phase::Fault, (int)plant.phase());
    TEST_ASSERT_EQUAL((int)FaultCode::SensorFault, (int)plant.fault());
    TEST_ASSERT_FALSE(plant.wantsPump());
}

void test_disabled_plant_never_reads(void) {
    plant.cfg.enabled = false;
    TEST_ASSERT_FALSE(plant.wantsReading(0));
    TEST_ASSERT_FALSE(plant.wantsReading(100 * HOUR));
}

void test_timers_survive_millis_wraparound(void) {
    const uint32_t start = 0xFFFFFFFFUL - HOUR;
    plant.onReading(start, raw(50));
    TEST_ASSERT_FALSE(plant.wantsReading(start + 5 * HOUR)); // wraps past 0
    TEST_ASSERT_TRUE(plant.wantsReading(start + 6 * HOUR));
}

static void runAll() {
    UNITY_BEGIN();
    RUN_TEST(test_moisture_percent_maps_and_clamps);
    RUN_TEST(test_pump_duration);
    RUN_TEST(test_first_reading_is_immediate);
    RUN_TEST(test_wet_soil_stays_in_dry_cycle);
    RUN_TEST(test_dry_soil_requests_pump);
    RUN_TEST(test_does_not_water_above_min_threshold);
    RUN_TEST(test_full_cycle_waters_until_target);
    RUN_TEST(test_no_moisture_rise_latches_fault);
    RUN_TEST(test_clear_fault_rechecks_immediately);
    RUN_TEST(test_max_doses_latches_fault);
    RUN_TEST(test_aborted_pump_is_retried_without_counting);
    RUN_TEST(test_disconnected_sensor_faults_instead_of_watering);
    RUN_TEST(test_disabled_plant_never_reads);
    RUN_TEST(test_timers_survive_millis_wraparound);
    UNITY_END();
}

#ifndef NATIVE
#include <Arduino.h>
void setup() { delay(2000); runAll(); }
void loop() {}
#else
int main(void) { runAll(); return 0; }
#endif
