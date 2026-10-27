#pragma once
#include <cmath>
#include <cstdint>
#include "HeaterPulseCommissioning.h"

// STEP09: actual HEATER request policy for ONE supervised trial of
// SHT45 CHAMBER feedback. Uses NTC resistance as an independent, hard
// electrical guard; NO unqualified R_NTC -> temperature conversion.
// This is NOT a certified thermostat, no GPIO and no fan tach input.
// Do not flash HEATER-enabled candidate before physical protocol review.
namespace HeaterStep09AirFeedback {
using Phase = HeaterCommissioning::Phase; // 0 Idle;1 Prerun;2 Pulse;3 Gap;4 Cooldown;5 Complete;6 Aborted
using Fault = HeaterCommissioning::Fault;

struct Parameters {
  uint32_t prerunMs = 5000;
  uint32_t maximumOnWindowMs = 20000;
  uint32_t minimumOffMs = 12000;
  uint32_t cooldownMs = 90000;
  uint32_t maximumRunMs = 600000;
  uint32_t maximumTotalHeatMs = 120000;
  uint32_t fanStartupTimeoutMs = 12000;
  uint32_t maxNtcAgeMs = 1500;
  uint32_t maxAirAgeMs = 5000;
  float targetAirC = 25.0f;
  float airHysteresisC = 1.0f;
  float airFaultCeilingC = 40.0f;
  float ntcSoftPauseOhms = 50000.0f;
  float ntcResumeOhms = 65000.0f;
  float ntcHardTripOhms = 40000.0f;
  float ntcOpenOhms = 250000.0f;
};

struct Inputs {
  bool cycleActive = false;
  bool fanRunning = false;    // PWM state only. Physical fan rotation unmeasured.
  bool ntcValid = false;
  float ntcOhms = NAN;
  uint32_t ntcAgeMs = UINT32_MAX;
  bool airValid = false;
  float airC = NAN;
  uint32_t airAgeMs = UINT32_MAX;
};

class Controller {
 public:
  explicit Controller(Parameters params = Parameters{}) : params_(params) {}

  bool update(uint32_t nowMs, const Inputs &in) {
    requestedOn_ = false;
    if (!in.cycleActive) {
      if (phase_ != Phase::Idle && phase_ != Phase::Complete &&
          phase_ != Phase::Aborted) cancel();
      return false;
    }
    if (phase_ == Phase::Complete || phase_ == Phase::Aborted) return false;
    if (phase_ == Phase::Idle) {
      phase_ = Phase::Prerun;
      startedMs_ = nowMs;
      phaseSinceMs_ = nowMs;
    }

    // Cooling always runs to completion; never resume HEAT after maximum
    // energy/time or STOP, even when the SHT45 still wants warmth.
    if (phase_ == Phase::Cooldown) {
      if (!cooldownNeeded(nowMs)) phase_ = Phase::Complete;
      return false;
    }
    if (phase_ == Phase::Pulse) {
      const uint32_t delta = nowMs - lastOnTickMs_;
      totalHeatMs_ += delta > params_.maximumOnWindowMs
                    ? params_.maximumOnWindowMs : delta;
      lastOnTickMs_ = nowMs;
      lastHeaterOnMs_ = nowMs;
    }
    if (nowMs - startedMs_ >= params_.maximumRunMs ||
        totalHeatMs_ >= params_.maximumTotalHeatMs) {
      finishHeating(nowMs);
      return false;
    }
    if (!in.ntcValid || !std::isfinite(in.ntcOhms)) return abort(Fault::NtcInvalid);
    if (in.ntcAgeMs > params_.maxNtcAgeMs) return abort(Fault::NtcStale);
    if (!in.airValid || !std::isfinite(in.airC)) return abort(Fault::AirInvalid);
    if (in.airAgeMs > params_.maxAirAgeMs) return abort(Fault::AirStale);
    if (in.airC >= params_.airFaultCeilingC) return abort(Fault::AirLimit);
    // Same RAW 40kOhm hard trip demonstrated by physical S7U.
    // The separate 50kOhm soft PAUSE is deliberately more conservative.
    if (in.ntcOhms <= params_.ntcHardTripOhms ||
        in.ntcOhms >= params_.ntcOpenOhms)
      return abort(Fault::NtcResistanceLimit);
    if (!in.fanRunning) {
      if (fanSeen_ || phase_ != Phase::Prerun)
        return abort(Fault::FanLost);
      if (nowMs - startedMs_ >= params_.fanStartupTimeoutMs)
        return abort(Fault::FanUnavailable);
      return false;
    }
    if (!fanSeen_) {
      fanSeen_ = true;
      phaseSinceMs_ = nowMs;
      return false;
    }
    if (phase_ == Phase::Prerun) {
      if (nowMs - phaseSinceMs_ < params_.prerunMs) return false;
      phase_ = Phase::Gap;
      // First heat can start immediately after the fan prerun.
      phaseSinceMs_ = nowMs - params_.minimumOffMs;
    }
    if (phase_ == Phase::Pulse) {
      const bool shouldPause =
          nowMs - heatWindowStartedMs_ >= params_.maximumOnWindowMs ||
          in.ntcOhms <= params_.ntcSoftPauseOhms ||
          in.airC >= params_.targetAirC;
      if (shouldPause) {
        ++windowsDone_;
        phase_ = Phase::Gap;
        phaseSinceMs_ = nowMs;
        return false;
      }
      requestedOn_ = true;
      return true;
    }
    if (phase_ == Phase::Gap &&
        nowMs - phaseSinceMs_ >= params_.minimumOffMs &&
        in.ntcOhms >= params_.ntcResumeOhms &&
        in.airC <= params_.targetAirC - params_.airHysteresisC) {
      phase_ = Phase::Pulse;
      heatWindowStartedMs_ = nowMs;
      lastOnTickMs_ = nowMs;
      lastHeaterOnMs_ = nowMs;
      hadHeat_ = true;
      requestedOn_ = true;
      return true;
    }
    return false;
  }

  void cancel() {
    requestedOn_ = false;
    if (phase_ != Phase::Idle && phase_ != Phase::Complete &&
        phase_ != Phase::Aborted) phase_ = Phase::Aborted;
  }
  void emergencyStop(Fault reason) { abort(reason); }
  bool canStart() const { return phase_ == Phase::Idle; }
  bool cooldownNeeded(uint32_t nowMs) const {
    return hadHeat_ && nowMs - lastHeaterOnMs_ < params_.cooldownMs;
  }
  bool finished() const { return phase_ == Phase::Complete; }
  bool heaterRequested() const { return requestedOn_; }
  uint8_t pulsesDone() const { return windowsDone_; }
  Phase phase() const { return phase_; }
  Fault fault() const { return fault_; }
  uint32_t totalHeatMs() const { return totalHeatMs_; }

 private:
  bool abort(Fault reason) {
    fault_ = reason;
    requestedOn_ = false;
    phase_ = Phase::Aborted;
    return false;
  }
  void finishHeating(uint32_t nowMs) {
    requestedOn_ = false;
    phase_ = Phase::Cooldown;
    phaseSinceMs_ = nowMs;
    if (!hadHeat_) phase_ = Phase::Complete;
  }

  Parameters params_{};
  Phase phase_ = Phase::Idle;
  Fault fault_ = Fault::None;
  bool requestedOn_ = false, fanSeen_ = false, hadHeat_ = false;
  uint8_t windowsDone_ = 0;
  uint32_t startedMs_ = 0, phaseSinceMs_ = 0;
  uint32_t heatWindowStartedMs_ = 0, lastOnTickMs_ = 0;
  uint32_t lastHeaterOnMs_ = 0, totalHeatMs_ = 0;
};

inline constexpr uint32_t kHardwareWindowCeilingUs = 20500000;
// Candidate one-shot supervised trial parameters. These are *proposals*,
// not production ratings and not permission for a physical test.
inline Parameters parameters() { return Parameters{}; }
}  // namespace HeaterStep09AirFeedback
