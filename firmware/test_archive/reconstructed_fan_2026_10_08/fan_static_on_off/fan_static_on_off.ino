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
  Serial.println("Fan static test: OFF 3 s, then ON (24V fan through J6)");
  delay(3000);
  digitalWrite(FAN_PIN, HIGH);
  Serial.println("Fan commanded ON");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
}
