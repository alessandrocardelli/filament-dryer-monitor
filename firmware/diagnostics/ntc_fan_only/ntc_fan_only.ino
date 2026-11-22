// Filament Dryer Monitor - NON-HEATING NTC/fan electrical-interference check.
// Purpose: after the 2026-11-22 Noctua+ASA AIR55 aborted run (NTC R LIMIT),
// check whether switching the FAN alone changes the ESP32's NTC ADC reading.
// The heater is ALWAYS commanded LOW on GPIO19; this sketch contains NO
// heater-on path, cycle controller, Wi-Fi, OTA, or normal dryer operation.
//
// This is a SINGLE cold, supervised diagnostic; NOT a heater test and NOT
// a thermistor calibration. Leave the Sanwa probe fixed. Wait until the
// assembly is completely at room temperature before running. Do not open
// hot equipment or change any safety device. This tests FAN-related ADC
// interference only; a normal result does NOT establish sensor accuracy
// during HEATER ON or rule out temperature/contact effects.
//
// Arduino IDE: ESP32 Dev Module, ESP32 Arduino core 3.3.12 and U8g2 library.
// Open this sketch, flash with the heater cold, and observe its OLED; the
// Serial Monitor at 115200 baud provides raw evidence if connected.
// Sequence after boot: FAN OFF 20 s; FAN ON at 100% 20 s; FAN OFF 20 s; END.
// Wait for END; do not reset midway. Original dryer firmware must be
// reloaded separately before ordinary operation. No thermal limits changed.
// Sources for ADC constants/GPIO/PWM: firmware/AppConfig.h and NtcSensor.h.
//
// Expected: compare average NTC resistance and corrected ADC millivolts
// across OFF -> ON -> OFF, ignoring the first 5 seconds of each state.
// Record the three means and state whether a clear immediate fan-linked
// jump occurred. No conclusion about heating safety follows from this test.

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <math.h>

static constexpr uint8_t PIN_HEATER = 19;
static constexpr uint8_t PIN_FAN = 16;
static constexpr uint8_t PIN_NTC = 34;
static constexpr uint32_t STAGE_MS = 20000;
static constexpr uint32_t SETTLE_MS = 5000;
static constexpr float ADC_SLOPE = 0.984369f;
static constexpr float ADC_OFFSET_MV = -2.322f;
static constexpr float DIVIDER_SUPPLY_MV = 3311.0f;
static constexpr float PULLUP_OHMS = 46572.0f;

U8G2_SSD1309_128X64_NONAME0_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

enum class Stage : uint8_t { OffBefore = 0, FanOn = 1, OffAfter = 2, Done = 3 };
Stage stage = Stage::OffBefore;
uint32_t stageStartedMs = 0;
uint32_t lastSampleMs = 0;
bool pwmReady = false;

struct Sample {
  float rawMv = NAN;
  float correctedMv = NAN;
  float ohms = NAN;
  bool valid = false;
};
struct Mean {
  double sumOhms = 0;
  double sumMv = 0;
  uint32_t count = 0;
  void add(const Sample &s) {
    if (!s.valid) return;
    sumOhms += s.ohms;
    sumMv += s.correctedMv;
    ++count;
  }
  float ohms() const { return count ? static_cast<float>(sumOhms / count) : NAN; }
  float mv() const { return count ? static_cast<float>(sumMv / count) : NAN; }
};
Mean means[3];
Sample last;

Sample readNtc() {
  Sample s;
  uint32_t sum = 0;
  for (uint8_t i = 0; i < 16; ++i) sum += analogReadMilliVolts(PIN_NTC);
  s.rawMv = static_cast<float>(sum) / 16.0f;
  s.correctedMv = ADC_SLOPE * s.rawMv + ADC_OFFSET_MV;
  if (!isfinite(s.correctedMv) || s.correctedMv <= 150.0f ||
      s.correctedMv >= 3000.0f ||
      s.correctedMv >= DIVIDER_SUPPLY_MV) return s;
  s.ohms = PULLUP_OHMS * s.correctedMv /
           (DIVIDER_SUPPLY_MV - s.correctedMv);
  s.valid = isfinite(s.ohms) && s.ohms > 0.0f;
  return s;
}

const char *stageName(Stage s) {
  switch (s) {
    case Stage::OffBefore: return "OFF 1";
    case Stage::FanOn: return "ON 100%";
    case Stage::OffAfter: return "OFF 2";
    case Stage::Done: return "END";
  }
  return "?";
}

void setFan(bool on) {
  // Only FAN_GPIO16 is ever enabled; HEATER_GPIO19 stays LOW.
  digitalWrite(PIN_HEATER, LOW);
  if (pwmReady) ledcWrite(PIN_FAN, on ? 255 : 0);
}

void advanceStage(uint32_t now) {
  if (stage == Stage::Done) return;
  stage = static_cast<Stage>(static_cast<uint8_t>(stage) + 1);
  stageStartedMs = now;
  setFan(stage == Stage::FanOn);
  Serial.print("EVENT,"); Serial.print(now);
  Serial.print(","); Serial.println(stageName(stage));
}

void draw(uint32_t now) {
  char line[31];
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tf);
  oled.drawStr(1, 9, "NTC / FAN - HEATER OFF");
  oled.drawHLine(0, 12, 128);

  snprintf(line, sizeof(line), "%s  %lus",
           stageName(stage),
           static_cast<unsigned long>(
             stage == Stage::Done ? 0 : (STAGE_MS - min(STAGE_MS, now-stageStartedMs))/1000UL));
  oled.drawStr(1, 24, line);
  if (last.valid) {
    snprintf(line, sizeof(line), "NTC %.2f kOhm", last.ohms / 1000.0f);
    oled.drawStr(1, 36, line);
    snprintf(line, sizeof(line), "ADC %.0f mV", last.correctedMv);
    oled.drawStr(1, 47, line);
  } else {
    oled.drawStr(1, 36, "NTC INVALID / CHECK");
  }
  if (stage != Stage::Done) {
    oled.drawStr(1, 61, "20s OFF / ON / OFF");
  } else {
    const float a=means[0].ohms()/1000.0f;
    const float b=means[1].ohms()/1000.0f;
    const float c=means[2].ohms()/1000.0f;
    if (isfinite(a) && isfinite(b) && isfinite(c)) {
      snprintf(line,sizeof(line),"%.1f / %.1f / %.1f k",a,b,c);
      oled.drawStr(1, 61, line);
    } else {
      oled.drawStr(1, 61, "INSUFFICIENT SAMPLES");
    }
  }
  oled.sendBuffer();
}

void setup() {
  digitalWrite(PIN_HEATER, LOW);
  pinMode(PIN_HEATER, OUTPUT);
  digitalWrite(PIN_HEATER, LOW);
  digitalWrite(PIN_FAN, LOW);
  pinMode(PIN_FAN, OUTPUT);
  digitalWrite(PIN_FAN, LOW);

  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_NTC, ADC_11db);
  pwmReady = ledcAttachChannel(PIN_FAN, 25000, 8, 9);
  setFan(false);

  Wire.begin(21, 22);
  Wire.setClock(100000);
  oled.setI2CAddress(0x3C << 1);
  oled.begin();

  Serial.println("NTC-FAN-ONLY DIAGNOSTIC; HEATER HARD OFF");
  Serial.println("No heat control. Original firmware must be restored.");
  Serial.println("ms,stage,fan_percent,reported_mv,corrected_mv,ntc_ohms,valid");
  stageStartedMs = millis();
  lastSampleMs = stageStartedMs - 500;
  if (!pwmReady) {
    stage = Stage::Done;
    Serial.println("ERROR: FAN LEDC ATTACH FAILED; FAN STAYS OFF");
  }
}

void loop() {
  digitalWrite(PIN_HEATER, LOW);
  const uint32_t now = millis();
  if (stage != Stage::Done && now - stageStartedMs >= STAGE_MS)
    advanceStage(now);

  if (now - lastSampleMs < 500) return;
  lastSampleMs = now;
  last = readNtc();

  if (stage != Stage::Done &&
      now - stageStartedMs >= SETTLE_MS) {
    means[static_cast<uint8_t>(stage)].add(last);
  }

  Serial.print(now); Serial.print(',');
  Serial.print(stageName(stage)); Serial.print(',');
  Serial.print(stage == Stage::FanOn ? 100 : 0); Serial.print(',');
  Serial.print(last.rawMv, 1); Serial.print(',');
  Serial.print(last.correctedMv, 1); Serial.print(',');
  Serial.print(last.ohms, 1); Serial.print(',');
  Serial.println(last.valid ? 1 : 0);

  if (stage == Stage::Done) {
    static bool printed = false;
    if (!printed) {
      printed = true;
      for (uint8_t i=0;i<3;++i) {
        Serial.print("SUMMARY,"); Serial.print(i);
        Serial.print(','); Serial.print(means[i].ohms(), 1);
        Serial.print(','); Serial.print(means[i].mv(), 1);
        Serial.print(','); Serial.println(means[i].count);
      }
    }
  }
  draw(now);
}
