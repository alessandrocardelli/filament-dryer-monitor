#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "AppConfig.h"
#include "AppTypes.h"
#include "HeaterControlPolicy.h"
#include "HeaterPulseCommissioning.h"
#include "HeaterStep07GuardTrial.h"
#ifdef ARDUINO_ARCH_ESP32
#include "driver/gpio.h"
#endif

// Integrates the real GPIO19 output with the host-tested control policy.
// STEP07 experimental branch: only one-shot supervised heater pulses are
// enabled; the normal thermostatic path is explicitly DISABLED.
// Sensor/thermal qualification and independent physical safeguards are
// separate from the optional added TCO (D029).
class SafetyController {
 public:
  void begin() {
    // GPIO is driven LOW before any other peripheral is initialized.
    pinMode(AppConfig::kPinHeaterPwm, OUTPUT);
    forceHeaterOff();
#ifdef ARDUINO_ARCH_ESP32
    if (AppConfig::kHeaterCommissioningMode) {
      // ESP32 hardware timer cuts the pulse off even if loop() stalls.
      pulseTimer_ = timerBegin(1000000); // one tick per microsecond
      if (!pulseTimer_) {
        faultActive_ = true;
        return; // never heat without the independent pulse deadline
      }
      timerAttachInterrupt(pulseTimer_, &SafetyController::pulseTimerIsr);
      timerAlarm(pulseTimer_, HeaterStep07GuardTrial::kPulseHardwareCeilingUs,
                 false, 0); // bounded 10.5 s; never continuous
      timerStop(pulseTimer_);
    }
#endif
  }

  // Synchronous immediate shutdown for STOP / COMPLETE / FAULT / BOOT.
  void stop() {
    forceHeaterOff();
    commission_.cancel();
    HeaterControl::Inputs stopped{};
    policy_.evaluate(0, HeaterControl::Limits{}, stopped);
  }

  void update(uint32_t nowMs, const NtcSnapshot &ntc,
              const SensorSnapshot &chamber, bool cycleActive,
              bool fanPwmRunning, uint32_t elapsedCycleMs,
              float targetChamberC) {
    lastNowMs_ = nowMs;
    // Remember the last known physical ON while the loop remains responsive.
    // stop()/FAULT must not erase residual plate heat and fan-overrun needs.
    if (heaterOn_) lastHeaterOnMs_ = nowMs;
#ifdef ARDUINO_ARCH_ESP32
    if (AppConfig::kHeaterCommissioningMode && pulseTimerFired_) {
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

    if (!AppConfig::kHeaterOutputEnabled || !AppConfig::kHeaterProfileApproved ||
        !AppConfig::kHeaterNtcModelApproved) {
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
  bool commissioningComplete() const {
    return AppConfig::kHeaterCommissioningMode && commission_.finished();
  }
  bool commissioningCanStart() const { return commission_.canStart(); }
  bool coolingRequired(uint32_t nowMs) const {
    const bool commissioningCooldown =
        AppConfig::kHeaterCommissioningMode && commission_.cooldownNeeded(nowMs);
    const bool recentPhysicalHeating = everHeated_ &&
        static_cast<uint32_t>(nowMs - lastHeaterOnMs_) <
            AppConfig::kPostHeatCooldownMs;
    return commissioningCooldown || recentPhysicalHeating;
  }
  uint8_t commissioningPhase() const {
    return static_cast<uint8_t>(commission_.phase());
  }
  uint8_t commissioningPulsesDone() const { return commission_.pulsesDone(); }
  uint8_t commissioningFault() const {
    return static_cast<uint8_t>(commission_.fault());
  }
  bool heaterLocked() const {
    if (AppConfig::kHeaterCommissioningMode) return false;
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
  HeaterControl::Policy policy_{};
  HeaterCommissioning::Sequence commission_{HeaterStep07GuardTrial::parameters()};
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
        (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterOutputEnabled) &&
        requested;
    if (heaterOn_ == desired) return;
    if (desired) {
#ifdef ARDUINO_ARCH_ESP32
      if (AppConfig::kHeaterCommissioningMode) {
        if (!pulseTimer_ || pulseTimerFired_) {
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
