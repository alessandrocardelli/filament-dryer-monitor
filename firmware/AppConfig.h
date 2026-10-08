#pragma once
#include <Arduino.h>

namespace AppConfig {

constexpr uint8_t kPinSda = 21;
constexpr uint8_t kPinScl = 22;
constexpr uint8_t kPinStatusLed = 26;
constexpr uint8_t kPinFanPwm = 16;
constexpr uint8_t kPinBuzzer = 33;
constexpr uint8_t kPinHeaterPwm = 19;
constexpr uint8_t kPinNtcAdc = 34;
constexpr uint8_t kPinButtonOnOff = 35;
constexpr uint8_t kPinButtonMode = 32;
constexpr uint8_t kPinButtonUp = 14;
constexpr uint8_t kPinButtonDown = 27;

constexpr uint8_t kOledAddress = 0x3C;
constexpr uint8_t kSht45Address = 0x44;
constexpr uint32_t kI2cFrequencyHz = 100000;

constexpr uint32_t kBootSplashMs = 1500;
constexpr uint32_t kDisplaySleepMs = 60000;
constexpr uint32_t kRenderPeriodMs = 100;
constexpr uint32_t kButtonDebounceMs = 30;
constexpr uint32_t kButtonLongPressMs = 1500;
constexpr uint32_t kButtonRepeatDelayMs = 500;
constexpr uint32_t kButtonRepeatPeriodMs = 120;
constexpr uint32_t kSht45PeriodMs = 2000;
constexpr uint32_t kSht45ConversionMs = 12;
constexpr uint32_t kSht45StaleMs = 10000;

// SAFE BRING-UP firmware: actuator outputs are intentionally hard-disabled.
// These UI-only bounds exist solely to exercise the setup screen and buttons.
// They are NOT heater safety limits or accepted product settings.
constexpr int16_t kPrototypeTempMinC = 30;
constexpr int16_t kPrototypeTempMaxC = 80;
constexpr int16_t kPrototypeTempStepC = 1;
constexpr uint16_t kPrototypeDurationMinMinutes = 15;
constexpr uint16_t kPrototypeDurationMaxMinutes = 12 * 60;
constexpr uint16_t kPrototypeDurationStepMinutes = 15;

constexpr uint8_t kBuzzerResolutionBits = 8;
constexpr uint8_t kBuzzerLedcChannel = 8; // ESP32 group 1; keep independent from future fan PWM.
constexpr uint8_t kBuzzerDutyLow = 18;    // ~7% of 8-bit full scale; tune on the first article.

}  // namespace AppConfig
