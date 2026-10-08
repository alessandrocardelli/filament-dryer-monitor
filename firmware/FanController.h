#pragma once
#include <Arduino.h>
#include "AppConfig.h"

enum class FanPhase : uint8_t {
  Off,
  Kickstart,
  Running,
};

class FanController {
 public:
  void begin() {
    pinMode(AppConfig::kPinFanPwm, OUTPUT);
    digitalWrite(AppConfig::kPinFanPwm, LOW);

    attached_ = ledcAttachChannel(AppConfig::kPinFanPwm,
                                  AppConfig::kFanPwmFrequencyHz,
                                  AppConfig::kFanPwmResolutionBits,
                                  AppConfig::kFanLedcChannel);
    stop();
  }

  void startAuto(uint32_t nowMs) {
    if (!attached_) return;
    phase_ = FanPhase::Kickstart;
    kickstartEndsMs_ = nowMs + AppConfig::kFanKickstartMs;
    writePercent(AppConfig::kFanKickstartPercent);
  }

  void update(uint32_t nowMs, bool permitted) {
    if (!permitted) {
      stop();
      return;
    }

    if (phase_ == FanPhase::Kickstart &&
        static_cast<int32_t>(nowMs - kickstartEndsMs_) >= 0) {
      phase_ = FanPhase::Running;
      writePercent(AppConfig::kFanBringupRunPercent);
    }
  }

  void stop() {
    phase_ = FanPhase::Off;
    dutyPercent_ = 0;
    if (attached_) {
      ledcWrite(AppConfig::kPinFanPwm, 0);
    } else {
      digitalWrite(AppConfig::kPinFanPwm, LOW);
    }
  }

  bool attached() const { return attached_; }
  FanPhase phase() const { return phase_; }
  uint8_t dutyPercent() const { return dutyPercent_; }

 private:
  bool attached_ = false;
  FanPhase phase_ = FanPhase::Off;
  uint8_t dutyPercent_ = 0;
  uint32_t kickstartEndsMs_ = 0;

  static uint32_t percentToDuty(uint8_t percent) {
    const uint32_t maxDuty = (1UL << AppConfig::kFanPwmResolutionBits) - 1UL;
    return (static_cast<uint32_t>(percent) * maxDuty + 50UL) / 100UL;
  }

  void writePercent(uint8_t percent) {
    dutyPercent_ = constrain(percent, 0, 100);
    ledcWrite(AppConfig::kPinFanPwm, percentToDuty(dutyPercent_));
  }
};
