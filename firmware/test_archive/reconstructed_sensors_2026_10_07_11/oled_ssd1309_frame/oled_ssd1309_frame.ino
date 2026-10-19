/*
 * RECONSTRUCTION, not the original conversation .ino (see archive index).
 * Behavior mirrors a physically successful historical test.
 * Reconstructed file itself NOT bench-tested.
 * Heater drive GPIO19 forced LOW. Isolate J5 physically before any bench test.
 */
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
constexpr uint8_t HEATER_PIN = 19;
constexpr uint8_t FAN_PIN = 16;
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C oled(U8G2_R0,U8X8_PIN_NONE);
void setup() {
  pinMode(HEATER_PIN, OUTPUT); digitalWrite(HEATER_PIN, LOW);
  pinMode(FAN_PIN, OUTPUT); digitalWrite(FAN_PIN, LOW);
  Serial.begin(115200);
  Wire.begin(21,22,100000);
  oled.setI2CAddress(0x3C << 1);
  oled.begin();
  Serial.println("SSD1309 test frame on 0x3C");
}
void loop() {
  digitalWrite(HEATER_PIN, LOW);
  oled.clearBuffer();
  oled.drawFrame(0,0,128,64);
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(8,17,"SSD1309 128 x 64");
  oled.drawStr(8,34,"Slewform I2C TEST");
  oled.drawStr(8,51,"OLED 0x3C - OK?");
  oled.sendBuffer();
  delay(2000);
}
