#pragma once
#include <cmath>
#include <cstdint>

// SINGLE supervised, non-repeating commissioning sequence. Not a thermostat.
// R_NTC is used as a relative electrical observation, NOT calibrated degrees C.
// This stage deliberately does not depend on purchasing an added TCO (D029).
namespace HeaterCommissioning {
enum class Phase : uint8_t {
  Idle, Prerun, Pulse, Gap, Cooldown, Complete, Aborted
};
enum class Fault : uint8_t {
  None, NtcInvalid, NtcStale, AirInvalid, AirStale, AirLimit,
  NtcResistanceLimit, FanUnavailable, FanLost, OverallTimeout,
  PulseWatchdog, ManualStop
};
struct Inputs {
  bool cycleActive = false;
  bool fanRunning = false; // PWM status, no tachometer/airflow measurement
  bool ntcValid = false;
  float ntcResistanceOhms = NAN;
  uint32_t ntcAgeMs = UINT32_MAX;
  bool airValid = false;
  float chamberC = NAN;
  uint32_t airAgeMs = UINT32_MAX;
};

struct Parameters {
  // Conservative short-pulse FIRST CHARACTERIZATION, NOT product setpoints.
  uint32_t prerunMs = 5000;
  uint32_t pulseMs = 1500;
  uint32_t gapMs = 12000;
  uint32_t cooldownMs = 45000;
  uint32_t overallTimeoutMs = 120000;
  uint32_t fanStartupTimeoutMs = 12000;
  uint32_t maxNtcAgeMs = 1500;
  uint32_t maxAirAgeMs = 5000;
  uint8_t pulseCount = 3;
  float maximumChamberC = 35.0f;
  // 42.27 kOhm was observed at nearby Sanwa ~39 C in OPEN/fan-OFF
  // geometry; this is a cautious RAW-NTC abort threshold for this test,
  // not a transfer function or a qualified closed-case plate temperature.
  float minimumNtcOhms = 42270.0f;
  float maximumNtcOhms = 250000.0f;
};

class Sequence {
 public:
  explicit Sequence(Parameters params = Parameters{}) : p_(params) {}

  bool update(uint32_t nowMs, Inputs in) {
    if (!in.cycleActive) {
      // Once started, a stopped test cannot be restarted without a reset.
      if (phase_ != Phase::Idle && phase_ != Phase::Complete &&
          phase_ != Phase::Aborted) abort(Fault::ManualStop);
      requestOn_ = false;
      return false;
    }
    if (phase_ == Phase::Aborted || phase_ == Phase::Complete) {
      requestOn_ = false;
      return false;
    }
    if (phase_ == Phase::Idle) {
      phase_ = Phase::Prerun;
      startedMs_ = nowMs;
      phaseSinceMs_ = nowMs;
      fanSeen_ = false;
      pulsesDone_ = 0;
    }
    requestOn_ = false;
    if (nowMs - startedMs_ >= p_.overallTimeoutMs) return abort(Fault::OverallTimeout);
    if (!in.ntcValid || !std::isfinite(in.ntcResistanceOhms))
      return abort(Fault::NtcInvalid);
    if (in.ntcAgeMs > p_.maxNtcAgeMs) return abort(Fault::NtcStale);
    if (!in.airValid || !std::isfinite(in.chamberC))
      return abort(Fault::AirInvalid);
    if (in.airAgeMs > p_.maxAirAgeMs) return abort(Fault::AirStale);
    if (in.chamberC >= p_.maximumChamberC) return abort(Fault::AirLimit);
    if (in.ntcResistanceOhms <= p_.minimumNtcOhms ||
        in.ntcResistanceOhms >= p_.maximumNtcOhms)
      return abort(Fault::NtcResistanceLimit);

    // Fan may need ~1 s kick-start before its PWM state becomes Running.
    if (!in.fanRunning) {
      if (fanSeen_ || phase_ != Phase::Prerun)
        return abort(Fault::FanLost);
      if (nowMs - startedMs_ >= p_.fanStartupTimeoutMs)
        return abort(Fault::FanUnavailable);
      return false;
    }
    if (!fanSeen_) {
      fanSeen_ = true;
      phaseSinceMs_ = nowMs;
    }
    switch (phase_) {
      case Phase::Prerun:
        if (nowMs - phaseSinceMs_ >= p_.prerunMs) {
          phase_ = Phase::Pulse;
          phaseSinceMs_ = nowMs;
        } else return false;
        // deliberate fallthrough: first pulse begins after prerun
      case Phase::Pulse:
        if (nowMs - phaseSinceMs_ >= p_.pulseMs) {
          ++pulsesDone_;
          phase_ = pulsesDone_ >= p_.pulseCount ? Phase::Cooldown : Phase::Gap;
          phaseSinceMs_ = nowMs;
          requestOn_ = false;
          return false;
        }
        hadHeat_ = true;
        lastHeatMs_ = nowMs;
        requestOn_ = true;
        return true;
      case Phase::Gap:
        if (nowMs - phaseSinceMs_ >= p_.gapMs) {
          phase_ = Phase::Pulse;
          phaseSinceMs_ = nowMs;
          lastHeatMs_ = nowMs;
          hadHeat_ = true;
          requestOn_ = true;
          return true;
        }
        return false;
      case Phase::Cooldown:
        if (nowMs - phaseSinceMs_ >= p_.cooldownMs) {
          phase_ = Phase::Complete;
        }
        return false;
      case Phase::Idle:
      case Phase::Complete:
      case Phase::Aborted:
        return false;
    }
    return false;
  }

  // Heater OFF immediately on stop; keep any previous heat history so fan
  // can continue through cooldown even after an operator abort / fault.
  void cancel() {
    requestOn_ = false;
    if (phase_ != Phase::Idle && phase_ != Phase::Aborted &&
        phase_ != Phase::Complete) abort(Fault::ManualStop);
  }
  void emergencyStop(Fault reason) { abort(reason); }
  bool cooldownNeeded(uint32_t nowMs) const {
    return hadHeat_ && (nowMs - lastHeatMs_) < p_.cooldownMs;
  }
  bool heaterRequested() const { return requestOn_; }
  Phase phase() const { return phase_; }
  Fault fault() const { return fault_; }
  uint8_t pulsesDone() const { return pulsesDone_; }
  bool finished() const { return phase_ == Phase::Complete; }

 private:
  bool abort(Fault why) {
    requestOn_ = false;
    fault_ = why;
    phase_ = Phase::Aborted;
    return false;
  }
  Parameters p_{};
  Phase phase_ = Phase::Idle;
  Fault fault_ = Fault::None;
  bool requestOn_ = false, fanSeen_ = false, hadHeat_ = false;
  uint8_t pulsesDone_ = 0;
  uint32_t startedMs_ = 0, phaseSinceMs_ = 0, lastHeatMs_ = 0;
};
} // namespace HeaterCommissioning
