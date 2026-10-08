#pragma once
#include <Arduino.h>
#include "AppConfig.h"
#include "AppTypes.h"

class Buzzer {
 public:
  void begin() {
    attached_ = ledcAttachChannel(AppConfig::kPinBuzzer, 2000,
                                  AppConfig::kBuzzerResolutionBits,
                                  AppConfig::kBuzzerLedcChannel);
    stopTone();
  }

  void setVolume(BuzzerVolume volume) {
    volume_ = volume;
    if (volume_ == BuzzerVolume::Off) stop();
  }

  BuzzerVolume volume() const { return volume_; }

  void update(uint32_t nowMs) {
    if (!playing_ || sequence_ == nullptr || sequenceLength_ == 0) return;
    if (static_cast<int32_t>(nowMs - stepEndsMs_) < 0) return;

    ++stepIndex_;
    if (stepIndex_ >= sequenceLength_) {
      if (repeat_) {
        stepIndex_ = 0;
        applyStep(nowMs);
      } else {
        playing_ = false;
        stopTone();
      }
      return;
    }
    applyStep(nowMs);
  }

  void playStartup(uint32_t nowMs) {
    startSequence(kStartupChime, countOf(kStartupChime), nowMs, false);
  }

  void playKeyClick(uint32_t nowMs) {
    startSequence(kKeyClick, countOf(kKeyClick), nowMs, false);
  }

  void playCycleStart(uint32_t nowMs) {
    startSequence(kStartBeep, countOf(kStartBeep), nowMs, false);
  }

  void playComplete(uint32_t nowMs) {
    startSequence(kCompleteChime, countOf(kCompleteChime), nowMs, false);
  }

  void playWarning(uint32_t nowMs) {
    startSequence(kWarning, countOf(kWarning), nowMs, false);
  }

  void startFaultAlarm(uint32_t nowMs) {
    startSequence(kFaultAlarm, countOf(kFaultAlarm), nowMs, true);
  }

  void muteFault() { stop(); }

  void playVolumePreview(uint32_t nowMs) {
    startSequence(kVolumePreview, countOf(kVolumePreview), nowMs, false);
  }

  void stop() {
    playing_ = false;
    repeat_ = false;
    stopTone();
  }

 private:
  struct ToneStep {
    uint16_t frequencyHz;
    uint16_t durationMs;
  };

  static constexpr ToneStep kStartupChime[] = {
      {1047, 90}, {0, 35}, {1568, 210},
  };
  static constexpr ToneStep kKeyClick[] = {
      {1800, 22},
  };
  static constexpr ToneStep kStartBeep[] = {
      {1400, 80},
  };
  static constexpr ToneStep kCompleteChime[] = {
      {880, 90}, {0, 45}, {1175, 90}, {0, 45}, {1568, 180},
  };
  static constexpr ToneStep kWarning[] = {
      {1200, 100}, {0, 80}, {1200, 100},
  };
  static constexpr ToneStep kFaultAlarm[] = {
      {900, 140}, {0, 90}, {900, 140}, {0, 900},
  };
  static constexpr ToneStep kVolumePreview[] = {
      {1350, 110},
  };

  template <size_t N>
  static constexpr uint8_t countOf(const ToneStep (&)[N]) {
    return static_cast<uint8_t>(N);
  }

  const ToneStep *sequence_ = nullptr;
  uint8_t sequenceLength_ = 0;
  uint8_t stepIndex_ = 0;
  uint32_t stepEndsMs_ = 0;
  bool attached_ = false;
  bool playing_ = false;
  bool repeat_ = false;
  BuzzerVolume volume_ = BuzzerVolume::Low;

  uint8_t dutyForVolume() const {
    switch (volume_) {
      case BuzzerVolume::Off: return 0;
      case BuzzerVolume::Low: return AppConfig::kBuzzerDutyLow;
      case BuzzerVolume::Medium: return AppConfig::kBuzzerDutyMedium;
      case BuzzerVolume::High: return AppConfig::kBuzzerDutyHigh;
    }
    return AppConfig::kBuzzerDutyLow;
  }

  void startSequence(const ToneStep *sequence, uint8_t length,
                     uint32_t nowMs, bool repeat) {
    if (!attached_ || sequence == nullptr || length == 0 ||
        volume_ == BuzzerVolume::Off) {
      return;
    }
    sequence_ = sequence;
    sequenceLength_ = length;
    stepIndex_ = 0;
    repeat_ = repeat;
    playing_ = true;
    applyStep(nowMs);
  }

  void applyStep(uint32_t nowMs) {
    const ToneStep &step = sequence_[stepIndex_];
    if (step.frequencyHz == 0) {
      stopTone();
    } else {
      ledcChangeFrequency(AppConfig::kPinBuzzer, step.frequencyHz,
                          AppConfig::kBuzzerResolutionBits);
      ledcWrite(AppConfig::kPinBuzzer, dutyForVolume());
    }
    stepEndsMs_ = nowMs + step.durationMs;
  }

  void stopTone() {
    if (attached_) ledcWrite(AppConfig::kPinBuzzer, 0);
  }
};
