#pragma once
#include <Arduino.h>
#include "AppConfig.h"

class Buzzer {
 public:
  void begin() {
    attached_ = ledcAttachChannel(AppConfig::kPinBuzzer, 2000,
                                  AppConfig::kBuzzerResolutionBits,
                                  AppConfig::kBuzzerLedcChannel);
    stopTone();
  }

  void update(uint32_t nowMs) {
    if (!playing_ || sequence_ == nullptr || sequenceLength_ == 0) return;
    if ((int32_t)(nowMs - stepEndsMs_) < 0) return;

    ++stepIndex_;
    if (stepIndex_ >= sequenceLength_) {
      playing_ = false;
      stopTone();
      return;
    }
    applyStep(nowMs);
  }

  void playStartup(uint32_t nowMs) { startSequence(kStartupChime, countOf(kStartupChime), nowMs); }
  void playCycleStart(uint32_t nowMs) { startSequence(kStartBeep, countOf(kStartBeep), nowMs); }
  void playComplete(uint32_t nowMs) { startSequence(kCompleteChime, countOf(kCompleteChime), nowMs); }

  void stop() {
    playing_ = false;
    stopTone();
  }

 private:
  struct ToneStep {
    uint16_t frequencyHz;
    uint16_t durationMs;
  };

  // Original short rising power-on chime: intentionally evokes a 1990s handheld
  // startup feel without reproducing the Game Boy startup sound note-for-note.
  static constexpr ToneStep kStartupChime[] = {
      {1047, 90}, {0, 35}, {1568, 210},
  };
  static constexpr ToneStep kStartBeep[] = {
      {1400, 80},
  };
  static constexpr ToneStep kCompleteChime[] = {
      {880, 90}, {0, 45}, {1175, 90}, {0, 45}, {1568, 180},
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

  void startSequence(const ToneStep *sequence, uint8_t length, uint32_t nowMs) {
    if (!attached_ || sequence == nullptr || length == 0) return;
    sequence_ = sequence;
    sequenceLength_ = length;
    stepIndex_ = 0;
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
      ledcWrite(AppConfig::kPinBuzzer, AppConfig::kBuzzerDutyLow);
    }
    stepEndsMs_ = nowMs + step.durationMs;
  }

  void stopTone() {
    if (attached_) ledcWrite(AppConfig::kPinBuzzer, 0);
  }
};
