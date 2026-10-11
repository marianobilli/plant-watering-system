#include "PlantLogic.h"

int moisturePercent(int raw, int dry, int wet) {
    if (dry == wet) return 0;
    long pct = (long)(raw - dry) * 100 / (wet - dry);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return (int)pct;
}

bool sensorReadingValid(int raw) {
    // 12-bit ADC: ~0 or ~4095 means the sensor line is shorted or floating.
    return raw > 50 && raw < 4045;
}

uint32_t pumpDurationMs(uint16_t ml, float mlPerSec) {
    if (mlPerSec <= 0.0f) return 0;
    return (uint32_t)((float)ml / mlPerSec * 1000.0f);
}

bool PlantController::wantsReading(uint32_t nowMs) const {
    if (!cfg.enabled) return false;
    switch (phase_) {
    case Phase::Dry:
        if (!haveChecked_) return true;
        return (uint32_t)(nowMs - lastCheckMs_) >= (uint32_t)cfg.checkHours * 3600000UL;
    case Phase::Soaking:
        return (uint32_t)(nowMs - soakStartMs_) >= (uint32_t)cfg.soakMin * 60000UL;
    default:
        return false;
    }
}

void PlantController::enterFault(FaultCode f) {
    phase_ = Phase::Fault;
    fault_ = f;
}

void PlantController::onReading(uint32_t nowMs, int raw) {
    const bool act = wantsReading(nowMs);

    if (!sensorReadingValid(raw)) {
        if (act) enterFault(FaultCode::SensorFault);
        return;
    }
    const int pct = moisturePercent(raw, cfg.sensorDry, cfg.sensorWet);
    moisture_ = pct;
    if (!act) return;

    if (phase_ == Phase::Dry) {
        haveChecked_ = true;
        lastCheckMs_ = nowMs;
        if (pct <= cfg.minPct) startWatering();
        return;
    }

    // Soaking: decide whether the cycle is done
    if (pct >= cfg.targetPct) {
        phase_ = Phase::Dry;
        haveChecked_ = true;
        lastCheckMs_ = nowMs;
    } else if (pct <= prevMoisture_) {
        enterFault(FaultCode::NoMoistureRise);
    } else if (doses_ >= PLANT_MAX_DOSES) {
        enterFault(FaultCode::MaxDoses);
    } else {
        prevMoisture_ = pct;
        phase_ = Phase::PumpRequested;
    }
}

void PlantController::startWatering() {
    if (phase_ != Phase::Dry) return;
    prevMoisture_ = moisture_ < 0 ? 0 : moisture_;
    doses_ = 0;
    phase_ = Phase::PumpRequested;
}

void PlantController::onPumpStarted() {
    if (phase_ != Phase::PumpRequested) return;
    phase_ = Phase::Pumping;
    doses_++;
}

void PlantController::onPumpFinished(uint32_t nowMs) {
    if (phase_ != Phase::Pumping) return;
    phase_ = Phase::Soaking;
    soakStartMs_ = nowMs;
}

void PlantController::onPumpAborted() {
    if (phase_ != Phase::Pumping) return;
    phase_ = Phase::PumpRequested;
    if (doses_ > 0) doses_--;
}

void PlantController::clearFault(uint32_t nowMs) {
    if (phase_ != Phase::Fault) return;
    phase_ = Phase::Dry;
    fault_ = FaultCode::None;
    haveChecked_ = false; // re-check right away
    lastCheckMs_ = nowMs;
}

uint32_t PlantController::checkRemainingMs(uint32_t nowMs) const {
    if (phase_ != Phase::Dry || !haveChecked_) return 0;
    const uint32_t interval = (uint32_t)cfg.checkHours * 3600000UL;
    const uint32_t elapsed = nowMs - lastCheckMs_;
    return elapsed >= interval ? 0 : interval - elapsed;
}

uint32_t PlantController::soakRemainingMs(uint32_t nowMs) const {
    if (phase_ != Phase::Soaking) return 0;
    const uint32_t soak = (uint32_t)cfg.soakMin * 60000UL;
    const uint32_t elapsed = nowMs - soakStartMs_;
    return elapsed >= soak ? 0 : soak - elapsed;
}
