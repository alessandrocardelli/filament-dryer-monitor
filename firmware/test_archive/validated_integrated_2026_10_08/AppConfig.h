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

constexpr uint32_t kBootSplashMs = 3500;
constexpr uint32_t kDisplaySleepMs = 60000;
constexpr uint32_t kRenderPeriodMs = 100;
constexpr uint32_t kButtonDebounceMs = 30;
constexpr uint32_t kButtonLongPressMs = 1500;
constexpr uint32_t kButtonRepeatDelayMs = 500;
constexpr uint32_t kButtonRepeatPeriodMs = 120;
constexpr uint32_t kSht45PeriodMs = 2000;
constexpr uint32_t kSht45ConversionMs = 12;
constexpr uint32_t kSht45StaleMs = 10000;

// SAFE BRING-UP: heater remains hard-disabled.
// These UI bounds are provisional and are NOT heater safety limits.
constexpr int16_t kPrototypeTempMinC = 30;
constexpr int16_t kPrototypeTempMaxC = 80;
constexpr int16_t kPrototypeTempStepC = 1;
constexpr uint16_t kPrototypeDurationMinMinutes = 15;
constexpr uint16_t kPrototypeDurationMaxMinutes = 12 * 60;
constexpr uint16_t kPrototypeDurationStepMinutes = 15;

// Fan first-article bring-up policy.
// Measured at 25 kHz: 25% first tested start duty, 15% sustained-running floor.
constexpr uint32_t kFanPwmFrequencyHz = 25000;
constexpr uint8_t kFanPwmResolutionBits = 8;
constexpr uint8_t kFanLedcChannel = 9;
constexpr uint32_t kFanKickstartMs = 1000;
constexpr uint8_t kFanKickstartPercent = 100;
constexpr uint8_t kFanBringupRunPercent = 40;

constexpr uint8_t kBuzzerResolutionBits = 8;
constexpr uint8_t kBuzzerLedcChannel = 8;
constexpr uint8_t kBuzzerDutyLow = 12;
constexpr uint8_t kBuzzerDutyMedium = 24;
constexpr uint8_t kBuzzerDutyHigh = 48;

constexpr uint16_t kSettingsSchemaVersion = 1;

}  // namespace AppConfig
