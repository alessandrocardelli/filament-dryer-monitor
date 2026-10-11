#pragma once
#include <Arduino.h>
#include "AppConfig.h"
#include "AppTypes.h"

class SafetyController {
 public:
  void begin() {
    pinMode(AppConfig::kPinHeaterPwm, OUTPUT);
    forceHeaterOff();
  }

  void update(const NtcSnapshot &ntc) {
    forceHeaterOff();

    // First real safety input: obvious electrical NTC faults. This does not
    // define any final thermal trip point; heater enable remains unavailable.
    if (ntc.sampled && ntc.fault != NtcFault::None) {
      faultActive_ = true;
      ntcFault_ = ntc.fault;
    }
  }

  bool heaterPermitted() const { return false; }
  bool heaterLocked() const { return true; }
  bool fanPermitted() const { return !faultActive_; }
  bool faultActive() const { return faultActive_; }
  NtcFault ntcFault() const { return ntcFault_; }

 private:
  bool faultActive_ = false;
  NtcFault ntcFault_ = NtcFault::None;

  void forceHeaterOff() {
    digitalWrite(AppConfig::kPinHeaterPwm, LOW);
  }
};
