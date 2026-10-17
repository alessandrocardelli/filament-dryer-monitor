// Filament Dryer Monitor - passive NTC diagnostic only.
// DO NOT USE FOR HEATER CONTROL OR SAFETY THRESHOLDS.
// It cannot energize the heater: GPIO19 is continuously forced LOW.
// The recorded Sanwa temperature is a nearby-probe equivalent,
// NOT necessarily the thermistor bead's physical temperature.
//
// Seven reference points were taken with BOTH enclosure covers open
// and the fan OFF. This standalone sketch retains that limitation.
// See docs/NTC_CALIBRATION_ANALYSIS.md for details.

#include <Arduino.h>
#include <math.h>

constexpr uint8_t PIN_HEATER = 19;
constexpr uint8_t PIN_FAN = 16;
constexpr uint8_t PIN_NTC = 34;

constexpr float ADC_SLOPE = 0.984369f;
constexpr float ADC_OFFSET_MV = -2.322f;
constexpr float SUPPLY_MV = 3311.0f;
constexpr float PULLUP_OHM = 46572.0f;
constexpr uint32_t PERIOD_MS = 1000;

constexpr size_t N_POINTS = 7;
constexpr float SANWA_T_C[N_POINTS] = {
  30.0f, 39.0f, 47.0f, 56.0f, 64.0f, 74.0f, 85.0f
};
constexpr float OBSERVED_NTC_OHM[N_POINTS] = {
  64620.0f, 42270.0f, 28720.0f, 19680.0f,
  13730.0f, 9810.0f, 7120.0f
};

// Original reference table in firmware/NtcSensor.h.
constexpr float LEGACY_T_C[7] = {
  25.0f, 28.0f, 30.0f, 35.0f, 40.0f, 44.0f, 50.0f
};
constexpr float LEGACY_R_OHM[7] = {
  82500.0f, 75500.0f, 67400.0f, 51700.0f,
  43600.0f, 35930.0f, 28800.0f
};

float betaToTempC(float r, float r25, float beta) {
  if (!isfinite(r) || r <= 0.0f) return NAN;
  const float invTempK =
      (1.0f / 298.15f) + logf(r / r25) / beta;
  if (!isfinite(invTempK) || invTempK <= 0.0f) return NAN;
  return 1.0f / invTempK - 273.15f;
}

// Exact currently integrated application conversion:
// 25..50 C empirical table plus R25=82.5k/B4100 outside.
float currentAppEstimateC(float r) {
  for (size_t i = 0; i + 1 < 7; ++i) {
    if (r <= LEGACY_R_OHM[i] &&
        r >= LEGACY_R_OHM[i + 1]) {
      const float f = (LEGACY_R_OHM[i] - r) /
                      (LEGACY_R_OHM[i] - LEGACY_R_OHM[i + 1]);
      return LEGACY_T_C[i] +
          f * (LEGACY_T_C[i + 1] - LEGACY_T_C[i]);
    }
  }
  return betaToTempC(r, 82500.0f, 4100.0f);
}

// Continuous B4100 high-temperature extrapolation used in the
// manually flashed heating test sketches, anchored at 50 C/28.8k.
// NOT the same as the integrated app's outside-table extrapolation.
float oldHeatingTestEstimateC(float r) {
  if (r >= 28800.0f) return currentAppEstimateC(r);
  const float inverseK = 1.0f / 323.15f +
      logf(r / 28800.0f) / 4100.0f;
  if (!isfinite(inverseK) || inverseK <= 0.0f) return NAN;
  return 1.0f / inverseK - 273.15f;
}

// Bench Sanwa-equivalent lookup: linear T versus ln(R).
// At the seven observed points it exactly returns the Sanwa readings.
// No extrapolation. It must NOT be used as a true-NTC temperature
// measurement, a heater setpoint conversion, or a safety input.
bool sanwaEquivalentC(float r, float &tempC) {
  tempC = NAN;
  if (!isfinite(r) ||
      r > OBSERVED_NTC_OHM[0] ||
      r < OBSERVED_NTC_OHM[N_POINTS - 1]) return false;

  for (size_t i = 0; i + 1 < N_POINTS; ++i) {
    const float hi = OBSERVED_NTC_OHM[i];
    const float lo = OBSERVED_NTC_OHM[i + 1];
    if (r <= hi && r >= lo) {
      const float f = logf(hi / r) / logf(hi / lo);
      tempC = SANWA_T_C[i] +
          f * (SANWA_T_C[i + 1] - SANWA_T_C[i]);
      return isfinite(tempC);
    }
  }
  return false;
}

float readResistanceOhms() {
  uint32_t sumMv = 0;
  for (uint8_t i = 0; i < 16; ++i) {
    sumMv += analogReadMilliVolts(PIN_NTC);
  }
  const float correctedMv = ADC_SLOPE *
      (static_cast<float>(sumMv) / 16.0f) + ADC_OFFSET_MV;
  if (!isfinite(correctedMv) ||
      correctedMv <= 150.0f ||
      correctedMv >= 3000.0f ||
      correctedMv >= SUPPLY_MV) return NAN;

  const float ohms =
      PULLUP_OHM * correctedMv / (SUPPLY_MV - correctedMv);
  return (isfinite(ohms) && ohms > 0.0f) ? ohms : NAN;
}

void setup() {
  digitalWrite(PIN_HEATER, LOW);
  pinMode(PIN_HEATER, OUTPUT);
  digitalWrite(PIN_HEATER, LOW);
  digitalWrite(PIN_FAN, LOW);
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_FAN, LOW);

  Serial.begin(115200);
  delay(700);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_NTC, ADC_11db);

  Serial.println();
  Serial.println("NTC PASSIVE REFERENCE VIEWER - NO HEATING");
  Serial.println("Both covers OPEN, fan OFF: bench-only data.");
  Serial.println("Sanwa-equivalent valid ONLY for 7.12..64.62 kOhm.");
  Serial.println("100k/B3950 is an unverified thermistor hypothesis.");
  Serial.println("appC/testC are legacy diagnostic estimates.");
  Serial.println("R_kOhm | appC | testC | SanwaEqC | beta7EqC | 100kB3950C");
}

void loop() {
  // Defensive guarantee: no heater PWM or enable path in this sketch.
  digitalWrite(PIN_HEATER, LOW);
  digitalWrite(PIN_FAN, LOW);

  static uint32_t lastSampleMs = 0;
  const uint32_t now = millis();
  if (now - lastSampleMs < PERIOD_MS) return;
  lastSampleMs = now;

  const float r = readResistanceOhms();
  if (!isfinite(r)) {
    Serial.println("NTC ELECTRICAL FAULT / ADC OUT OF RANGE; HEATER OFF");
    return;
  }

  float sanwaC = NAN;
  const bool withinRange = sanwaEquivalentC(r, sanwaC);
  Serial.print(r / 1000.0f, 2);
  Serial.print(" | ");
  Serial.print(currentAppEstimateC(r), 2);
  Serial.print(" | ");
  Serial.print(oldHeatingTestEstimateC(r), 2);
  Serial.print(" | ");
  if (withinRange) {
    Serial.print(sanwaC, 2);
  } else {
    Serial.print("OUT_OF_MEASURED_RANGE");
  }
  Serial.print(" | ");
  if (withinRange) {
    Serial.print(betaToTempC(r, 80690.9343f, 4421.57114f), 2);
  } else {
    Serial.print("OUT_OF_MEASURED_RANGE");
  }
  Serial.print(" | ");
  Serial.println(betaToTempC(r, 100000.0f, 3950.0f), 2);
}
