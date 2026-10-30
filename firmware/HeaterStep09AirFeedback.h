#pragma once
#include <cmath>
#include <cstdint>
#include "HeaterPulseCommissioning.h"

// STEP13A: candidate AIR30 and less restrictive but still bounded local NTC.
// Source of change: 2026-10-29 STEP12P real 1089-row CSV and manual table:
// AIR25 reached at+400s (vs STEP10B+563s), genuine AIR24 rearm at+685s,
// ON=231.114s at global12min timeout, minNTC34.483k, zero faults.
// STEP12P soft35k clipped ON windows to13-17.5s; new soft33k retains
// a *3kOhm* resistance interval above the UNQUALIFIED 30k backup fault.
// Previous S6C closed/fan100 saw Sanwa PLATE53C at minNTC30.32k.
// This does not predict the new plate Tmax, NTC bead temperature or materials.
// ALL numbers below are provisional test inputs, not operational approvals.
// No automatic heater energization or bypass of installed TCO105 authorized.
namespace HeaterStep09AirFeedback {
using Phase = HeaterCommissioning::Phase; // 0 Idle;1 Prerun;2 Pulse;3 Gap;4 Cooldown;5 Complete;6 Aborted
using Fault = HeaterCommissioning::Fault;

// Distinguish a physically bounded one-off heater experiment from
// the product design. A cumulative-ON budget must NEVER become a normal
// drying-cycle limit. LongDuration is currently HOST-TEST ONLY:
// SafetyController's hardware constructor uses SupervisedTrial exclusively.
enum class RunPolicy : uint8_t {
  SupervisedTrial = 0,
  DurationControlledDesign = 1
};

struct Parameters {
  RunPolicy runPolicy = RunPolicy::SupervisedTrial;
  // Only the new explicitly selected attended 85/80 local-NTC controller.
  // Legacy STEP12/13 one-shot time-bounded pulses remain isolated.
  bool continuousLocalThermostat = false;
  uint32_t prerunMs = 5000;
  uint32_t maximumOnWindowMs = 20000;
  uint32_t minimumOffMs = 12000;
  uint32_t cooldownMs = 90000;
  // SUPERVISED TRIAL ONLY: historical 900s/360s caps exist to bound a
  // single, separately approved physical test, NOT normal device operation.
  // DurationControlledDesign ignores these fields: CycleController's
  // configured user duration ends the cycle through Inputs.cycleActive.
  uint32_t maximumRunMs = 900000;
  uint32_t maximumTotalHeatMs = 360000;
  uint32_t fanStartupTimeoutMs = 12000;
  uint32_t maxNtcAgeMs = 1500;
  uint32_t maxAirAgeMs = 5000;
  float targetAirC = 30.0f; // next higher CHAMBER target, not heater-plate target
  float airHysteresisC = 1.0f;
  float airFaultCeilingC = 40.0f;
  // All R_NTC boundaries are PROVISIONAL TEST values, not fixed temperatures.
  // STEP13A: SOFT33k (down2k from STEP12P, NOT a calibrated Celsius
  // ceiling), REARM50k unchanged, BACKUP raw cutoff30k unchanged.
  // The STEP07 40k experimental early trip is historically retired.
  // Sensor invalid/stale, air40C, ISR20.5s, time/energy bounds, STOP and
  // physically installed series TCO remain independent of these thresholds.
  float ntcSoftPauseOhms = 33000.0f;
  float ntcResumeOhms = 50000.0f;
  float ntcHardTripOhms = 30000.0f;
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
  // External user-selected AIR target for attended long-duration research.
  // Legacy STEP12/13 one-shot profiles keep their fixed params_.targetAirC.
  float selectedAirTargetC = NAN;
};

class Controller {
 public:
  enum class WaitReason : uint8_t {
    None = 0, MinimumOff = 1, NtcRecovery = 2, AirHysteresis = 3
  };
  explicit Controller(Parameters params = Parameters{}) : params_(params) {}

  bool update(uint32_t nowMs, const Inputs &in) {
    requestedOn_ = false;
    const bool userDuration = params_.runPolicy == RunPolicy::DurationControlledDesign;
    const float demandTargetC = userDuration ? in.selectedAirTargetC : params_.targetAirC;
    if (!in.cycleActive) {
      if (params_.runPolicy == RunPolicy::DurationControlledDesign) {
        // Duration selection belongs to the external CycleController, not
        // an accumulated heater-seconds counter. Reaching the user timer is
        // a normal finish with cooling, NOT a ManualStop fault.
        if (phase_ == Phase::Cooldown) {
          if (!cooldownNeeded(nowMs)) phase_ = Phase::Complete;
        } else if (phase_ != Phase::Idle && phase_ != Phase::Complete &&
                   phase_ != Phase::Aborted) {
          finishHeating(nowMs);
        }
      } else if (phase_ != Phase::Idle && phase_ != Phase::Complete &&
                 phase_ != Phase::Aborted) {
        cancel(); // historical one-shot experiment
      }
      return false;
    }
    if (phase_ == Phase::Complete || phase_ == Phase::Aborted) return false;
    // Never energize for a missing/out-of-range menu selection.
    if (!std::isfinite(demandTargetC) ||
        (userDuration && (demandTargetC < 40.0f || demandTargetC > 55.0f)) ||
        demandTargetC >= params_.airFaultCeilingC)
      return abort(Fault::AirInvalid);
    if (phase_ == Phase::Idle) {
      phase_ = Phase::Prerun;
      startedMs_ = nowMs;
      phaseSinceMs_ = nowMs;
      // Initially request heat for any chamber air below target. Hysteresis
      // is armed only AFTER the chamber actually reaches target.
      airDemandActive_ = in.airC < demandTargetC;
      // A hot restart cannot bypass the lower NTC rearm threshold.
      localHeatReady_ = !params_.continuousLocalThermostat ||
                        (std::isfinite(in.ntcOhms) &&
                         in.ntcOhms >= params_.ntcResumeOhms);
    }

    // Cooling always runs to completion after a trial cap or STOP.
    // Normal drying is governed by user-selected duration, NOT total
    // accumulated heater ON time; no autonomous hardware mode enabled.
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
    // Trial energy/time caps are NOT product protection thresholds.
    // DurationControlledDesign is tested offline and must not be selected
    // on hardware without independently validated thermal protection.
    // Per-window 20s limit, 20.5s GPIO hardware timer, fan/NTC/air faults
    // and STOP remain active with either run policy.
    if (params_.runPolicy == RunPolicy::SupervisedTrial &&
        (nowMs - startedMs_ >= params_.maximumRunMs ||
         totalHeatMs_ >= params_.maximumTotalHeatMs)) {
      finishHeating(nowMs);
      return false;
    }
    if (!in.ntcValid || !std::isfinite(in.ntcOhms)) return abort(Fault::NtcInvalid);
    if (in.ntcAgeMs > params_.maxNtcAgeMs) return abort(Fault::NtcStale);
    if (!in.airValid || !std::isfinite(in.airC)) return abort(Fault::AirInvalid);
    if (in.airAgeMs > params_.maxAirAgeMs) return abort(Fault::AirStale);
    if (in.airC >= params_.airFaultCeilingC) return abort(Fault::AirLimit);
    // Chamber thermostat is a latch, independent from NTC soft-pause.
    // Previously every NTC pause required air<=24C to restart even when
    // chamber had NEVER reached 25C. Keep demand until 25C is first reached.
    if (in.airC >= demandTargetC) airDemandActive_ = false;
    else if (in.airC <= demandTargetC - params_.airHysteresisC)
      airDemandActive_ = true;
    // Local NTC raw-resistance boundary; never use the nearby Sanwa as if
    // the MCU could read it. For this attended 85C-contact research profile:
    //   R==7.12k -> NORMAL thermostat OFF;
    //   R <7.12k -> LATCHED FAULT (outside highest measured hot endpoint).
    // This protects against continuing beyond the empirical range; any
    // thermal-inertia undershoot may intentionally fault the session.
    // Historical one-shot trials keep their old <= hard-trip semantics.
    if ((params_.continuousLocalThermostat
             ? in.ntcOhms < params_.ntcHardTripOhms
             : in.ntcOhms <= params_.ntcHardTripOhms) ||
        in.ntcOhms >= params_.ntcOpenOhms)
      return abort(Fault::NtcResistanceLimit);
    if (params_.continuousLocalThermostat) {
      if (in.ntcOhms <= params_.ntcSoftPauseOhms)
        localHeatReady_ = false;
      else if (in.ntcOhms >= params_.ntcResumeOhms)
        localHeatReady_ = true;
    }
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
          (!params_.continuousLocalThermostat &&
           nowMs - heatWindowStartedMs_ >= params_.maximumOnWindowMs) ||
          (params_.continuousLocalThermostat
               ? !localHeatReady_
               : in.ntcOhms <= params_.ntcSoftPauseOhms) ||
          !airDemandActive_;
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
        (params_.continuousLocalThermostat ||
         nowMs - phaseSinceMs_ >= params_.minimumOffMs) &&
        (params_.continuousLocalThermostat
             ? localHeatReady_
             : in.ntcOhms >= params_.ntcResumeOhms) &&
        airDemandActive_) {
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
        phase_ != Phase::Aborted) abort(Fault::ManualStop);
  }
  // The user-selected timer expiring is a NORMAL end, never ManualStop.
  // Keep cooldown and post-OFF sampling distinct from an abort.
  void finishUserCycle(uint32_t nowMs) {
    if (params_.runPolicy == RunPolicy::DurationControlledDesign &&
        phase_ != Phase::Idle && phase_ != Phase::Aborted &&
        phase_ != Phase::Complete)
      finishHeating(nowMs);
  }
  void emergencyStop(Fault reason) { abort(reason); }
  bool canStart() const { return phase_ == Phase::Idle; }
  // After explicit UI acknowledgement and completed post-heat cooldown,
  // allow a fresh user timer without reboot or auto-resuming after a fault.
  bool resetForNewSession(uint32_t nowMs) {
    if (cooldownNeeded(nowMs) || requestedOn_) return false;
    if (phase_ != Phase::Aborted && phase_ != Phase::Complete) return false;
    phase_ = Phase::Idle;
    fault_ = Fault::None;
    fanSeen_ = hadHeat_ = airDemandActive_ = false;
    localHeatReady_ = false;
    windowsDone_ = totalHeatMs_ = 0;
    return true;
  }
  bool cooldownNeeded(uint32_t nowMs) const {
    return hadHeat_ && nowMs - lastHeaterOnMs_ < params_.cooldownMs;
  }
  bool finished() const { return phase_ == Phase::Complete; }
  bool heaterRequested() const { return requestedOn_; }
  uint8_t pulsesDone() const {
    // Exposed status is one byte; multi-hour sessions must not wrap 255->0.
    return windowsDone_ > 255U ? 255U : static_cast<uint8_t>(windowsDone_);
  }
  Phase phase() const { return phase_; }
  Fault fault() const { return fault_; }
  uint32_t totalHeatMs() const { return totalHeatMs_; }
  bool airDemandActive() const { return airDemandActive_; }
  WaitReason waitReason(uint32_t nowMs, float ntcOhms) const {
    if (phase_ != Phase::Gap) return WaitReason::None;
    if (!params_.continuousLocalThermostat &&
        nowMs - phaseSinceMs_ < params_.minimumOffMs)
      return WaitReason::MinimumOff;
    if (params_.continuousLocalThermostat
            ? !localHeatReady_
            : (!std::isfinite(ntcOhms) || ntcOhms < params_.ntcResumeOhms))
      return WaitReason::NtcRecovery;
    if (!airDemandActive_) return WaitReason::AirHysteresis;
    return WaitReason::None;
  }

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
  bool airDemandActive_ = false;
  bool localHeatReady_ = false;
  uint32_t windowsDone_ = 0;
  uint32_t startedMs_ = 0, phaseSinceMs_ = 0;
  uint32_t heatWindowStartedMs_ = 0, lastOnTickMs_ = 0;
  uint32_t lastHeaterOnMs_ = 0, totalHeatMs_ = 0;
};

inline constexpr uint32_t kHardwareWindowCeilingUs = 20500000;
// Candidate one-shot supervised trial parameters. These are *proposals*,
// not production ratings and not permission for a physical test.
inline Parameters parameters() { return Parameters{}; }

// Offline design fixture only: never pass this to the HEATER-capable
// SafetyController constructor until the product control/safety gate is
// explicitly qualified and approved. No production enable flag is changed.
inline Parameters durationControlledDesignParameters() {
  Parameters p{};
  p.runPolicy = RunPolicy::DurationControlledDesign;
  p.airFaultCeilingC = 60.0f; // HOST-ONLY synthetic target40 trial
  return p;
}
// NEW attended source profile: requested continuous heater-local 85/80C
// thermostat at the independently measured Sanwa PLATE contact, NOT a
// calibrated NTC-bead Celsius model.
// ORIGINAL 17 October OPEN covers/FAN OFF:
//   Sanwa PLATE74C -> NTC9.81k; PLATE85C -> NTC7.12k.
//   PLATE80C estimated by log(R) interpolation -> 8,236.595 ohm.
//   PLATE64C ->13.73k was the PREVIOUS mistakenly wide rearm: RETIRED.
// The current closed-box/FAN100 heater may have DIFFERENT plate-to-bead
// gradients; these are EXPERIMENTAL operator-approved targets, not qualified
// local overheat or TCO-body ratings. Stop/FAULT on any sub-7.12k reading.
// Independent 20.5s GPIO ISR is FED WHILE HEATING by a validated live loop,
// never used to force repeated 20s heater-OFF windows. Do not bypass TCO105.
// AIR55/54 is a separate thermostat; operator timer is sole overall duration.
inline Parameters attendedAir55Parameters() {
  Parameters p{};
  p.runPolicy = RunPolicy::DurationControlledDesign;
  p.continuousLocalThermostat = true;
  p.ntcSoftPauseOhms = 7120.0f; // measured Sanwa PLATE85C
  p.ntcResumeOhms = 8236.6f; // interpolated Sanwa PLATE80C, NOT measured
  p.ntcHardTripOhms = 7120.0f; // strictly BELOW this => latching fault
  p.minimumOffMs = 0; // no timer-driven OFF; only actual two thermostats
  p.airFaultCeilingC = 60.0f; // provisional independent AIR fault
  p.targetAirC = 55.0f; // fallback only; user's AIR menu is authoritative
  return p;
}
}  // namespace HeaterStep09AirFeedback
