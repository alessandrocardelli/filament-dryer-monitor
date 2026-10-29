#pragma once
#include <cmath>
#include <cstdint>

// V1 AIR thermostat design, intentionally independent of STEP07-13 trial
// thresholds. Does NOT own GPIO or approve operation. All local temperature
// limits must be independently characterized and explicitly qualified before
// the hardware SafetyController may call this with a live heater.
// No accumulated HEATER ON budget; the user cycle is the time authority.
namespace HeaterProductAir {

enum class Fault : uint8_t {
  None, Unqualified, NtcInvalid, NtcStale, AirInvalid, AirStale,
  HeaterTooHot, AirTooHot, FanLost
};

struct Limits {
  bool qualified = false;
  float ntcSoftC = NAN;   // local heater temp: pause, NOT the chamber setpoint
  float ntcRearmC = NAN;  // local heater temp: resume
  float ntcHardC = NAN;   // latched fault; requires qualified local NTC model
  float airFaultC = NAN;  // independent SHT45 chamber fault limit
  float airHysteresisC = NAN;
  uint32_t fanPrerunMs = 0;
  uint32_t maxOnWindowMs = 0;
  uint32_t minOffMs = 0;
  uint32_t maxNtcAgeMs = 0;
  uint32_t maxAirAgeMs = 0;
};

struct Inputs {
  bool cycleActive = false;
  bool fanCommandedRunning = false; // no tachometer; never claim actual RPM
  bool ntcElectricalValid = false;
  bool ntcThermallyQualified = false;
  float heaterTempC = NAN;
  uint32_t ntcAgeMs = UINT32_MAX;
  bool airValid = false;
  float airC = NAN;
  uint32_t airAgeMs = UINT32_MAX;
  float targetAirC = NAN;           // user-selected 40..55C, not hardcoded
};

class Controller {
 public:
  explicit Controller(Limits limits = Limits{}) : limits_(limits) {}
  bool update(uint32_t nowMs, const Inputs& in) {
    requestedOn_ = false;
    if (fault_ != Fault::None) return false;
    if (!in.cycleActive) {
      phase_ = Phase::Idle;
      airDemand_ = false;
      localReady_ = true;
      wasHeating_ = false;
      return false; // normal cycle timer/STOP, no cumulative time threshold
    }
    if (!validLimits(limits_, in.targetAirC))
      return abort(Fault::Unqualified);
    if (!in.ntcElectricalValid || !in.ntcThermallyQualified ||
        !std::isfinite(in.heaterTempC)) return abort(Fault::NtcInvalid);
    if (in.ntcAgeMs > limits_.maxNtcAgeMs) return abort(Fault::NtcStale);
    if (!in.airValid || !std::isfinite(in.airC))
      return abort(Fault::AirInvalid);
    if (in.airAgeMs > limits_.maxAirAgeMs) return abort(Fault::AirStale);
    if (in.heaterTempC >= limits_.ntcHardC)
      return abort(Fault::HeaterTooHot);
    if (in.airC >= limits_.airFaultC)
      return abort(Fault::AirTooHot);

    if (!in.fanCommandedRunning) {
      if (wasHeating_) return abort(Fault::FanLost);
      phase_ = Phase::Idle;
      return false;
    }
    if (phase_ == Phase::Idle) {
      phase_ = Phase::Prerun;
      fanStartedMs_ = nowMs;
      airDemand_ = in.airC < in.targetAirC;
      localReady_ = in.heaterTempC < limits_.ntcSoftC;
      return false;
    }
    if (phase_ == Phase::Prerun) {
      if (nowMs - fanStartedMs_ < limits_.fanPrerunMs) return false;
      phase_ = Phase::Gap;
      lastOffMs_ = nowMs - limits_.minOffMs; // first ON after fan prerun
    }

    // AIR thermostat: demand is kept while first warming to target. Once
    // reached, restart only after the configured user-target hysteresis.
    if (in.airC >= in.targetAirC) airDemand_ = false;
    else if (in.airC <= in.targetAirC - limits_.airHysteresisC)
      airDemand_ = true;
    // Local plate limiter has its OWN hysteresis; never map raw ohms directly
    // to C without a qualified NTC installation/thermal response.
    if (in.heaterTempC >= limits_.ntcSoftC) localReady_ = false;
    else if (in.heaterTempC <= limits_.ntcRearmC) localReady_ = true;

    if (phase_ == Phase::Heat) {
      if (!airDemand_ || !localReady_ ||
          nowMs - onStartedMs_ >= limits_.maxOnWindowMs) {
        phase_ = Phase::Gap;
        lastOffMs_ = nowMs;
        return false;
      }
      requestedOn_ = true;
      return true;
    }
    if (phase_ == Phase::Gap && airDemand_ && localReady_ &&
        nowMs - lastOffMs_ >= limits_.minOffMs) {
      phase_ = Phase::Heat;
      onStartedMs_ = nowMs;
      wasHeating_ = true;
      requestedOn_ = true;
      return true;
    }
    return false;
  }

  void stop() { requestedOn_ = false; phase_ = Phase::Idle; }
  Fault fault() const { return fault_; }
  bool heaterRequested() const { return requestedOn_; }
  bool airDemand() const { return airDemand_; }
  bool localReady() const { return localReady_; }

 private:
  enum class Phase : uint8_t { Idle, Prerun, Gap, Heat };
  static bool validLimits(const Limits& l, float target) {
    return l.qualified &&
        std::isfinite(l.ntcSoftC) && std::isfinite(l.ntcRearmC) &&
        std::isfinite(l.ntcHardC) && std::isfinite(l.airFaultC) &&
        std::isfinite(l.airHysteresisC) &&
        l.ntcRearmC < l.ntcSoftC && l.ntcSoftC < l.ntcHardC &&
        target >= 40.0f && target <= 55.0f &&
        l.airFaultC > target &&
        l.airHysteresisC > 0.0f && l.airHysteresisC < 5.0f &&
        l.fanPrerunMs > 0 && l.maxOnWindowMs > 0 &&
        l.maxOnWindowMs <= 20000 && l.minOffMs > 0 &&
        l.maxNtcAgeMs > 0 && l.maxAirAgeMs > 0;
  }
  bool abort(Fault reason) {
    requestedOn_ = false;
    fault_ = reason;
    phase_ = Phase::Idle;
    return false;
  }
  Limits limits_;
  Phase phase_ = Phase::Idle;
  Fault fault_ = Fault::None;
  bool requestedOn_ = false, airDemand_ = false;
  bool localReady_ = true, wasHeating_ = false;
  uint32_t fanStartedMs_ = 0, onStartedMs_ = 0, lastOffMs_ = 0;
};
} // namespace HeaterProductAir
