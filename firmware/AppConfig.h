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

// Keep all three phases at their original pace; extend the opening still frame.
// 1200 ms static + 1200 ms growth + 350 ms full emblem + 1400 ms wordmark.
constexpr uint32_t kBootSplashMs = 4150;
constexpr uint32_t kBootRootHoldMs = 1200;
constexpr uint32_t kBootWordmarkStartMs = 2750;
constexpr uint32_t kDisplaySleepMs = 60000;
constexpr uint32_t kRenderPeriodMs = 100;
constexpr uint32_t kButtonDebounceMs = 30;
constexpr uint32_t kButtonLongPressMs = 1500;
constexpr uint32_t kButtonRepeatDelayMs = 500;
constexpr uint32_t kButtonRepeatPeriodMs = 120;
constexpr uint32_t kSht45PeriodMs = 2000;
constexpr uint32_t kSht45ConversionMs = 12;
constexpr uint32_t kSht45StaleMs = 10000;

// Heater NTC first-article ADC calibration, measured 2026-10-11.
// analogReadMilliVolts() was compared with a DMM using 33.03k, 47.0k,
// 68.0k, 81.9k and 99.9k substitutes on J7. The linear correction maps
// ESP32-reported mV to the DMM node voltage over the characterized range.
constexpr uint32_t kNtcSamplePeriodMs = 500;
constexpr uint8_t kNtcSamplesPerReading = 16;
constexpr float kNtcAdcCorrectionSlope = 0.984369f;
constexpr float kNtcAdcCorrectionOffsetMv = -2.322f;
// Effective divider parameters fitted to those same first-article points.
// They include the actual R28 tolerance and measured 3V3 rail behavior.
constexpr float kNtcDividerSupplyMv = 3311.0f;
constexpr float kNtcEffectivePullupOhms = 46572.0f;
// Electrical bring-up fault windows only; final thermal trip thresholds remain TBD.
constexpr float kNtcShortThresholdMv = 150.0f;
constexpr float kNtcOpenThresholdMv = 3000.0f;


// EXPERIMENTAL STEP07-G: supervised early raw-NTC shutdown trial.
// Six 20s windows are a MAXIMUM; abort if R_NTC <= 40kΩ.
// Fan100; NOT autonomous air regulation or a qualified NTC model.
// Do not merge this experiment into main or the operational COLD branch.
constexpr bool kHeaterCommissioningMode = false;
// STEP09 isolated HEATER-CAPABLE supervised-feedback candidate: NOT production
// and NOT approved for flashing until the review/gate in docs/STEP09.
// Attended heater characterization: explicit HEATER-capable experiment.
// Host-verified interlocks still mandatory, no unattended production approval.
constexpr bool kHeaterStep09TrialMode = true;
static_assert(!(kHeaterCommissioningMode && kHeaterStep09TrialMode),
              "Only one supervised heater trial mode can run");
// STEP05 candidate heater profile is NEVER implicitly approved.
// Dedicated heater-enabled trial: explicit two review flags and candidate limits.
constexpr bool kHeaterStage05TrialReviewed = true;
// Historical STEP06 approval flags gate the pulse code path only.
// They do NOT constitute user approval of a new STEP07-G physical run.
// No new physical test before independent user review and preflight.
constexpr bool kHeaterStep06TrialReviewed = true;
static_assert(!kHeaterCommissioningMode ||
                  (kHeaterStage05TrialReviewed && kHeaterStep06TrialReviewed),
              "STEP06 heating requires independent physical-trial approval");
// The normal closed-loop heater regulator remains DISABLED.
// Test-only synthetic profiles in host fixtures are NOT hardware approvals.
// Real thermal limits, NTC protection and physical safeguards remain OPEN.
//
// Neither the old donor 82.5k / B4100 conversion nor the seven Sanwa contact
// points qualify as a calibrated heater-bead thermometer. Never switch these
// booleans on simply to make a build heat.
constexpr bool kHeaterOutputEnabled = false;
static_assert(!(kHeaterCommissioningMode || kHeaterStep09TrialMode) ||
                  !kHeaterOutputEnabled,
              "Do not combine an experimental trial and production thermostat");
constexpr bool kHeaterProfileApproved = false;
constexpr bool kHeaterNtcModelApproved = false;

// Provisional POST-HEAT fan-overrun ceiling for development and host tests.
// This is a minimum candidate, NOT validation that the heater is safe after
// 45 seconds: actual post-OFF Sanwa/NTC traces will determine the V1 rule.
constexpr uint32_t kPostHeatCooldownMs = 45000;

// Zeros intentionally mean UNCONFIGURED, not temperature recommendations.
// Values must be independently set/reviewed for actual board/materials.
constexpr float kHeaterMaxMeasuredC = 0.0f; // hard local thermal fault, UNQUALIFIED
constexpr float kHeaterSoftPauseMeasuredC = 0.0f; // local soft pause, UNQUALIFIED
constexpr float kHeaterRearmMeasuredC = 0.0f; // local rearm, UNQUALIFIED
constexpr float kChamberMaxMeasuredC = 0.0f;
constexpr float kChamberControlHysteresisC = 0.0f;
constexpr uint32_t kHeaterNtcMaxAgeMs = 0;
constexpr uint32_t kHeaterChamberMaxAgeMs = 0;
constexpr uint32_t kHeaterFanPrerunMs = 0;
constexpr uint32_t kHeaterMaxContinuousOnMs = 0; // per-window hard bound; <=20000ms
constexpr uint32_t kHeaterMinimumOffMs = 0; // per-window dead time; UNQUALIFIED
constexpr uint32_t kHeaterMaxCycleMs = 0;

// Prevent accidental heater ON by toggling a single flag.
// These are compile-time guards, while the policy has additional runtime ones.
static_assert(!kHeaterOutputEnabled ||
              (kHeaterProfileApproved && kHeaterNtcModelApproved &&
               kHeaterMaxMeasuredC > 0.0f &&
               kHeaterSoftPauseMeasuredC > kHeaterRearmMeasuredC &&
               kHeaterMaxMeasuredC > kHeaterSoftPauseMeasuredC &&
               kChamberMaxMeasuredC > 0.0f &&
               kChamberControlHysteresisC > 0.0f &&
               kHeaterNtcMaxAgeMs > 0 &&
               kHeaterChamberMaxAgeMs > 0 &&
               kHeaterFanPrerunMs > 0 &&
               kHeaterMaxContinuousOnMs > 0 &&
               kHeaterMaxContinuousOnMs <= 20000 &&
               kHeaterMinimumOffMs > 0 &&
               kHeaterMaxCycleMs > 0),
              "Heater cannot be enabled without reviewed calibration, limits and timers.");

// Legacy UI setup temperature bounds are NOT heater safety limits.
// These UI bounds are provisional and are NOT heater safety limits.
constexpr int16_t kPrototypeTempMinC = 40;
constexpr int16_t kPrototypeTempMaxC = 55;
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
// STEP06B experiment ONLY: maximize available commanded airflow while
// characterizing heater-to-air response. D025's 40% is historical safe bring-up,
// NOT a proven best duty for the dryer.
constexpr uint8_t kFanBringupRunPercent = 100;

constexpr uint8_t kBuzzerResolutionBits = 8;
constexpr uint8_t kBuzzerLedcChannel = 8;
constexpr uint8_t kBuzzerDutyLow = 12;
constexpr uint8_t kBuzzerDutyMedium = 24;
constexpr uint8_t kBuzzerDutyHigh = 48;

constexpr uint16_t kSettingsSchemaVersion = 1;

}  // namespace AppConfig
