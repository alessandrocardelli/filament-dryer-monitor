#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "AppConfig.h"
#include "AppTypes.h"
#include "HeaterControlPolicy.h"

// Integrates the real GPIO19 output with the host-tested control policy.
// HEATER OUTPUT REMAINS OFF IN THE SHIPPED CONFIGURATION.
// Sensor/thermal qualification and independent physical safeguards are
// separate from the optional added TCO (D029).
class SafetyController {
 public:
  void begin() {
    // GPIO is driven LOW before any other peripheral is initialized.
    pinMode(AppConfig::kPinHeaterPwm, OUTPUT);
    forceHeaterOff();
  }

  // Synchronous immediate shutdown for STOP / COMPLETE / FAULT / BOOT.
  void stop() {
    forceHeaterOff();
    HeaterControl::Inputs stopped{};
    policy_.evaluate(0, HeaterControl::Limits{}, stopped);
  }

  void update(uint32_t nowMs, const NtcSnapshot &ntc,
              const SensorSnapshot &chamber, bool cycleActive,
              bool fanPwmRunning, uint32_t elapsedCycleMs,
              float targetChamberC) {
    // Carry over the previously validated electrical NTC fault latch even
    // when heating is disabled.
    if (ntc.sampled && ntc.fault != NtcFault::None) {
      faultActive_ = true;
      ntcFault_ = ntc.fault;
    }

    if (!AppConfig::kHeaterOutputEnabled || !AppConfig::kHeaterProfileApproved ||
        !AppConfig::kHeaterNtcModelApproved || faultActive_) {
      forceHeaterOff();
      return;
    }

    HeaterControl::Inputs in{};
    in.cycleRequested = cycleActive;
    // Fan PWM command is only a proxy. The board has no tachometer.
    in.fanCommandedRunning = fanPwmRunning;
    in.ntcElectricalValid = ntc.sampled && ntc.valid &&
                            ntc.fault == NtcFault::None;
    in.ntcThermallyCalibrated = AppConfig::kHeaterNtcModelApproved &&
                                in.ntcElectricalValid;
    in.heaterTemperatureC = ntc.temperatureC;
    in.ntcAgeMs = ntc.sampled ? static_cast<uint32_t>(nowMs - ntc.lastSampleMs)
                              : UINT32_MAX;
    in.chamberValid = chamber.valid;
    in.chamberTemperatureC = chamber.temperatureC;
    in.chamberAgeMs = chamber.valid
                        ? static_cast<uint32_t>(nowMs - chamber.lastSuccessMs)
                        : UINT32_MAX;
    in.targetChamberC = targetChamberC;
    in.elapsedCycleMs = elapsedCycleMs;

    HeaterControl::Limits lim{};
    lim.approved = AppConfig::kHeaterProfileApproved;
    lim.maximumHeaterC = AppConfig::kHeaterMaxMeasuredC;
    lim.maximumChamberC = AppConfig::kChamberMaxMeasuredC;
    lim.hysteresisC = AppConfig::kChamberControlHysteresisC;
    lim.maximumNtcAgeMs = AppConfig::kHeaterNtcMaxAgeMs;
    lim.maximumChamberAgeMs = AppConfig::kHeaterChamberMaxAgeMs;
    lim.minimumFanPrerunMs = AppConfig::kHeaterFanPrerunMs;
    lim.maximumContinuousHeatMs = AppConfig::kHeaterMaxContinuousOnMs;
    lim.maximumCycleMs = AppConfig::kHeaterMaxCycleMs;

    const bool requested = policy_.evaluate(nowMs, lim, in);
    if (policy_.fault() != HeaterControl::Fault::None) {
      faultActive_ = true;
      forceHeaterOff();
      return;
    }

    // Compile-time output gate + runtime policy BOTH required.
    setHeater(requested && cycleActive);
  }

  bool heaterPermitted() const { return !heaterLocked() && !faultActive_; }
  bool heaterLocked() const {
    return !AppConfig::kHeaterOutputEnabled ||
           !AppConfig::kHeaterProfileApproved ||
           !AppConfig::kHeaterNtcModelApproved;
  }
  bool heaterOn() const { return heaterOn_; }
  HeaterControl::Fault heaterFault() const { return policy_.fault(); }
  bool fanPermitted() const { return !faultActive_; }
  bool faultActive() const { return faultActive_; }
  NtcFault ntcFault() const { return ntcFault_; }

 private:
  HeaterControl::Policy policy_{};
  bool faultActive_ = false;
  bool heaterOn_ = false;
  NtcFault ntcFault_ = NtcFault::None;

  void forceHeaterOff() {
    digitalWrite(AppConfig::kPinHeaterPwm, LOW);
    heaterOn_ = false;
  }

  void setHeater(bool requested) {
    const bool desired = AppConfig::kHeaterOutputEnabled && requested;
    // No PWM here: hysteretic chamber thermostat, not analog PID.
    if (heaterOn_ != desired) {
      digitalWrite(AppConfig::kPinHeaterPwm, desired ? HIGH : LOW);
      heaterOn_ = desired;
    }
  }
};
