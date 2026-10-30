#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "AppConfig.h"
#include "AppTypes.h"
#include "HeaterControlPolicy.h"
#include "HeaterProductAir.h"
#include "HeaterPulseCommissioning.h"
#include "HeaterStep07GuardTrial.h"
#include "HeaterStep09AirFeedback.h"
#ifdef ARDUINO_ARCH_ESP32
#include "driver/gpio.h"
#endif

// Integrates the real GPIO19 output with the host-tested control policy.
// STEP13 historical trial is OFF in the current source. The production AIR
// controller is integrated but GPIO remains locked by independently required
// thermal qualification flags and unconfigured local temperature limits.
// Sensor/thermal qualification and independent physical safeguards are
// separate from the optional added TCO (D029).
class SafetyController {
 public:
  void begin() {
    // GPIO is driven LOW before any other peripheral is initialized.
    pinMode(AppConfig::kPinHeaterPwm, OUTPUT);
    forceHeaterOff();
#ifdef ARDUINO_ARCH_ESP32
    if (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode ||
        AppConfig::kHeaterOutputEnabled) {
      // Product mode MUST have the same independent per-window GPIO hard OFF.
      // Do not enable a thermal regulator without this ISR and validated limits.
      pulseTimer_ = timerBegin(1000000); // one tick per microsecond
      if (!pulseTimer_) {
        faultActive_ = true;
        return; // never heat without the independent pulse deadline
      }
      timerAttachInterrupt(pulseTimer_, &SafetyController::pulseTimerIsr);
      timerAlarm(pulseTimer_, (AppConfig::kHeaterStep09TrialMode ||
                               AppConfig::kHeaterOutputEnabled)
                      ? HeaterStep09AirFeedback::kHardwareWindowCeilingUs
                      : HeaterStep07GuardTrial::kPulseHardwareCeilingUs,
                 false, 0); // bounded 20.5 s per window; never continuous
      timerStop(pulseTimer_);
    }
#endif
  }

  // Synchronous immediate shutdown for STOP / COMPLETE / FAULT / BOOT.
  void stop() {
    forceHeaterOff();
    commission_.cancel();
    feedback_.cancel();
    product_.stop();
    HeaterControl::Inputs stopped{};
    policy_.evaluate(0, HeaterControl::Limits{}, stopped);
  }
  // Normal cycle timer expiration, as distinct from explicit operator STOP.
  // The heater is off synchronously before the UI transitions to COMPLETE.
  void completeUserCycle(uint32_t nowMs) {
    forceHeaterOff();
    if (AppConfig::kHeaterStep09TrialMode)
      feedback_.finishUserCycle(nowMs);
    else commission_.cancel();
    product_.stop();
  }

  void update(uint32_t nowMs, const NtcSnapshot &ntc,
              const SensorSnapshot &chamber, bool cycleActive,
              bool fanPwmRunning, uint32_t elapsedCycleMs,
              float targetChamberC) {
    lastNowMs_ = nowMs;
    // The user cycle timer owns duration; no accumulated-heat test budget.
    (void)elapsedCycleMs;
    // Remember the last known physical ON while the loop remains responsive.
    // stop()/FAULT must not erase residual plate heat and fan-overrun needs.
    if (heaterOn_) lastHeaterOnMs_ = nowMs;
#ifdef ARDUINO_ARCH_ESP32
    if ((AppConfig::kHeaterCommissioningMode ||
         AppConfig::kHeaterStep09TrialMode ||
         AppConfig::kHeaterOutputEnabled) && pulseTimerFired_) {
      if (AppConfig::kHeaterStep09TrialMode)
        feedback_.emergencyStop(HeaterCommissioning::Fault::PulseWatchdog);
      else if (AppConfig::kHeaterCommissioningMode)
        commission_.emergencyStop(HeaterCommissioning::Fault::PulseWatchdog);
      faultActive_ = true;
      forceHeaterOff();
      return;
    }
#endif
    // Carry over the previously validated electrical NTC fault latch even
    // when heating is disabled.
    if (ntc.sampled && ntc.fault != NtcFault::None) {
      faultActive_ = true;
      ntcFault_ = ntc.fault;
    }

    if (faultActive_) {
      forceHeaterOff();
      return;
    }

    if (AppConfig::kHeaterCommissioningMode) {
      HeaterCommissioning::Inputs t{};
      t.cycleActive = cycleActive;
      t.fanRunning = fanPwmRunning;
      t.ntcValid = ntc.sampled && ntc.valid && ntc.fault == NtcFault::None;
      t.ntcResistanceOhms = ntc.resistanceOhms;
      t.ntcAgeMs = ntc.sampled ? nowMs - ntc.lastSampleMs : UINT32_MAX;
      t.airValid = chamber.valid;
      t.chamberC = chamber.temperatureC;
      t.airAgeMs = chamber.valid ? nowMs - chamber.lastSuccessMs : UINT32_MAX;
      const bool pulse = commission_.update(nowMs, t);
      if (commission_.fault() != HeaterCommissioning::Fault::None) {
        faultActive_ = true;
        forceHeaterOff();
        return;
      }
      setHeater(pulse);
      return;
    }

    if (AppConfig::kHeaterStep09TrialMode) {
      HeaterStep09AirFeedback::Inputs t{};
      t.cycleActive = cycleActive;
      t.fanRunning = fanPwmRunning; // command only, no tachometer
      t.ntcValid = ntc.sampled && ntc.valid && ntc.fault == NtcFault::None;
      t.ntcOhms = ntc.resistanceOhms; // RAW OHMS, NO Celsius conversion
      t.ntcAgeMs = ntc.sampled ? nowMs - ntc.lastSampleMs : UINT32_MAX;
      t.airValid = chamber.valid;
      t.airC = chamber.temperatureC;
      t.airAgeMs = chamber.valid ? nowMs - chamber.lastSuccessMs : UINT32_MAX;
      t.selectedAirTargetC = targetChamberC; // USER menu, 40..55C
      const bool request = feedback_.update(nowMs, t);
      if (feedback_.fault() != HeaterCommissioning::Fault::None) {
        faultActive_ = true;
        forceHeaterOff();
        return;
      }
      setHeater(request);
      return;
    }

    if (!AppConfig::kHeaterOutputEnabled || !AppConfig::kHeaterProfileApproved ||
        !AppConfig::kHeaterNtcModelApproved) {
      forceHeaterOff();
      return;
    }

    // Qualified V1 thermostat: actual user-set chamber AIR temperature,
    // independent local heater soft/rearm/hard limits and sensor faults.
    // In the repository all qualification flags and Celsius limits are
    // deliberately UNSET, so reaching this code cannot energize GPIO19.
    HeaterProductAir::Inputs in{};
    in.cycleActive = cycleActive;
    in.fanCommandedRunning = fanPwmRunning; // PWM only, no tachometer
    in.ntcElectricalValid = ntc.sampled && ntc.valid &&
                            ntc.fault == NtcFault::None;
    in.ntcThermallyQualified = AppConfig::kHeaterNtcModelApproved;
    in.heaterTempC = ntc.temperatureC;
    in.ntcAgeMs = ntc.sampled ? nowMs - ntc.lastSampleMs : UINT32_MAX;
    in.airValid = chamber.valid;
    in.airC = chamber.temperatureC;
    in.airAgeMs = chamber.valid ? nowMs - chamber.lastSuccessMs : UINT32_MAX;
    in.targetAirC = targetChamberC;
    const bool requested = product_.update(nowMs, in);
    if (product_.fault() != HeaterProductAir::Fault::None) {
      faultActive_ = true;
      forceHeaterOff();
      return;
    }
    // Runtime state + separately approved configuration + independent ISR
    // are ALL needed. No trial NTC resistance number becomes a V1 limit.
    setHeater(requested && cycleActive);
    return;


  }

  bool heaterPermitted() const { return !heaterLocked() && !faultActive_; }
  bool commissioningComplete() const {
    return AppConfig::kHeaterStep09TrialMode ? feedback_.finished()
        : AppConfig::kHeaterCommissioningMode && commission_.finished();
  }
  bool commissioningCanStart(uint32_t nowMs = 0) {
    if (faultActive_ || coolingRequired(nowMs)) return false;
    if (AppConfig::kHeaterStep09TrialMode && !feedback_.canStart())
      feedback_.resetForNewSession(nowMs); // only invoked on explicit START press
    return AppConfig::kHeaterStep09TrialMode ? feedback_.canStart()
                                              : commission_.canStart();
  }
  // Display-only estimate of the fan cooling countdown. No control decision
  // relies on this value; coolingRequired() below remains authoritative.
  uint32_t coolingSecondsRemaining(uint32_t nowMs) const {
    if (!coolingRequired(nowMs) || !everHeated_) return 0;
    const uint32_t elapsed = nowMs - lastHeaterOnMs_;
    const uint32_t total = AppConfig::kHeaterStep09TrialMode
        ? HeaterStep09AirFeedback::parameters().cooldownMs
        : (AppConfig::kHeaterCommissioningMode
            ? HeaterStep07GuardTrial::parameters().cooldownMs
            : AppConfig::kPostHeatCooldownMs);
    return elapsed < total ? (total - elapsed + 999U) / 1000U : 0;
  }
  bool coolingRequired(uint32_t nowMs) const {
    const bool commissioningCooldown =
        (AppConfig::kHeaterCommissioningMode && commission_.cooldownNeeded(nowMs)) ||
        (AppConfig::kHeaterStep09TrialMode && feedback_.cooldownNeeded(nowMs));
    const bool recentPhysicalHeating = everHeated_ &&
        static_cast<uint32_t>(nowMs - lastHeaterOnMs_) <
            AppConfig::kPostHeatCooldownMs;
    return commissioningCooldown || recentPhysicalHeating;
  }
  uint8_t commissioningPhase() const {
    return static_cast<uint8_t>(AppConfig::kHeaterStep09TrialMode
        ? feedback_.phase() : commission_.phase());
  }
  uint8_t commissioningPulsesDone() const {
    return AppConfig::kHeaterStep09TrialMode
        ? feedback_.pulsesDone() : commission_.pulsesDone();
  }
  uint8_t commissioningFault() const {
    return static_cast<uint8_t>(AppConfig::kHeaterStep09TrialMode
        ? feedback_.fault() : commission_.fault());
  }
  uint8_t feedbackWaitReason(uint32_t nowMs, float ntcOhms) const {
    return AppConfig::kHeaterStep09TrialMode
      ? static_cast<uint8_t>(feedback_.waitReason(nowMs, ntcOhms)) : 0;
  }
  uint32_t feedbackTotalHeatMs() const {
    return AppConfig::kHeaterStep09TrialMode ? feedback_.totalHeatMs() : 0;
  }
  bool heaterLocked() const {
    if (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode)
      return false;
    return !AppConfig::kHeaterOutputEnabled ||
           !AppConfig::kHeaterProfileApproved ||
           !AppConfig::kHeaterNtcModelApproved;
  }
  bool heaterOn() const { return heaterOn_; }
  HeaterControl::Fault heaterFault() const { return policy_.fault(); }
  bool fanPermitted() const {
    return !faultActive_ || coolingRequired(lastNowMs_);
  }
  bool faultActive() const { return faultActive_; }
  NtcFault ntcFault() const { return ntcFault_; }

 private:
  HeaterControl::Policy policy_{}; // legacy native replay and stop() only
  static HeaterProductAir::Limits productLimits() {
    HeaterProductAir::Limits lim{};
    lim.qualified = AppConfig::kHeaterProfileApproved &&
                    AppConfig::kHeaterNtcModelApproved;
    lim.ntcSoftC = AppConfig::kHeaterSoftPauseMeasuredC;
    lim.ntcRearmC = AppConfig::kHeaterRearmMeasuredC;
    lim.ntcHardC = AppConfig::kHeaterMaxMeasuredC;
    lim.airFaultC = AppConfig::kChamberMaxMeasuredC;
    lim.airHysteresisC = AppConfig::kChamberControlHysteresisC;
    lim.fanPrerunMs = AppConfig::kHeaterFanPrerunMs;
    lim.maxNtcAgeMs = AppConfig::kHeaterNtcMaxAgeMs;
    lim.maxAirAgeMs = AppConfig::kHeaterChamberMaxAgeMs;
    return lim;
  }
  HeaterProductAir::Controller product_{productLimits()};
  HeaterCommissioning::Sequence commission_{HeaterStep07GuardTrial::parameters()};
  HeaterStep09AirFeedback::Controller feedback_{HeaterStep09AirFeedback::attendedAir55Parameters()};
  uint32_t lastNowMs_ = 0;
  uint32_t lastHeaterOnMs_ = 0;
  bool everHeated_ = false;
#ifdef ARDUINO_ARCH_ESP32
  hw_timer_t *pulseTimer_ = nullptr;
  inline static volatile bool pulseTimerFired_ = false;
  static void ARDUINO_ISR_ATTR pulseTimerIsr() {
    gpio_set_level(static_cast<gpio_num_t>(AppConfig::kPinHeaterPwm), 0);
    pulseTimerFired_ = true;
  }
#endif
  bool faultActive_ = false;
  bool heaterOn_ = false;
  NtcFault ntcFault_ = NtcFault::None;

  void forceHeaterOff() {
    digitalWrite(AppConfig::kPinHeaterPwm, LOW);
    heaterOn_ = false;
#ifdef ARDUINO_ARCH_ESP32
    if (pulseTimer_) timerStop(pulseTimer_);
#endif
  }

  void setHeater(bool requested) {
    const bool desired =
        (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode ||
         AppConfig::kHeaterOutputEnabled) &&
        requested;
    if (heaterOn_ == desired) {
#ifdef ARDUINO_ARCH_ESP32
      // PRODUCT-ONLY: healthy-loop watchdog refresh during CONTINUOUS
      // thermostat demand. A stalled loop cannot refresh the independent
      // timer ISR, which cuts GPIO19 LOW after 20.5s. Never introduce a
      // periodic heater OFF pulse merely to rearm this watchdog. The legacy
      // experimental 20s pulse path remains UNCHANGED and separately gated.
      if (desired && AppConfig::kHeaterOutputEnabled &&
          !AppConfig::kHeaterStep09TrialMode &&
          !AppConfig::kHeaterCommissioningMode) {
        if (!pulseTimer_ || pulseTimerFired_) {
          faultActive_ = true;
          forceHeaterOff();
          return;
        }
        timerWrite(pulseTimer_, 0);
        timerRestart(pulseTimer_);
      }
#endif
      return;
    }
    if (desired) {
#ifdef ARDUINO_ARCH_ESP32
      if (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode ||
          AppConfig::kHeaterOutputEnabled) {
        if (!pulseTimer_ || pulseTimerFired_) {
          if (AppConfig::kHeaterStep09TrialMode)
            feedback_.emergencyStop(HeaterCommissioning::Fault::PulseWatchdog);
          else if (AppConfig::kHeaterCommissioningMode)
            commission_.emergencyStop(HeaterCommissioning::Fault::PulseWatchdog);
          faultActive_ = true;
          forceHeaterOff();
          return;
        }
        timerWrite(pulseTimer_, 0);
        timerRestart(pulseTimer_); // hardware ceiling before GPIO HIGH
      }
#endif
      // Applies only after all mode-specific gates and watchdog setup.
      digitalWrite(AppConfig::kPinHeaterPwm, HIGH);
      heaterOn_ = true;
      everHeated_ = true;
      lastHeaterOnMs_ = lastNowMs_;
    } else {
      forceHeaterOff();
    }
  }
};
