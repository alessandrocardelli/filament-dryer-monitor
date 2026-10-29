#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "AppConfig.h"
#include "AppTypes.h"
#include "FirmwareBuild.h"
#include "HeaterPulseCommissioning.h"
#include "HeaterStep07GuardTrial.h"
#include "assets/slewform/slewform_animation_compact.h"

class Ui {
 public:
  explicit Ui(U8G2 &display) : display_(display) {}

  void begin(uint32_t nowMs) {
    display_.setI2CAddress(AppConfig::kOledAddress << 1);
    // initDisplay() leaves the SSD1309 in power-save mode, unlike begin().
    // Write the first complete emblem frame before allowing it to illuminate.
    display_.initDisplay();
    display_.clearBuffer();
    drawSplash(0);  // The static crown/trunk only; no root pixels yet.
    display_.sendBuffer();
    display_.setContrast(180);
    display_.setPowerSave(0);
    awake_ = true;
    lastBootStage_ = 0;
    lastInteractionMs_ = nowMs;
    lastRenderMs_ = nowMs;
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

  // Returns true only after the solo wordmark has been sent AND revealed.
  // The caller starts the chime after this returns, avoiding an audio lead
  // during the ~100 ms full-frame I2C transfer.
  bool render(uint32_t nowMs, AppState state, SetupField setupField,
              SettingsField settingsField,
              const PersistentSettings &settings,
              const SensorSnapshot &sensor, const NtcSnapshot &ntc,
              uint32_t remainingSeconds, uint8_t fanDutyPercent,
              bool technicalPage = false, uint32_t bootElapsedMs = 0,
              bool heaterLocked = true, bool heaterOn = false,
              uint8_t pulsePhase = 0, uint8_t pulsesDone = 0,
              uint8_t pulseFault = 0,
              const char *apName = nullptr,
              const char *apPassword = nullptr,
              const char *stationIp = nullptr,
              uint32_t coolingSecondsLeft = 0,
              bool csvReady = false, bool csvSaveFailed = false,
              uint8_t waitReason = 0) {
    if (!awake_) return false;

    const uint8_t stage = (state == AppState::Boot)
                              ? bootStage(bootElapsedMs) : 0;
    // Only send boot frames when their image changes, not every 100 ms.
    // In particular, never resend a partial/no-roots frame at the end.
    if (state == AppState::Boot && stage == lastBootStage_) return false;
    if (state != AppState::Boot &&
        (nowMs - lastRenderMs_) < AppConfig::kRenderPeriodMs) return false;
    lastRenderMs_ = nowMs;

    const bool revealingWordmark =
        state == AppState::Boot &&
        stage == SLEWFORM_ROOT_STEPS + 1 &&
        lastBootStage_ != stage;

    // The SSD1309 doesn't swap full frames atomically over I2C. Turn pixels
    // off only for the logo-to-wordmark page transition, send the entire new
    // image while hidden, then turn pixels back on. No half-erased roots.
    if (revealingWordmark) display_.setPowerSave(1);
    display_.clearBuffer();
    switch (state) {
      case AppState::Boot: drawSplash(stage); break;
      case AppState::Standby:
        if (bootElapsedMs >= AppConfig::kBootSplashMs &&
            (bootElapsedMs - AppConfig::kBootSplashMs) <
                FirmwareBuild::kSplashExtraMs)
          drawFirmwareSplash();
        else if (apName && apPassword)
          drawWifiSetup(apName, apPassword);
        else
          drawStandby(sensor, stationIp);
        break;
      case AppState::Setup: drawSetup(setupField, settings.cycle); break;
      case AppState::Settings: drawSettings(settingsField, settings); break;
      case AppState::Drying:
        technicalPage ? drawSystem(sensor, ntc, fanDutyPercent,
                                   heaterLocked, heaterOn,
                                   pulsePhase, pulsesDone)
                      : drawDrying(settings.cycle, sensor, remainingSeconds,
                                  fanDutyPercent, heaterLocked, heaterOn,
                                  pulsePhase, pulsesDone, coolingSecondsLeft,
                                  waitReason);
        break;
      case AppState::Complete: drawComplete(sensor, csvReady, csvSaveFailed); break;
      case AppState::Fault:
        drawFault(ntc.fault, pulseFault, coolingSecondsLeft,
                  csvReady, csvSaveFailed);
        break;
    }
    display_.sendBuffer();
    if (state == AppState::Boot) lastBootStage_ = stage;
    if (revealingWordmark) display_.setPowerSave(0);
    return revealingWordmark;
  }

 private:
  U8G2 &display_;
  bool awake_ = true;
  uint32_t lastInteractionMs_ = 0;
  uint32_t lastRenderMs_ = 0;
  uint8_t lastBootStage_ = 0;

  static uint8_t bootStage(uint32_t bootElapsedMs) {
    // 0 = upper emblem only; 1..11 = progressively longer roots;
    // 12 = separate wordmark screen.
    if (bootElapsedMs >= AppConfig::kBootWordmarkStartMs)
      return SLEWFORM_ROOT_STEPS + 1;
    if (bootElapsedMs < AppConfig::kBootRootHoldMs) return 0;
    const uint32_t step =
        (bootElapsedMs - AppConfig::kBootRootHoldMs) / SLEWFORM_ROOT_STEP_MS;
    return 1 + static_cast<uint8_t>(
        min(static_cast<uint32_t>(SLEWFORM_ROOT_STEPS - 1), step));
  }

  static const char *volumeName(BuzzerVolume volume) {
    switch (volume) {
      case BuzzerVolume::Off: return "OFF";
      case BuzzerVolume::Low: return "LOW";
      case BuzzerVolume::Medium: return "MED";
      case BuzzerVolume::High: return "HIGH";
    }
    return "LOW";
  }

  void drawSplash(uint8_t stage) {
    // Separate full-screen page; never display the wordmark below the emblem.
    if (stage == SLEWFORM_ROOT_STEPS + 1) {
      display_.drawXBMP(SLEWFORM_WORDMARK_X, SLEWFORM_WORDMARK_Y,
                        SLEWFORM_WORDMARK_WIDTH, SLEWFORM_WORDMARK_HEIGHT,
                        slewform_wordmark);
      return;
    }

    // Upper symbol stays fixed. Stage 0 is an intentional roots-free hold.
    display_.drawXBMP(SLEWFORM_SYMBOL_X, SLEWFORM_SYMBOL_Y,
                      SLEWFORM_SYMBOL_WIDTH, SLEWFORM_SYMBOL_HEIGHT,
                      slewform_symbol_upper);
    if (stage == 0) return;
    const uint16_t radiusSq4 = slewform_root_radius_sq_x4[stage - 1];
    const uint8_t stride = (SLEWFORM_SYMBOL_WIDTH + 7) / 8;
    for (uint8_t y = 0; y < SLEWFORM_SYMBOL_HEIGHT; ++y) {
      for (uint8_t x = 0; x < SLEWFORM_SYMBOL_WIDTH; ++x) {
        const uint16_t idx = y * stride + x / 8;
        const uint8_t packed = pgm_read_byte(&slewform_symbol_roots[idx]);
        if (!(packed & (1u << (x & 7)))) continue;
        const int16_t dx2 = 2 * x - SLEWFORM_ROOT_ORIGIN_X2;
        const int16_t dy2 = 2 * y - SLEWFORM_ROOT_ORIGIN_Y2;
        if (dx2 * dx2 + dy2 * dy2 <= radiusSq4) {
          display_.drawPixel(SLEWFORM_SYMBOL_X + x, SLEWFORM_SYMBOL_Y + y);
        }
      }
    }
  }

  // Keep the existing animated Slewform logo untouched. Immediately AFTER it,
  // identify the compiled test firmware for three seconds with no USB needed.
  void drawFirmwareSplash() {
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(13, 21, "AIR FEEDBACK");
    display_.drawHLine(0, 28, 128);
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 44, "FW"); // x2..14; separate from build id
    display_.drawStr(20, 44, FirmwareBuild::kId); // x20..122; no overlap with FW x2..14
    display_.drawStr(22, 60, FirmwareBuild::kMode);
  }

  void drawWifiSetup(const char *ssid, const char *password) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(34, 10, "WIFI SETUP");
    display_.drawHLine(0, 13, 128);
    display_.drawStr(2, 26, ssid);
    display_.drawStr(2, 39, "KEY:");
    display_.drawStr(30, 39, password);
    display_.drawStr(2, 57, "Go to 192.168.4.1");
  }

  void drawStandby(const SensorSnapshot &sensor, const char *stationIp) {
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
    if (stationIp && stationIp[0]) {
      display_.drawStr(2, 60, stationIp);
    } else {
      display_.drawStr(2, 60, sensor.valid ? "Ready" : "SHT45 waiting...");
      display_.drawStr(78, 60, FirmwareBuild::kShortId); // 8 chars * 6px; fits 128px
    }
  }

  void drawSetup(SetupField selected, const CycleSettings &settings) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 9, AppConfig::kHeaterStep09TrialMode
        ? "STEP11D AIR25 TEST" : "SET CYCLE   SAFE MODE");
    display_.drawHLine(0, 12, 128);

    char temp[22];
    char time[22];
    snprintf(temp, sizeof(temp), "TEMP        %d °C", AppConfig::kHeaterStep09TrialMode ? 25 : settings.targetTempC);
    snprintf(time, sizeof(time), "TIME        %02u:%02u",
             AppConfig::kHeaterStep09TrialMode ? 0 : settings.durationMinutes / 60,
              AppConfig::kHeaterStep09TrialMode ? 12 : settings.durationMinutes % 60);
    drawSelectableRow(15, temp, selected == SetupField::Temperature);
    drawSelectableRow(31, time, selected == SetupField::Duration);
    drawSelectableRow(47, AppConfig::kHeaterStep09TrialMode
        ? "FAN          100%" : "FAN         AUTO", selected == SetupField::Fan);
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
                   uint32_t remainingSeconds, uint8_t fanDutyPercent,
                   bool heaterLocked, bool heaterOn,
                   uint8_t pulsePhase, uint8_t pulsesDone,
                   uint32_t coolingSecondsLeft, uint8_t waitReason) {
    display_.setFont(u8g2_font_6x10_tf);
    char header[22];
    if (AppConfig::kHeaterStep09TrialMode) {
      snprintf(header, sizeof(header), "STEP11D AIR25 TEST");
    } else if (AppConfig::kHeaterCommissioningMode) {
      snprintf(header, sizeof(header), "NTC GUARD TEST FAN100");
    } else {
      snprintf(header, sizeof(header), "DRYING TEST   %02lu:%02lu",
               static_cast<unsigned long>(remainingSeconds / 3600UL),
               static_cast<unsigned long>((remainingSeconds / 60UL) % 60UL));
    }
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
    if (AppConfig::kHeaterStep09TrialMode) {
      if (static_cast<HeaterCommissioning::Phase>(pulsePhase) ==
          HeaterCommissioning::Phase::Cooldown)
        snprintf(target, sizeof(target), "COOLING %lu s",
            static_cast<unsigned long>(coolingSecondsLeft));
      else snprintf(target, sizeof(target), "AIR25 NTC45/60 H40");
    } else if (AppConfig::kHeaterCommissioningMode) {
      if (static_cast<HeaterCommissioning::Phase>(pulsePhase) ==
          HeaterCommissioning::Phase::Cooldown) {
        snprintf(target, sizeof(target), "COOLING LEFT %lu s",
                 static_cast<unsigned long>(coolingSecondsLeft));
      } else {
        snprintf(target, sizeof(target), "NTC TRIP 40k FAN %u%%",
                 fanDutyPercent);
      }
    } else {
      snprintf(target, sizeof(target), "SET %d °C   FAN %u%%",
               settings.targetTempC, fanDutyPercent);
    }
    display_.drawUTF8(1, 47, target);
    if (AppConfig::kHeaterStep09TrialMode) {
      const char *progress = "HEATER OFF";
      switch (static_cast<HeaterCommissioning::Phase>(pulsePhase)) {
        case HeaterCommissioning::Phase::Pulse: progress = "HEATER ON"; break;
        case HeaterCommissioning::Phase::Prerun: progress = "FAN PRERUN"; break;
        case HeaterCommissioning::Phase::Gap:
          switch (waitReason) {
            case 1: progress = "WAIT 12s OFF"; break;
            case 2: progress = "WAIT NTC 60k"; break;
            case 3: progress = "WAIT AIR 24C"; break;
            default: progress = "WAIT CONTROL"; break;
          }
          break;
        case HeaterCommissioning::Phase::Cooldown: progress = "COOLING"; break;
        case HeaterCommissioning::Phase::Complete: progress = "TEST COMPLETE"; break;
        default: break;
      }
      display_.setFont(u8g2_font_7x14B_tf);
      display_.drawStr(1, 62, progress);
    } else if (AppConfig::kHeaterCommissioningMode) {
      const unsigned totalPulses =
          static_cast<unsigned>(HeaterStep07GuardTrial::parameters().pulseCount);
      char progress[24];
      switch (static_cast<HeaterCommissioning::Phase>(pulsePhase)) {
        case HeaterCommissioning::Phase::Pulse:
          snprintf(progress, sizeof(progress), "HEATER ON %u/%u",
                   static_cast<unsigned>(pulsesDone + 1), totalPulses);
          break;
        case HeaterCommissioning::Phase::Complete:
          snprintf(progress, sizeof(progress), "TEST COMPLETE");
          break;
        default:
          snprintf(progress, sizeof(progress), "HEATER OFF");
          break;
      }
      // Large high-contrast actual heater command, not PH/code ambiguity.
      display_.setFont(u8g2_font_7x14B_tf);
      display_.drawStr(1, 62, progress);
    } else {
      display_.drawStr(1, 61, heaterLocked ? "HEATER LOCKED" :
                       (heaterOn ? "HEATER ON" : "HEATER OFF"));
    }
  }

  void drawSystem(const SensorSnapshot &sensor, const NtcSnapshot &ntc,
                  uint8_t fanDutyPercent, bool heaterLocked,
                  bool heaterOn, uint8_t pulsePhase, uint8_t pulsesDone) {
    display_.setFont(u8g2_font_6x10_tf);
    display_.drawStr(2, 9, "SYSTEM / HEATER");
    display_.drawHLine(0, 12, 128);
    char line[24];

    if (sensor.valid) {
      snprintf(line, sizeof(line), "Air SHT45 %.1f °C", sensor.temperatureC);
      display_.drawUTF8(2, 27, line);
    } else {
      display_.drawStr(2, 27, "Air SHT45 WAIT");
    }

    // Only the electrical NTC resistance has been validated for diagnostics.
    // SHT45 reads chamber air; the NTC is embedded in the heater assembly.
    if (ntc.valid) {
      // Technical page: raw resistance plus distance from the confirmed
      // experimental 40kOhm HARD trip. Not a Celsius thermometer.
      snprintf(line, sizeof(line), "NTC %.1fk HARD%+.1fk",
               ntc.resistanceOhms / 1000.0f,
               (ntc.resistanceOhms - 40000.0f) / 1000.0f);
      display_.drawStr(2, 39, line);
    } else {
      display_.drawStr(2, 39, "NTC R  FAULT");
    }

    if (sensor.valid) {
      snprintf(line, sizeof(line), "H %.0f%%       F %u%%",
               sensor.humidityRh, fanDutyPercent);
    } else {
      snprintf(line, sizeof(line), "H --%%       F %u%%", fanDutyPercent);
    }
    display_.drawStr(2, 54, line);
    if (AppConfig::kHeaterStep09TrialMode) {
      snprintf(line, sizeof(line), "PH%u WIN%u H:%s",
        static_cast<unsigned>(pulsePhase),
        static_cast<unsigned>(pulsesDone), heaterOn ? "ON" : "OFF");
      display_.drawStr(2, 63, line);
    } else if (AppConfig::kHeaterCommissioningMode) {
      snprintf(line, sizeof(line), "PH %u  PULSES %u/%u  H:%s",
               static_cast<unsigned>(pulsePhase),
               static_cast<unsigned>(pulsesDone),
               static_cast<unsigned>(HeaterStep07GuardTrial::parameters().pulseCount),
               heaterOn ? "ON" : "OFF");
      display_.drawStr(2, 63, line);
    } else {
      display_.drawStr(2, 63, heaterLocked ? "HEATER LOCKED" :
                       (heaterOn ? "HEATER ON" : "HEATER OFF"));
    }
  }

  void drawComplete(const SensorSnapshot &sensor, bool csvReady,
                    bool csvSaveFailed) {
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(24, 18, "COMPLETE");
    display_.setFont(u8g2_font_6x10_tf);
    char line[24];
    if (sensor.valid) {
      snprintf(line, sizeof(line), "%.1f °C     %.1f %%RH",
               sensor.temperatureC, sensor.humidityRh);
      display_.drawUTF8(14, 38, line);
    }
    display_.drawStr(2, 52, csvReady ? "CSV SAVED" :
                     (csvSaveFailed ? "CSV SAVE ERROR" : "CSV SAVING"));
    display_.drawStr(2, 63, "ON/OFF = acknowledge");
  }

  static const char *commissionFaultText(uint8_t code) {
    switch (static_cast<HeaterCommissioning::Fault>(code)) {
      case HeaterCommissioning::Fault::NtcInvalid: return "NTC INVALID";
      case HeaterCommissioning::Fault::NtcStale: return "NTC STALE";
      case HeaterCommissioning::Fault::AirInvalid: return "SHT45 INVALID";
      case HeaterCommissioning::Fault::AirStale: return "SHT45 STALE";
      case HeaterCommissioning::Fault::AirLimit: return "AIR LIMIT 40C";
      case HeaterCommissioning::Fault::NtcResistanceLimit: return "NTC R LIMIT";
      case HeaterCommissioning::Fault::FanUnavailable: return "FAN NO START";
      case HeaterCommissioning::Fault::FanLost: return "FAN LOST";
      case HeaterCommissioning::Fault::OverallTimeout: return "TEST TIMEOUT";
      case HeaterCommissioning::Fault::PulseWatchdog: return "PULSE TIMEOUT";
      case HeaterCommissioning::Fault::ManualStop: return "MANUAL STOP";
      case HeaterCommissioning::Fault::None: return "NO TEST FAULT";
    }
    return "UNKNOWN";
  }

  void drawFault(NtcFault ntcFault, uint8_t pulseFault,
                 uint32_t coolingSecondsLeft, bool csvReady,
                 bool csvSaveFailed) {
    display_.setFont(u8g2_font_6x10_tf);
    if (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode) {
      // Sanwa is external: prompt the operator, never imply MCU measures it.
      display_.drawStr(1, 9, "FAULT / HEATER OFF");
      display_.drawHLine(0, 12, 128);
      display_.drawStr(1, 24, commissionFaultText(pulseFault));
      display_.drawStr(1, 36, "SANWA: CHECK FALL");
      char line[24];
      if (coolingSecondsLeft > 0) {
        snprintf(line, sizeof(line), "COOLING  %lu:%02lu",
                 static_cast<unsigned long>(coolingSecondsLeft / 60U),
                 static_cast<unsigned long>(coolingSecondsLeft % 60U));
      } else if (csvReady) {
        snprintf(line, sizeof(line), "COOL DONE / CSV READY");
      } else if (csvSaveFailed) {
        snprintf(line, sizeof(line), "CSV SAVE FAILED");
      } else {
        snprintf(line, sizeof(line), "COOL DONE / SAVING");
      }
      display_.drawStr(1, 48, line);
      display_.drawStr(1, 62, "MODE = MUTE BEEP");
      return;
    }

    // Original V1 alarm presentation retained; D022 is not reopened.
    display_.setFont(u8g2_font_helvB12_tf);
    display_.drawStr(27, 17, "FAULT");
    display_.setFont(u8g2_font_6x10_tf);
    switch (ntcFault) {
      case NtcFault::OpenCircuit:
        display_.drawStr(24, 34, "HEATER NTC");
        display_.drawStr(20, 45, "invalid / open");
        break;
      case NtcFault::ShortCircuit:
        display_.drawStr(24, 34, "HEATER NTC");
        display_.drawStr(17, 45, "invalid / short");
        break;
      case NtcFault::InvalidReading:
        display_.drawStr(24, 34, "HEATER NTC");
        display_.drawStr(27, 45, "invalid");
        break;
      case NtcFault::None:
        display_.drawStr(8, 35, "Safety controller fault");
        break;
    }
    display_.drawStr(28, 55, "HEATER OFF");
    display_.drawStr(40, 64, "M=mute");
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
