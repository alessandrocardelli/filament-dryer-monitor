/*
 * RECONSTRUCTED FROM CHAT TEST RECORDS — NOT THE ORIGINAL OCT 8 FILE.
 * This isolated bench sketch has NOT been retested after reconstruction.
 * Heater GPIO19 is forced LOW; physically isolate heater J5 for testing.
 * ESP32 Arduino Core 3.x. Fan: GPIO16, switched 24V load through Q5/J6.
 */

#include <Arduino.h>
constexpr uint8_t FAN_PIN = 16;
constexpr uint8_t HEATER_PIN = 19;
void setup() {
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  Serial.begin(115200);
  analogWriteResolution(FAN_PIN, 8);
  analogWriteFrequency(FAN_PIN, 25000);
  analogWrite(FAN_PIN, 0);
  Serial.println("Fan OFF for 3 s, then 50% PWM 25 kHz");
  delay(3000);
  analogWrite(FAN_PIN, 128);
  Serial.println("Fan 50% commanded");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
}
