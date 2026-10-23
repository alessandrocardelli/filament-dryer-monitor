#pragma once

// Pure heater REQUEST policy for native simulation and later firmware review.
// NO GPIO, PWM, heater actuation, or production integration lives here.
// The integrated SafetyController continues to force GPIO19 LOW.
#include <cmath>
#include <cstdint>
#include <limits>

namespace HeaterControl {

enum class Fault : uint8_t {
  None,
  UnapprovedLimits,
  NtcInvalid,
  NtcStale,
  ChamberInvalid,
  ChamberStale,
  FanLost,
  HeaterTooHot,
  ChamberTooHot,
  CycleTooLong,
  ContinuousHeatTooLong,
};

struct Limits {
  // Deliberately no hardware setpoints: these require measured qualification.
  bool approved = false;
  float maximumHeaterC = NAN;
  float maximumChamberC = NAN;
  float hysteresisC = NAN;
  uint32_t maximumNtcAgeMs = 0;
  uint32_t maximumChamberAgeMs = 0;
  uint32_t minimumFanPrerunMs = 0;
  uint32_t maximumContinuousHeatMs = 0;
  uint32_t maximumCycleMs = 0;
};

struct Inputs {
  bool cycleRequested = false;
  bool fanCommandedRunning = false; // Not proof of physical airflow: no tachometer!
  bool ntcElectricalValid = false;
  bool ntcThermallyCalibrated = false;
  float heaterTemperatureC = NAN;
  uint32_t ntcAgeMs = UINT32_MAX;
  bool chamberValid = false;
  float chamberTemperatureC = NAN;
  uint32_t chamberAgeMs = UINT32_MAX;
  float targetChamberC = NAN;
  uint32_t elapsedCycleMs = 0;
};

class Policy {
 public:
  bool evaluate(uint32_t nowMs, const Limits &limits, const Inputs &in) {
    requestOn_ = false; // fail OFF for each update; never a physical output
    if (fault_ != Fault::None) return false;
    if (!in.cycleRequested) {
      fanSeen_ = false;
      wasHeating_ = false;
      return false;
    }
    if (!validLimits(limits, in.targetChamberC)) return trip(Fault::UnapprovedLimits);
    if (!in.ntcElectricalValid || !in.ntcThermallyCalibrated ||
        !std::isfinite(in.heaterTemperatureC)) return trip(Fault::NtcInvalid);
    if (in.ntcAgeMs > limits.maximumNtcAgeMs) return trip(Fault::NtcStale);
    if (!in.chamberValid || !std::isfinite(in.chamberTemperatureC))
      return trip(Fault::ChamberInvalid);
    if (in.chamberAgeMs > limits.maximumChamberAgeMs)
      return trip(Fault::ChamberStale);
    if (in.heaterTemperatureC >= limits.maximumHeaterC)
      return trip(Fault::HeaterTooHot);
    if (in.chamberTemperatureC >= limits.maximumChamberC)
      return trip(Fault::ChamberTooHot);
    if (in.elapsedCycleMs >= limits.maximumCycleMs)
      return trip(Fault::CycleTooLong);

    if (!in.fanCommandedRunning) {
      fanSeen_ = false;
      if (wasHeating_) return trip(Fault::FanLost);
      wasHeating_ = false;
      return false;
    }
    if (!fanSeen_) {
      fanSeen_ = true;
      fanStartedMs_ = nowMs;
      wasHeating_ = false;
      return false;
    }
    if (static_cast<uint32_t>(nowMs - fanStartedMs_) <
        limits.minimumFanPrerunMs) return false;

    if (wasHeating_ && static_cast<uint32_t>(nowMs - heatStartedMs_) >=
                           limits.maximumContinuousHeatMs)
      return trip(Fault::ContinuousHeatTooLong);

    // Chamber control only. The heater NTC is a safety observation, not
    // the air setpoint. Hysteresis is deliberately conservative.
    const bool wantHeat = wasHeating_
        ? in.chamberTemperatureC < in.targetChamberC
        : in.chamberTemperatureC <= in.targetChamberC - limits.hysteresisC;
    if (wantHeat && !wasHeating_) heatStartedMs_ = nowMs;
    wasHeating_ = wantHeat;
    requestOn_ = wantHeat;
    return requestOn_;
  }

  // An explicit stop AND explicit operator acknowledgement are prerequisites.
  // Never auto-clear faults just because a reading later becomes plausible.
  bool clearFault(bool cycleStopped, bool operatorAcknowledged) {
    if (!cycleStopped || !operatorAcknowledged) return false;
    fault_ = Fault::None;
    requestOn_ = false;
    wasHeating_ = false;
    fanSeen_ = false;
    return true;
  }

  bool requestedOn() const { return requestOn_; }
  Fault fault() const { return fault_; }

 private:
  Fault fault_ = Fault::None;
  bool requestOn_ = false;
  bool wasHeating_ = false;
  bool fanSeen_ = false;
  uint32_t fanStartedMs_ = 0;
  uint32_t heatStartedMs_ = 0;

  static bool validLimits(const Limits &c, float target) {
    return c.approved && std::isfinite(c.maximumHeaterC) &&
        c.maximumHeaterC > 0.0f && std::isfinite(c.maximumChamberC) &&
        c.maximumChamberC > 0.0f && std::isfinite(c.hysteresisC) &&
        c.hysteresisC > 0.0f && std::isfinite(target) &&
        target > c.hysteresisC && target < c.maximumChamberC &&
        c.maximumNtcAgeMs > 0 && c.maximumChamberAgeMs > 0 &&
        c.minimumFanPrerunMs > 0 && c.maximumContinuousHeatMs > 0 &&
        c.maximumCycleMs > 0;
  }

  bool trip(Fault reason) {
    fault_ = reason;
    requestOn_ = false;
    wasHeating_ = false;
    return false;
  }
};

}  // namespace HeaterControl
