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

  void render(uint32_t nowMs, AppState state, SetupField setupField,
              SettingsField settingsField,
              const PersistentSettings &settings,
              const SensorSnapshot &sensor, uint32_t remainingSeconds,
              uint8_t fanDutyPercent, bool technicalPage = false) {
    if (!awake_ || (nowMs - lastRenderMs_) < AppConfig::kRenderPeriodMs) return;
    lastRenderMs_ = nowMs;

    display_.clearBuffer();
    switch (state) {
      case AppState::Boot: drawSplash(); break;
      case AppState::Standby: drawStandby(sensor); break;
      case AppState::Setup: drawSetup(setupField, settings.cycle); break;
      case AppState::Settings: drawSettings(settingsField, settings); break;
      case AppState::Drying:
        technicalPage ? drawSystem(sensor, fanDutyPercent)
                      : drawDrying(settings.cycle, sensor, remainingSeconds,
                                  fanDutyPercent);
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

  static const char *volumeName(BuzzerVolume volume) {
    switch (volume) {
      case BuzzerVolume::Off: return "OFF";
      case BuzzerVolume::Low: return "LOW";
      case BuzzerVolume::Medium: return "MED";
      case BuzzerVolume::High: return "HIGH";
    }
    return "LOW";
  }

  void drawSplash() {
    const int16_t x = (128 - SLEWFORM_FULL_LOGO_WIDTH) / 2;
    const int16_t y = (64 - SLEWFORM_FULL_LOGO_HEIGHT) / 2;
    display_.drawXBMP(x, y, SLEWFORM_FULL_LOGO_WIDTH,
                      SLEWFORM_FULL_LOGO_HEIGHT, slewform_full_logo);
  }

  void drawStandby(const SensorSnapshot &sensor) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(20, 9, "FILAMENT DRYER");
    display_.drawHLine(0, 12, 128);

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
    display_.drawUTF8(2, 36, left);
    const int16_t rw = display_.getUTF8Width(right);
    display_.drawUTF8(126 - rw, 36, right);

    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 60, sensor.valid ? "Ready" : "SHT45 waiting...");
  }

  void drawSetup(SetupField selected, const CycleSettings &settings) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 9, "SET CYCLE   SAFE MODE");
    display_.drawHLine(0, 12, 128);

    char temp[22];
    char time[22];
    snprintf(temp, sizeof(temp), "TEMP        %d °C", settings.targetTempC);
    snprintf(time, sizeof(time), "TIME        %02u:%02u",
             settings.durationMinutes / 60, settings.durationMinutes % 60);
    drawSelectableRow(15, temp, selected == SetupField::Temperature);
    drawSelectableRow(31, time, selected == SetupField::Duration);
    drawSelectableRow(47, "FAN         AUTO", selected == SetupField::Fan);
  }

  void drawSettings(SettingsField selected,
                    const PersistentSettings &settings) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(35, 9, "SETTINGS");
    display_.drawHLine(0, 12, 128);

    char volume[22];
    char click[22];
    snprintf(volume, sizeof(volume), "BUZZER      %s",
             volumeName(settings.buzzerVolume));
    snprintf(click, sizeof(click), "KEY CLICK   %s",
             settings.keyClick ? "ON" : "OFF");

    drawSelectableRow(20, volume, selected == SettingsField::BuzzerVolume);
    drawSelectableRow(39, click, selected == SettingsField::KeyClick);
  }

  void drawDrying(const CycleSettings &settings, const SensorSnapshot &sensor,
                   uint32_t remainingSeconds, uint8_t fanDutyPercent) {
    display_.setFont(u8g2_font_6x10_tf);
    char header[22];
    snprintf(header, sizeof(header), "DRYING TEST   %02lu:%02lu",
             static_cast<unsigned long>(remainingSeconds / 3600UL),
             static_cast<unsigned long>((remainingSeconds / 60UL) % 60UL));
    display_.drawStr(0, 9, header);
    display_.drawHLine(0, 12, 128);

    char values[24];
    if (sensor.valid) {
      snprintf(values, sizeof(values), "%.1f °C   %.1f %%RH",
               sensor.temperatureC, sensor.humidityRh);
    } else {
      snprintf(values, sizeof(values), "--.- °C   --.- %%RH");
    }
    display_.setFont(u8g2_font_7x14B_tf);
    display_.drawUTF8(1, 32, values);

    display_.setFont(u8g2_font_6x10_tf);
    char target[24];
    snprintf(target, sizeof(target), "SET %d °C   FAN %u%%",
             settings.targetTempC, fanDutyPercent);
    display_.drawUTF8(1, 47, target);
    display_.drawStr(1, 61, "HEATER LOCKED");
  }

  void drawSystem(const SensorSnapshot &sensor, uint8_t fanDutyPercent) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 9, "SYSTEM / BRING-UP");
    display_.drawHLine(0, 12, 128);
    char line[24];
    if (sensor.valid) {
      snprintf(line, sizeof(line), "Chamber      %.1f °C",
               sensor.temperatureC);
      display_.drawUTF8(2, 27, line);
      snprintf(line, sizeof(line), "Humidity     %.1f %%",
               sensor.humidityRh);
      display_.drawStr(2, 39, line);
    } else {
      display_.drawStr(2, 27, "SHT45        WAIT");
    }
    display_.drawStr(2, 51, "Heater       LOCK");
    snprintf(line, sizeof(line), "Fan          %u%%", fanDutyPercent);
    display_.drawStr(2, 63, line);
  }

  void drawComplete(const SensorSnapshot &sensor) {
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(24, 18, "COMPLETE");
    display_.setFont(u8g2_font_6x10_tf);
    char line[24];
    if (sensor.valid) {
      snprintf(line, sizeof(line), "%.1f °C     %.1f %%RH",
               sensor.temperatureC, sensor.humidityRh);
      display_.drawUTF8(14, 38, line);
    }
    display_.drawStr(2, 61, "ON/OFF = acknowledge");
  }

  void drawFault() {
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(27, 17, "FAULT");
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(8, 35, "Safety controller fault");
    display_.drawStr(16, 49, "HEATER OFF");
    display_.drawStr(24, 62, "M = mute");
  }

  void drawSelectableRow(uint8_t y, const char *text, bool selected) {
    display_.setFont(u8g2_font_6x10_tf);
    if (selected) {
      display_.setDrawColor(1);
      display_.drawBox(0, y, 128, 15);
      display_.setDrawColor(0);
      display_.drawUTF8(2, y + 11, text);
      display_.setDrawColor(1);
    } else {
      display_.drawUTF8(2, y + 11, text);
    }
  }
};
