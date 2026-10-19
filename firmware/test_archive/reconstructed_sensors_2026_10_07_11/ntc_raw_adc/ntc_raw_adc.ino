/*
 * RECONSTRUCTION, not the original conversation .ino (see archive index).
 * Behavior mirrors a physically successful historical test.
 * Reconstructed file itself NOT bench-tested.
 * Heater drive GPIO19 forced LOW. Isolate J5 physically before any bench test.
 */
#include <Arduino.h>
constexpr uint8_t HEATER_PIN = 19;
constexpr uint8_t FAN_PIN = 16;
constexpr uint8_t NTC_PIN = 34;
void setup() {
  pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, LOW);
  Serial.begin(115200);
  pinMode(NTC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(NTC_PIN, ADC_11db);
  Serial.println("NTC GPIO34 RAW + calibrated mV; heater OFF");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
  uint32_t sumRaw=0, sumMv=0;
  for (uint8_t i=0;i<16;++i) {
    sumRaw+=analogRead(NTC_PIN);
    sumMv+=analogReadMilliVolts(NTC_PIN);
  }
  Serial.printf("NTC RAW=%.1f ADC_mV=%.1f\n",sumRaw/16.0f,sumMv/16.0f);
  delay(1000);
}
