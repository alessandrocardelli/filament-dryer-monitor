#pragma once
#include <Arduino.h>
#include "AppTypes.h"

enum class CycleEvent : uint8_t {
  None,
  Completed,
};

class CycleController {
 public:
  void start(const CycleSettings &settings, uint32_t nowMs) {
    activeSettings_ = settings;
    startedMs_ = nowMs;
    running_ = true;
    completionReported_ = false;
  }

  void stop() {
    running_ = false;
    completionReported_ = false;
  }

  CycleEvent update(uint32_t nowMs) {
    if (!running_) return CycleEvent::None;
    if (remainingSeconds(nowMs) > 0) return CycleEvent::None;

    running_ = false;
    if (!completionReported_) {
      completionReported_ = true;
      return CycleEvent::Completed;
    }
    return CycleEvent::None;
  }

  bool isRunning() const { return running_; }

  uint32_t remainingSeconds(uint32_t nowMs) const {
    if (!running_) return 0;
    const uint32_t totalSeconds =
        static_cast<uint32_t>(activeSettings_.durationMinutes) * 60UL;
    const uint32_t elapsedSeconds = (nowMs - startedMs_) / 1000UL;
    return elapsedSeconds >= totalSeconds ? 0 : totalSeconds - elapsedSeconds;
  }

 private:
  CycleSettings activeSettings_{};
  uint32_t startedMs_ = 0;
  bool running_ = false;
  bool completionReported_ = false;
};
