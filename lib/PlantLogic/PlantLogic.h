#pragma once
#include <stdint.h>

// Hardware-free watering logic for one plant, so it can be unit-tested on the host.
//
// Two-phase cycle (see docs/WATERING_LOGIC.md):
//   Dry   - check moisture every checkHours; start watering only at <= minPct.
//   Water - pump waterMl, soak soakMin minutes, re-measure; repeat until >= targetPct.
// Failsafe: if moisture does not rise after a dose, stop and latch a fault.

constexpr uint8_t PLANT_MAX_DOSES = 10; // max pump doses per watering cycle

struct PlantConfig {
    bool     enabled      = true;
    uint8_t  targetPct    = 80;   // stop watering at/above this
    uint8_t  minPct       = 10;   // start watering at/below this
    uint16_t waterMl      = 50;   // per dose
    uint8_t  soakMin      = 5;    // wait after each dose
    uint8_t  checkHours   = 6;    // dry-cycle check interval
    uint16_t sensorDry    = 3100; // 12-bit ADC in air
    uint16_t sensorWet    = 1500; // 12-bit ADC in water
    float    pumpMlPerSec = 1.67f;
};

enum class Phase : uint8_t { Dry, PumpRequested, Pumping, Soaking, Fault };
enum class FaultCode : uint8_t { None, NoMoistureRise, MaxDoses, SensorFault };

int      moisturePercent(int raw, int dry, int wet); // clamped 0..100
bool     sensorReadingValid(int raw);                // false when shorted/disconnected
uint32_t pumpDurationMs(uint16_t ml, float mlPerSec);

class PlantController {
public:
    PlantConfig cfg;

    Phase     phase() const { return phase_; }
    FaultCode fault() const { return fault_; }
    int       moisture() const { return moisture_; } // -1 until first reading

    // True when the scheduler should power the sensors and take a reading.
    bool wantsReading(uint32_t nowMs) const;
    // Feed a raw ADC reading. Always updates moisture(); only acts on it if wantsReading().
    void onReading(uint32_t nowMs, int raw);

    bool wantsPump() const { return phase_ == Phase::PumpRequested; }
    void onPumpStarted();
    void onPumpFinished(uint32_t nowMs);
    void onPumpAborted(); // pump stopped early (e.g. reservoir empty); dose is not counted

    void startWatering(); // manual "water now"
    void clearFault(uint32_t nowMs);

    uint32_t checkRemainingMs(uint32_t nowMs) const; // Dry phase only
    uint32_t soakRemainingMs(uint32_t nowMs) const;  // Soaking phase only

private:
    Phase     phase_ = Phase::Dry;
    FaultCode fault_ = FaultCode::None;
    int       moisture_ = -1;
    int       prevMoisture_ = 0;
    uint8_t   doses_ = 0;
    bool      haveChecked_ = false;
    uint32_t  lastCheckMs_ = 0;
    uint32_t  soakStartMs_ = 0;

    void enterFault(FaultCode f);
};
