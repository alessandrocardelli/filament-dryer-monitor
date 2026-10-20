/*
 * GPIO16 / Q5 FAN STATIC CHECK — 2026-10-20
 *
 * DIAGNOSTIC ONLY. NOT PHYSICALLY TESTED.
 * ESP32-WROOM-32E-N4, Arduino-ESP32 core 3.x.
 * GPIO16 drives Q5 through R25; J6 supplies the 24V fan.
 *
 * IMPORTANT: Disconnect heater J5 with ALL supplies off before testing.
 * GPIO19 is commanded LOW here, but this is not physical isolation.
 * No PWM, LEDC, buzzer, UI or I2C is used.
 *
 * Timing after firmware setup:
 * 0–3 s: GPIO16 LOW (fan OFF)
 * 3–18 s: GPIO16 HIGH (fan commanded ON)
 * 18 s onward: GPIO16 LOW (fan OFF, stays OFF).
 * The test is ONCE PER RESET. Stay present and monitor the fan.
 */

#include <Arduino.h>

constexpr uint8_t kFanPin = 16;
constexpr uint8_t kHeaterPin = 19;
constexpr uint32_t kOffInitialMs = 3000;
constexpr uint32_t kOnDurationMs = 15000;

uint32_t startedMs = 0;
bool printedOn = false;
bool printedDone = false;

void setup() {
  // Inhibit heating before any fan activity.
  pinMode(kHeaterPin, OUTPUT);
  digitalWrite(kHeaterPin, LOW);

  pinMode(kFanPin, OUTPUT);
  digitalWrite(kFanPin, LOW);

  Serial.begin(115200);
  Serial.println();
  Serial.println("GPIO16 STATIC FAN CHECK - heater command LOCKED LOW");
  Serial.println("TEST NOT PHYSICALLY VALIDATED; J5 heater must be disconnected.");
  Serial.println("Fan OFF 3 s -> ON 15 s -> OFF permanently.");
  startedMs = millis();
}

void loop() {
  digitalWrite(kHeaterPin, LOW);
  const uint32_t elapsedMs = millis() - startedMs;

  if (elapsedMs < kOffInitialMs) {
    digitalWrite(kFanPin, LOW);
  } else if (elapsedMs < (kOffInitialMs + kOnDurationMs)) {
    digitalWrite(kFanPin, HIGH);
    if (!printedOn) {
      printedOn = true;
      Serial.println("FAN COMMAND HIGH: probe R25 pin 2 / R25 pin 1 / J6 pin 1");
    }
  } else {
    digitalWrite(kFanPin, LOW);
    if (!printedDone) {
      printedDone = true;
      Serial.println("FAN COMMAND LOW: timed test ended. Output stays OFF.");
    }
  }

  // Commands printed here are not measurements of the GPIO voltage.
  delay(10);
}
