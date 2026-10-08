#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "AppConfig.h"
#include "AppTypes.h"
#include "assets/slewform/slewform_logo_128x64.h"

class Ui {
 public:
  explicit Ui(U8G2 &display) : display_(display) {}

  void begin(uint32_t nowMs) {
    display_.setI2CAddress(AppConfig::kOledAddress << 1);
    display_.begin();

    // Prevent uninitialized OLED RAM from being visible before the first UI frame.
    // Keep the panel dark, write a known-black framebuffer, then enable it.
    display_.setPowerSave(1);
    display_.clearBuffer();
    display_.sendBuffer();
    display_.setContrast(180);
    display_.setPowerSave(0);

    awake_ = true;
    lastInteractionMs_ = nowMs;
    lastRenderMs_ = nowMs - AppConfig::kRenderPeriodMs;
  }

  void noteInteraction(uint32_t nowMs) { lastInteractionMs_ = nowMs; }

  bool wakeAndConsumeIfSleeping(uint32_t nowMs) {
    if (awake_) return false;
    display_.setPowerSave(0);
    awake_ = true;
    lastInteractionMs_ = nowMs;
    lastRenderMs_ = 0;
    return true;
  }

  void updateSleep(uint32_t nowMs, AppState state) {
    if (!awake_ || state != AppState::Standby) return;
    if ((nowMs - lastInteractionMs_) >= AppConfig::kDisplaySleepMs) {
      display_.setPowerSave(1);
      awake_ = false;
    }
  }

  bool isAwake() const { return awake_; }

  void render(uint32_t nowMs, AppState state, SetupField field,
              const CycleSettings &settings, const SensorSnapshot &sensor,
              uint32_t remainingSeconds, bool technicalPage = false) {
    if (!awake_ || (nowMs - lastRenderMs_) < AppConfig::kRenderPeriodMs) return;
    lastRenderMs_ = nowMs;

    display_.clearBuffer();
    switch (state) {
      case AppState::Boot: drawSplash(); break;
      case AppState::Standby: drawStandby(sensor); break;
      case AppState::Setup: drawSetup(field, settings); break;
      case AppState::Drying:
        technicalPage ? drawSystem(sensor) : drawDrying(settings, sensor, remainingSeconds);
        break;
      case AppState::Complete: drawComplete(sensor); break;
      case AppState::Fault: drawFault(); break;
    }
    display_.sendBuffer();
  }

 private:
  U8G2 &display_;
  bool awake_ = true;
  uint32_t lastInteractionMs_ = 0;
  uint32_t lastRenderMs_ = 0;

  void drawSplash() {
    const int16_t x = (128 - SLEWFORM_FULL_LOGO_WIDTH) / 2;
    const int16_t y = (64 - SLEWFORM_FULL_LOGO_HEIGHT) / 2;
    display_.drawXBMP(x, y, SLEWFORM_FULL_LOGO_WIDTH, SLEWFORM_FULL_LOGO_HEIGHT,
                      slewform_full_logo);
  }

  void drawStandby(const SensorSnapshot &sensor) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(20, 9, "FILAMENT DRYER");
    display_.drawHLine(0, 12, 128);
    drawButtonHints("START", "SET", nullptr, nullptr);

    display_.setFont(u8g2_font_helvB12_tf);
    char left[16];
    char right[16];
    if (sensor.valid) {
      snprintf(left, sizeof(left), "%.1f °C", sensor.temperatureC);
      snprintf(right, sizeof(right), "%.1f%%", sensor.humidityRh);
    } else {
      snprintf(left, sizeof(left), "--.- °C");
      snprintf(right, sizeof(right), "--.-%%");
    }
    display_.drawUTF8(2, 40, left);
    const int16_t rw = display_.getUTF8Width(right);
    display_.drawUTF8(126 - rw, 40, right);

    if (!sensor.valid) {
      display_.setFont(u8g2_font_4x6_tf);
      display_.drawStr(32, 61, "SHT45 waiting...");
    }
  }

  void drawSetup(SetupField selected, const CycleSettings &settings) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(38, 9, "SET CYCLE");
    display_.drawHLine(0, 12, 128);
    drawButtonHints("START", "NEXT", "+", "-");

    char temp[18];
    char time[18];
    snprintf(temp, sizeof(temp), "TEMP %d °C", settings.targetTempC);
    snprintf(time, sizeof(time), "TIME %02u:%02u",
             settings.durationMinutes / 60, settings.durationMinutes % 60);
    drawSelectableField(17, temp, selected == SetupField::Temperature);
    drawSelectableField(33, time, selected == SetupField::Duration);
    drawSelectableField(49, "FAN  AUTO", selected == SetupField::Fan);
  }

  void drawDrying(const CycleSettings &settings, const SensorSnapshot &sensor,
                   uint32_t remainingSeconds) {
    display_.setFont(u8g2_font_6x10_tf);
    char header[22];
    snprintf(header, sizeof(header), "DRYING SAFE   %02lu:%02lu",
             static_cast<unsigned long>(remainingSeconds / 3600UL),
             static_cast<unsigned long>((remainingSeconds / 60UL) % 60UL));
    display_.drawStr(0, 9, header);
    display_.drawHLine(0, 12, 128);
    drawButtonHints("STOP", "PAGE", nullptr, nullptr);

    char values[24];
    if (sensor.valid) {
      snprintf(values, sizeof(values), "%.1f °C   %.1f %%RH", sensor.temperatureC, sensor.humidityRh);
    } else {
      snprintf(values, sizeof(values), "--.- °C   --.- %%RH");
    }
    display_.setFont(u8g2_font_7x14B_tf);
    display_.drawUTF8(1, 32, values);

    display_.setFont(u8g2_font_6x10_tf);
    char target[24];
    snprintf(target, sizeof(target), "SET %d °C   FAN LOCK", settings.targetTempC);
    display_.drawUTF8(1, 47, target);
    display_.drawStr(1, 61, "HEATER LOCKED");
  }

  void drawSystem(const SensorSnapshot &sensor) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 9, "SYSTEM / BRING-UP");
    display_.drawHLine(0, 12, 128);
    drawButtonHints("STOP", "PAGE", nullptr, nullptr);
    char line[24];
    if (sensor.valid) {
      snprintf(line, sizeof(line), "Chamber      %.1f °C", sensor.temperatureC);
      display_.drawUTF8(2, 27, line);
      snprintf(line, sizeof(line), "Humidity     %.1f %%", sensor.humidityRh);
      display_.drawStr(2, 39, line);
    } else {
      display_.drawStr(2, 27, "SHT45        WAIT");
    }
    display_.drawStr(2, 51, "Heater       LOCK");
    display_.drawStr(2, 63, "Fan          LOCK");
  }

  void drawComplete(const SensorSnapshot &sensor) {
    drawButtonHints("OK", nullptr, nullptr, nullptr);
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(24, 18, "COMPLETE");
    display_.setFont(u8g2_font_6x10_tf);
    char line[24];
    if (sensor.valid) {
      snprintf(line, sizeof(line), "%.1f °C     %.1f %%RH", sensor.temperatureC, sensor.humidityRh);
      display_.drawUTF8(14, 38, line);
    }
    display_.drawStr(2, 61, "ON/OFF = acknowledge");
  }

  void drawFault() {
    drawButtonHints(nullptr, "MUTE", nullptr, nullptr);
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(27, 17, "FAULT");
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(8, 35, "Safety controller fault");
    display_.drawStr(16, 49, "HEATER OFF");
    display_.drawStr(24, 62, "M = mute");
  }

  void drawSelectableField(uint8_t y, const char *text, bool selected) {
    constexpr uint8_t kX = 24;
    constexpr uint8_t kW = 82;
    constexpr uint8_t kH = 14;
    display_.setFont(u8g2_font_6x10_tf);
    if (selected) {
      display_.setDrawColor(1);
      display_.drawBox(kX, y, kW, kH);
      display_.setDrawColor(0);
      display_.drawUTF8(kX + 3, y + 11, text);
      display_.setDrawColor(1);
    } else {
      display_.drawUTF8(kX + 3, y + 11, text);
    }
  }

  void drawButtonHints(const char *leftTop, const char *leftBottom,
                       const char *rightTop, const char *rightBottom) {
    display_.setFont(u8g2_font_4x6_tf);
    constexpr uint8_t kTopY = 23;
    constexpr uint8_t kBottomY = 62;

    if (leftTop) display_.drawStr(0, kTopY, leftTop);
    if (leftBottom) display_.drawStr(0, kBottomY, leftBottom);

    if (rightTop) {
      const int16_t w = display_.getStrWidth(rightTop);
      display_.drawStr(127 - w, kTopY, rightTop);
    }
    if (rightBottom) {
      const int16_t w = display_.getStrWidth(rightBottom);
      display_.drawStr(127 - w, kBottomY, rightBottom);
    }
  }
};
