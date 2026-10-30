#pragma once
#include <cstdint>

// Operator-only Sanwa RD701/K measurement prompts.
// No Arduino APIs, GPIO, sensor data, safety or heater policy dependencies.
// All offsets are relative to a START event or the final STOP/COMPLETE event.
// Fault immediately cancels the schedule; no reminders during faults.
// The Sanwa instrument is EXTERNAL and does not stop the HEATER.
class SanwaReminder {
 public:
  enum class Stage : uint8_t { Inactive, Heating, AfterStop };

  void start(uint32_t nowMs) {
    stage_ = Stage::Heating;
    originMs_ = nowMs;
    nextOffsetMs_ = 60000U;  // minute 1
  }
  void stop(uint32_t nowMs) {
    if (stage_ != Stage::Heating) return;
    stage_ = Stage::AfterStop;
    originMs_ = nowMs;
    nextOffsetMs_ = 30000U;  // +30, +60, +90, +120 seconds
  }
  void cancel() {
    stage_ = Stage::Inactive;
    nextOffsetMs_ = 0;
  }
  Stage stage() const { return stage_; }
  bool active() const { return stage_ != Stage::Inactive; }
  bool due(uint32_t nowMs) const {
    return active() && elapsed(nowMs) >= nextOffsetMs_;
  }
  // Ceil to seconds, to count 00:01 at 1..1000ms remaining.
  uint32_t secondsUntilNext(uint32_t nowMs) const {
    if (!active()) return 0;
    const uint32_t used = elapsed(nowMs);
    if (used >= nextOffsetMs_) return 0;
    return (nextOffsetMs_ - used + 999U) / 1000U;
  }
  // Consume only after an audible prompt was accepted, or sound is OFF.
  // Skip old reminders after loop delays: never emit a backlog of beeps.
  void consume(uint32_t nowMs) {
    if (!due(nowMs)) return;
    const uint32_t used = elapsed(nowMs);
    if (stage_ == Stage::Heating) {
      do {
        nextOffsetMs_ += (nextOffsetMs_ < 600000U) ? 60000U : 120000U;
      } while (used >= nextOffsetMs_);
      return;
    }
    do {
      nextOffsetMs_ += 30000U;
    } while (nextOffsetMs_ <= 120000U && used >= nextOffsetMs_);
    if (nextOffsetMs_ > 120000U) cancel();
  }

 private:
  uint32_t elapsed(uint32_t nowMs) const { return nowMs - originMs_; }
  Stage stage_ = Stage::Inactive;
  uint32_t originMs_ = 0;
  uint32_t nextOffsetMs_ = 0;
};
