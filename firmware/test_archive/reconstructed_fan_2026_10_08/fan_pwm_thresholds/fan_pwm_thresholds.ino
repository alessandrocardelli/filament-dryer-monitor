/*
 * RECONSTRUCTED FROM CHAT TEST RECORDS — NOT THE ORIGINAL OCT 8 FILE.
 * This isolated bench sketch has NOT been retested after reconstruction.
 * Heater GPIO19 is forced LOW; physically isolate heater J5 for testing.
 * ESP32 Arduino Core 3.x. Fan: GPIO16, switched 24V load through Q5/J6.
 */

#include <Arduino.h>
constexpr uint8_t FAN_PIN = 16;
constexpr uint8_t HEATER_PIN = 19;
static uint8_t duty(uint8_t percent) {
  return static_cast<uint8_t>((static_cast<unsigned>(percent) * 255U + 50U) / 100U);
}
static void setFan(uint8_t percent, unsigned long holdMs) {
  digitalWrite(HEATER_PIN, LOW);
  analogWrite(FAN_PIN, duty(percent));
  Serial.printf("Fan PWM %u%%, dwell %lu ms\n", percent, holdMs);
  delay(holdMs);
}
void setup() {
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  Serial.begin(115200);
  analogWriteResolution(FAN_PIN, 8);
  analogWriteFrequency(FAN_PIN, 25000);
  analogWrite(FAN_PIN, 0);
  Serial.println("Reconstructed fan thresholds study: watch spin manually.");
  // Every attempted start is preceded by a 4s stop.
  for (uint8_t p = 20; p <= 60; p += 5) {
    setFan(0, 4000);
    setFan(p, 5000);
  }
  // Run at 100% first, then lower duty progressively without a stop.
  setFan(100, 5000);
  for (int p = 50; p >= 10; p -= 5) {
    setFan(static_cast<uint8_t>(p), 5000);
  }
  setFan(0, 0);
  Serial.println("Fan threshold test completed; output OFF.");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
}
