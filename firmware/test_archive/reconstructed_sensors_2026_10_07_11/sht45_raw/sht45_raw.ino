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
constexpr uint8_t SHT45_ADDR = 0x44;
uint8_t crc8(uint8_t a, uint8_t b) {
  uint8_t crc = 0xFF;
  for (uint8_t data : {a, b}) {
    crc ^= data;
    for (uint8_t j = 0; j < 8; ++j)
      crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1;
  }
  return crc;
}
void setup() {
  pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, LOW);
  Serial.begin(115200);
  Wire.begin(21, 22, 100000);
  Serial.println("SHT45 single shot 0xFD, CRC-8 polynomial 0x31");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
  Wire.beginTransmission(SHT45_ADDR);
  Wire.write(0xFD); // high repeatability measurement
  if (Wire.endTransmission() != 0) {
    Serial.println("SHT45 write error");
    delay(2000); return;
  }
  delay(15);
  if (Wire.requestFrom(SHT45_ADDR, static_cast<uint8_t>(6)) != 6) {
    Serial.println("SHT45 read error");
    delay(2000); return;
  }
  uint8_t tH=Wire.read(), tL=Wire.read(), tC=Wire.read();
  uint8_t hH=Wire.read(), hL=Wire.read(), hC=Wire.read();
  if (crc8(tH,tL)!=tC || crc8(hH,hL)!=hC) {
    Serial.println("SHT45 CRC error");
    delay(2000); return;
  }
  uint16_t rawT=(static_cast<uint16_t>(tH)<<8)|tL;
  uint16_t rawH=(static_cast<uint16_t>(hH)<<8)|hL;
  float temperature=-45.0f+175.0f*rawT/65535.0f;
  float humidity=-6.0f+125.0f*rawH/65535.0f;
  humidity=constrain(humidity, 0.0f, 100.0f);
  Serial.printf("SHT45 T=%.2f C RH=%.2f%%\n", temperature, humidity);
  delay(2000);
}
