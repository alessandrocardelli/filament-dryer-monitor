#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "AppConfig.h"
#include "AppTypes.h"
#include "ButtonManager.h"
#include "Buzzer.h"
#include "CycleController.h"
#include "FanController.h"
#include "SafetyController.h"
#include "SettingsStore.h"
#include "Sht45Sensor.h"
#include "StatusLed.h"
#include "Ui.h"

U8G2_SSD1309_128X64_NONAME0_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

ButtonManager buttons;
Buzzer buzzer;
CycleController cycle;
FanController fan;
SafetyController safety;
SettingsStore settingsStore;
Sht45Sensor sht45(Wire);
StatusLed statusLed;
Ui ui(oled);

AppState appState = AppState::Boot;
SetupField setupField = SetupField::Temperature;
SettingsField settingsField = SettingsField::BuzzerVolume;
PersistentSettings settings;

uint32_t bootStartedMs = 0;
bool bootWordmarkChimeStarted = false;
bool technicalPage = false;

static void applyBuzzerSettings() {
  buzzer.setVolume(settings.buzzerVolume);
}

static void enterState(AppState next, uint32_t nowMs) {
  if (appState == AppState::Drying && next != AppState::Drying) {
    cycle.stop();
    fan.stop();
  }

  appState = next;
  technicalPage = false;
  ui.noteInteraction(nowMs);

  switch (next) {
    case AppState::Standby:
      fan.stop();
      Serial.println("STATE -> STANDBY");
      break;
    case AppState::Setup:
      fan.stop();
      Serial.println("STATE -> SETUP");
      break;
    case AppState::Settings:
      fan.stop();
      settingsField = SettingsField::BuzzerVolume;
      Serial.println("STATE -> SETTINGS");
      break;
    case AppState::Drying:
      settingsStore.save(settings);
      cycle.start(settings.cycle, nowMs);
      fan.startAuto(nowMs);
      buzzer.playCycleStart(nowMs);
      Serial.println("STATE -> DRYING TEST (fan active; heater locked OFF)");
      break;
    case AppState::Complete:
      fan.stop();
      buzzer.playComplete(nowMs);
      Serial.println("STATE -> COMPLETE");
      break;
    case AppState::Fault:
      fan.stop();
      buzzer.startFaultAlarm(nowMs);
      Serial.println("STATE -> FAULT");
      break;
    case AppState::Boot:
      fan.stop();
      break;
  }
}

static void selectNextSetupField() {
  switch (setupField) {
    case SetupField::Temperature: setupField = SetupField::Duration; break;
    case SetupField::Duration: setupField = SetupField::Fan; break;
    case SetupField::Fan: setupField = SetupField::Temperature; break;
  }
}

static void selectNextSettingsField() {
  settingsField = (settingsField == SettingsField::BuzzerVolume)
                      ? SettingsField::KeyClick
                      : SettingsField::BuzzerVolume;
}

static void adjustSetup(int8_t direction) {
  if (setupField == SetupField::Temperature) {
    const int16_t next =
        settings.cycle.targetTempC +
        direction * AppConfig::kPrototypeTempStepC;
    settings.cycle.targetTempC =
        constrain(next, AppConfig::kPrototypeTempMinC,
                  AppConfig::kPrototypeTempMaxC);
  } else if (setupField == SetupField::Duration) {
    const int32_t next =
        static_cast<int32_t>(settings.cycle.durationMinutes) +
        direction *
            static_cast<int32_t>(AppConfig::kPrototypeDurationStepMinutes);
    settings.cycle.durationMinutes = static_cast<uint16_t>(constrain(
        next,
        static_cast<int32_t>(AppConfig::kPrototypeDurationMinMinutes),
        static_cast<int32_t>(AppConfig::kPrototypeDurationMaxMinutes)));
  }
}

static void adjustSettings(int8_t direction, uint32_t nowMs) {
  if (settingsField == SettingsField::BuzzerVolume) {
    int value = static_cast<int>(settings.buzzerVolume) + direction;
    value = constrain(value, static_cast<int>(BuzzerVolume::Off),
                      static_cast<int>(BuzzerVolume::High));
    settings.buzzerVolume = static_cast<BuzzerVolume>(value);
    applyBuzzerSettings();
    buzzer.playVolumePreview(nowMs);
  } else {
    settings.keyClick = !settings.keyClick;
    if (settings.keyClick) buzzer.playKeyClick(nowMs);
  }
}

static void maybePlayKeyClick(ButtonEventType type, uint32_t nowMs) {
  if (!settings.keyClick || type == ButtonEventType::Repeat) return;
  buzzer.playKeyClick(nowMs);
}

static void handleButtonEvent(ButtonId id, ButtonEventType type) {
  const uint32_t nowMs = millis();

  if (ui.wakeAndConsumeIfSleeping(nowMs)) {
    Serial.println("OLED wake: button event consumed");
    return;
  }
  ui.noteInteraction(nowMs);

  if (appState != AppState::Fault) {
    maybePlayKeyClick(type, nowMs);
  }

  switch (appState) {
    case AppState::Boot:
      return;

    case AppState::Standby:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        setupField = SetupField::Temperature;
        enterState(AppState::Setup, nowMs);
      } else if (id == ButtonId::Mode &&
                 type == ButtonEventType::LongPress) {
        enterState(AppState::Settings, nowMs);
      } else if (id == ButtonId::OnOff &&
                 type == ButtonEventType::ShortPress) {
        enterState(AppState::Drying, nowMs);
      }
      return;

    case AppState::Setup:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        selectNextSetupField();
      } else if (id == ButtonId::Mode &&
                 type == ButtonEventType::LongPress) {
        settingsStore.save(settings);
        enterState(AppState::Standby, nowMs);
      } else if (id == ButtonId::OnOff &&
                 type == ButtonEventType::ShortPress) {
        enterState(AppState::Drying, nowMs);
      } else if ((id == ButtonId::Up || id == ButtonId::Down) &&
                 (type == ButtonEventType::ShortPress ||
                  type == ButtonEventType::Repeat)) {
        adjustSetup(id == ButtonId::Up ? +1 : -1);
      }
      return;

    case AppState::Settings:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        selectNextSettingsField();
      } else if (id == ButtonId::Mode &&
                 type == ButtonEventType::LongPress) {
        settingsStore.save(settings);
        enterState(AppState::Standby, nowMs);
      } else if ((id == ButtonId::Up || id == ButtonId::Down) &&
                 (type == ButtonEventType::ShortPress ||
                  type == ButtonEventType::Repeat)) {
        adjustSettings(id == ButtonId::Up ? +1 : -1, nowMs);
      }
      return;

    case AppState::Drying:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        technicalPage = !technicalPage;
      } else if (id == ButtonId::OnOff &&
                 type == ButtonEventType::LongPress) {
        enterState(AppState::Standby, nowMs);
      }
      return;

    case AppState::Complete:
      if (id == ButtonId::OnOff &&
          type == ButtonEventType::ShortPress) {
        enterState(AppState::Standby, nowMs);
      }
      return;

    case AppState::Fault:
      if (id == ButtonId::Mode &&
          type == ButtonEventType::ShortPress) {
        buzzer.muteFault();
        Serial.println("Fault buzzer muted; fault remains active");
      }
      return;
  }
}

void setup() {
  safety.begin();
  statusLed.begin();
  buttons.begin();
  fan.begin();

  Wire.begin(AppConfig::kPinSda, AppConfig::kPinScl,
             AppConfig::kI2cFrequencyHz);
  const uint32_t nowMs = millis();
  ui.begin(nowMs);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Filament Dryer Monitor - SAFE BRING-UP application");
  Serial.println("Heater locked OFF. Fan test control enabled.");

  const bool settingsOk = settingsStore.begin(settings);
  if (!settingsOk) {
    Serial.println("WARNING: Preferences/NVS unavailable; using defaults");
  }

  sht45.begin(nowMs);
  buzzer.begin();
  applyBuzzerSettings();

  bootStartedMs = nowMs;
  appState = AppState::Boot;
  bootWordmarkChimeStarted = false;  // Roots grow in silence.

  if (!fan.attached()) {
    Serial.println("WARNING: fan LEDC attachment failed; fan remains OFF");
  }

  ui.render(nowMs, appState, setupField, settingsField, settings,
            sht45.snapshot(), 0, fan.dutyPercent(), false,
            nowMs - bootStartedMs);
}

void loop() {
  const uint32_t nowMs = millis();

  safety.update();

  buttons.update(nowMs, handleButtonEvent);
  buzzer.update(nowMs);
  sht45.update(nowMs);
  fan.update(nowMs, safety.fanPermitted());

  if (appState == AppState::Boot) {
    const uint32_t elapsedMs = nowMs - bootStartedMs;
    if (elapsedMs >= AppConfig::kBootSplashMs) {
      enterState(AppState::Standby, nowMs);
    } else if (!bootWordmarkChimeStarted &&
               elapsedMs >= AppConfig::kBootWordmarkStartMs) {
      bootWordmarkChimeStarted = true;
      ui.requestRender(nowMs);  // Make the wordmark appear in this loop pass.
      buzzer.playStartup(nowMs);
    }
  }

  if (cycle.update(nowMs) == CycleEvent::Completed) {
    enterState(AppState::Complete, nowMs);
  }

  if (safety.faultActive() && appState != AppState::Fault) {
    enterState(AppState::Fault, nowMs);
  }

  const bool sensorHealthy = sht45.healthy(nowMs);
  DiagnosticLedState ledState = DiagnosticLedState::Boot;
  if (appState == AppState::Fault) {
    ledState = DiagnosticLedState::Fault;
  } else if (appState == AppState::Boot) {
    ledState = DiagnosticLedState::Boot;
  } else if (!sensorHealthy) {
    ledState = DiagnosticLedState::Warning;
  } else {
    ledState = DiagnosticLedState::Normal;
  }
  statusLed.update(nowMs, ledState);

  ui.updateSleep(nowMs, appState);
  ui.render(nowMs, appState, setupField, settingsField, settings,
            sht45.snapshot(), cycle.remainingSeconds(nowMs),
            fan.dutyPercent(), technicalPage, nowMs - bootStartedMs);
}
