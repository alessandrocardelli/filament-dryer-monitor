#pragma once
#include <Arduino.h>
#include <math.h>
#include "AppConfig.h"
#include "AppTypes.h"

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
    const float denominator = AppConfig::kNtcDividerSupplyMv - nodeMv;
    if (denominator <= 0.0f) return NAN;
    return AppConfig::kNtcEffectivePullupOhms * nodeMv / denominator;
  }

  static float betaTemperatureC(float resistanceOhms) {
    constexpr float kReferenceTempK = 25.0f + 273.15f;
    constexpr float kReferenceResistanceOhms = 82500.0f;
    constexpr float kBetaK = 4100.0f;

    const float invT =
        (1.0f / kReferenceTempK) +
        logf(resistanceOhms / kReferenceResistanceOhms) / kBetaK;
    if (!(invT > 0.0f)) return NAN;
    return (1.0f / invT) - 273.15f;
  }

  static float empiricalTemperatureC(float resistanceOhms,
                                     bool &insideEmpiricalRange) {
    // Original eSUN heater NTC characterization from README.md.
    constexpr float kTempC[] = {25.0f, 28.0f, 30.0f, 35.0f,
                                40.0f, 44.0f, 50.0f};
    constexpr float kResistanceOhms[] = {82500.0f, 75500.0f, 67400.0f,
                                         51700.0f, 43600.0f, 35930.0f,
                                         28800.0f};
    constexpr size_t kCount = sizeof(kTempC) / sizeof(kTempC[0]);

    insideEmpiricalRange = false;
    for (size_t i = 0; i + 1 < kCount; ++i) {
      if (resistanceOhms <= kResistanceOhms[i] &&
          resistanceOhms >= kResistanceOhms[i + 1]) {
        const float fraction =
            (kResistanceOhms[i] - resistanceOhms) /
            (kResistanceOhms[i] - kResistanceOhms[i + 1]);
        insideEmpiricalRange = true;
        return kTempC[i] + fraction * (kTempC[i + 1] - kTempC[i]);
      }
    }

    // Outside the measured 25-50 C interval, provide a service/diagnostic
    // estimate from the previously recorded beta~=4100 K model. This
    // extrapolation is not a validated heater safety limit.
    return betaTemperatureC(resistanceOhms);
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
    snapshot_.correctedMv = correctedMv;
    snapshot_.lastSampleMs = nowMs;
    lastSampleMs_ = nowMs;

    // Bring-up electrical fault windows only. Final thermal limits remain
    // deliberately TBD and the heater is still hard-locked OFF.
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

    bool insideEmpiricalRange = false;
    const float temperatureC =
        empiricalTemperatureC(resistanceOhms, insideEmpiricalRange);
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
