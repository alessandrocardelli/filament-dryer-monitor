#pragma once
#include <Arduino.h>
#include "AppConfig.h"

enum class ButtonId : uint8_t { OnOff, Mode, Up, Down };
enum class ButtonEventType : uint8_t { ShortPress, LongPress, Repeat };
using ButtonEventCallback = void (*)(ButtonId, ButtonEventType);

class ButtonManager {
 public:
  void begin() {
    initButton(0, ButtonId::OnOff, AppConfig::kPinButtonOnOff, false);
    initButton(1, ButtonId::Mode, AppConfig::kPinButtonMode, false);
    initButton(2, ButtonId::Up, AppConfig::kPinButtonUp, true);
    initButton(3, ButtonId::Down, AppConfig::kPinButtonDown, true);
  }

  void update(uint32_t nowMs, ButtonEventCallback callback) {
    for (auto &b : buttons_) {
      const bool rawPressed = digitalRead(b.pin) == LOW;

      if (rawPressed != b.rawPressed) {
        b.rawPressed = rawPressed;
        b.lastRawChangeMs = nowMs;
      }

      if ((nowMs - b.lastRawChangeMs) >= AppConfig::kButtonDebounceMs &&
          b.stablePressed != b.rawPressed) {
        b.stablePressed = b.rawPressed;
        if (b.stablePressed) {
          b.pressStartedMs = nowMs;
          b.longSent = false;
          b.lastRepeatMs = nowMs;
        } else {
          if (!b.longSent && !b.repeatStarted && callback) {
            callback(b.id, ButtonEventType::ShortPress);
          }
          b.repeatStarted = false;
        }
      }

      if (!b.stablePressed) continue;

      const uint32_t heldMs = nowMs - b.pressStartedMs;
      if (b.repeatEnabled) {
        if (heldMs >= AppConfig::kButtonRepeatDelayMs &&
            (!b.repeatStarted || (nowMs - b.lastRepeatMs) >= AppConfig::kButtonRepeatPeriodMs)) {
          b.repeatStarted = true;
          b.lastRepeatMs = nowMs;
          if (callback) callback(b.id, ButtonEventType::Repeat);
        }
      } else if (!b.longSent && heldMs >= AppConfig::kButtonLongPressMs) {
        b.longSent = true;
        if (callback) callback(b.id, ButtonEventType::LongPress);
      }
    }
  }

 private:
  struct ButtonState {
    ButtonId id = ButtonId::OnOff;
    uint8_t pin = 0;
    bool repeatEnabled = false;
    bool rawPressed = false;
    bool stablePressed = false;
    bool longSent = false;
    bool repeatStarted = false;
    uint32_t lastRawChangeMs = 0;
    uint32_t pressStartedMs = 0;
    uint32_t lastRepeatMs = 0;
  };

  ButtonState buttons_[4];

  void initButton(uint8_t index, ButtonId id, uint8_t pin, bool repeatEnabled) {
    buttons_[index].id = id;
    buttons_[index].pin = pin;
    buttons_[index].repeatEnabled = repeatEnabled;
    // GPIO35 has no internal pull-up on ESP32; the released PCB already provides pull-ups.
    pinMode(pin, INPUT);
  }
};
