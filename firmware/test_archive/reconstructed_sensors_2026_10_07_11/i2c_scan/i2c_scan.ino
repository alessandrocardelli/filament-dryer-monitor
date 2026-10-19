/*
 * RECONSTRUCTION, not the original conversation .ino (see archive index).
 * Behavior mirrors a physically successful historical test.
 * Reconstructed file itself NOT bench-tested.
 * Heater drive GPIO19 forced LOW. Isolate J5 physically before any bench test.
 */
#include <Arduino.h>
#include <Wire.h>
constexpr uint8_t HEATER_PIN = 19;
constexpr uint8_t FAN_PIN = 16;
void setup() {
  pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, LOW);
  Serial.begin(115200);
  Wire.begin(21, 22, 100000);
  Serial.println("I2C scan: expected SSD1309 0x3C, SHT45 0x44");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
  uint8_t count = 0;
  for (uint8_t addr = 1; addr < 127; ++addr) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C found 0x%02X\n", addr);
      ++count;
    }
  }
  Serial.printf("Scan complete, devices=%u\n", count);
  delay(2000);
}
