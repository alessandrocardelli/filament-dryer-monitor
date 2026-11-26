#pragma once
#include <Arduino.h>
#include <math.h>
#include "AppConfig.h"
#include "AppTypes.h"
#include "VishayNtcCurve.h"
#include "NtcReadoutCalibration.h"

class NtcSensor {
 public:
  void begin(uint32_t nowMs) {
    pinMode(AppConfig::kPinNtcAdc, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(AppConfig::kPinNtcAdc, ADC_11db);
    sample(nowMs);
  }

  void update(uint32_t nowMs) {
    if ((nowMs - lastSampleMs_) < AppConfig::kNtcSamplePeriodMs) return;
    sample(nowMs);
  }

  const NtcSnapshot &snapshot() const { return snapshot_; }
  bool healthy() const { return snapshot_.sampled && snapshot_.valid; }

 private:
  NtcSnapshot snapshot_{};
  uint32_t lastSampleMs_ = 0;

  static float correctedMillivolts(float reportedMv) {
    return AppConfig::kNtcAdcCorrectionSlope * reportedMv +
           AppConfig::kNtcAdcCorrectionOffsetMv;
  }

  static float resistanceFromMillivolts(float nodeMv) {
    return NtcReadoutCalibration::resistanceOhms(
        nodeMv, AppConfig::kNtcDividerSupplyMv, AppConfig::kNtcEffectivePullupOhms);
  }

  void sample(uint32_t nowMs) {
    uint32_t mvSum = 0;
    for (uint8_t i = 0; i < AppConfig::kNtcSamplesPerReading; ++i) {
      mvSum += analogReadMilliVolts(AppConfig::kPinNtcAdc);
    }

    const float reportedMv =
        static_cast<float>(mvSum) / AppConfig::kNtcSamplesPerReading;
    const float correctedMv = correctedMillivolts(reportedMv);

    snapshot_ = {};
    snapshot_.sampled = true;
    snapshot_.reportedMv = reportedMv;
    // Legacy calibrated voltage retained for diagnostics and unchanged fault windows.
    snapshot_.correctedMv = correctedMv;
    snapshot_.lastSampleMs = nowMs;
    lastSampleMs_ = nowMs;

    // Bring-up electrical fault windows only; these do NOT qualify any
    // local heater temperature or final thermal protection thresholds.
    // The separate commissioning mode can pulse the heater under its own gates.
    if (!isfinite(correctedMv)) {
      snapshot_.fault = NtcFault::InvalidReading;
      return;
    }
    if (correctedMv <= AppConfig::kNtcShortThresholdMv) {
      snapshot_.fault = NtcFault::ShortCircuit;
      return;
    }
    if (correctedMv >= AppConfig::kNtcOpenThresholdMv) {
      snapshot_.fault = NtcFault::OpenCircuit;
      return;
    }

    const float resistanceOhms = resistanceFromMillivolts(correctedMv);
    if (!isfinite(resistanceOhms) || resistanceOhms <= 0.0f) {
      snapshot_.fault = NtcFault::InvalidReading;
      return;
    }

    const float temperatureC=VishayNtcCurve::temperatureC(resistanceOhms);
    const bool insideEmpiricalRange=isfinite(temperatureC);
    if (!isfinite(temperatureC)) {
      snapshot_.fault = NtcFault::InvalidReading;
      return;
    }

    snapshot_.resistanceOhms = resistanceOhms;
    snapshot_.temperatureC = temperatureC;
    snapshot_.insideEmpiricalRange = insideEmpiricalRange;
    snapshot_.fault = NtcFault::None;
    snapshot_.valid = true;
  }
};
