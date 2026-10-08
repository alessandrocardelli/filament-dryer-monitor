#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "AppConfig.h"
#include "AppTypes.h"
#include "ButtonManager.h"
#include "Buzzer.h"
#include "Sht45Sensor.h"
#include "StatusLed.h"
#include "Ui.h"

// Exact constructor previously validated on the first article on 2026-10-08.
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

ButtonManager buttons;
Buzzer buzzer;
Sht45Sensor sht45(Wire);
StatusLed statusLed;
Ui ui(oled);

AppState appState = AppState::Boot;
SetupField setupField = SetupField::Temperature;
CycleSettings cycleSettings;
uint32_t bootStartedMs = 0;
uint32_t cycleStartedMs = 0;
bool technicalPage = false;

static void enforceSafeBringupOutputs() {
  // Do not remove this guard while heater NTC conversion/fault handling and the
  // independent series TCO are still open project gates.
  digitalWrite(AppConfig::kPinHeaterPwm, LOW);
  digitalWrite(AppConfig::kPinFanPwm, LOW);
}

static uint32_t remainingCycleSeconds(uint32_t nowMs) {
  if (appState != AppState::Drying) return 0;
  const uint32_t totalSeconds = static_cast<uint32_t>(cycleSettings.durationMinutes) * 60UL;
  const uint32_t elapsedSeconds = (nowMs - cycleStartedMs) / 1000UL;
  return elapsedSeconds >= totalSeconds ? 0 : totalSeconds - elapsedSeconds;
}

static void enterState(AppState next, uint32_t nowMs) {
  appState = next;
  technicalPage = false;
  ui.noteInteraction(nowMs);

  switch (next) {
    case AppState::Standby:
      Serial.println("STATE -> STANDBY");
      break;
    case AppState::Setup:
      Serial.println("STATE -> SETUP");
      break;
    case AppState::Drying:
      cycleStartedMs = nowMs;
      buzzer.playCycleStart(nowMs);
      Serial.println("STATE -> DRYING (SAFE BRING-UP: outputs locked OFF)");
      break;
    case AppState::Complete:
      buzzer.playComplete(nowMs);
      Serial.println("STATE -> COMPLETE");
      break;
    case AppState::Fault:
      Serial.println("STATE -> FAULT");
      break;
    case AppState::Boot:
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

static void adjustSelected(int8_t direction) {
  if (setupField == SetupField::Temperature) {
    const int16_t next = cycleSettings.targetTempC + direction * AppConfig::kPrototypeTempStepC;
    cycleSettings.targetTempC = constrain(next, AppConfig::kPrototypeTempMinC,
                                           AppConfig::kPrototypeTempMaxC);
  } else if (setupField == SetupField::Duration) {
    const int32_t next = static_cast<int32_t>(cycleSettings.durationMinutes) +
                         direction * static_cast<int32_t>(AppConfig::kPrototypeDurationStepMinutes);
    cycleSettings.durationMinutes = static_cast<uint16_t>(constrain(
        next, static_cast<int32_t>(AppConfig::kPrototypeDurationMinMinutes),
        static_cast<int32_t>(AppConfig::kPrototypeDurationMaxMinutes)));
  }
  // Fan remains AUTO in V1; no user-adjustable percentage here.
}

static void handleButtonEvent(ButtonId id, ButtonEventType type) {
  const uint32_t nowMs = millis();

  if (ui.wakeAndConsumeIfSleeping(nowMs)) {
    Serial.println("OLED wake: button event consumed");
    return;
  }
  ui.noteInteraction(nowMs);

  switch (appState) {
    case AppState::Boot:
      // Ignore front-panel commands during the short branding splash.
      return;

    case AppState::Standby:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        setupField = SetupField::Temperature;
        enterState(AppState::Setup, nowMs);
      } else if (id == ButtonId::OnOff && type == ButtonEventType::ShortPress) {
        enterState(AppState::Drying, nowMs);
      }
      return;

    case AppState::Setup:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        selectNextSetupField();
      } else if (id == ButtonId::OnOff && type == ButtonEventType::ShortPress) {
        enterState(AppState::Drying, nowMs);
      } else if (id == ButtonId::OnOff && type == ButtonEventType::LongPress) {
        enterState(AppState::Standby, nowMs);
      } else if ((id == ButtonId::Up || id == ButtonId::Down) &&
                 (type == ButtonEventType::ShortPress || type == ButtonEventType::Repeat)) {
        adjustSelected(id == ButtonId::Up ? +1 : -1);
      }
      return;

    case AppState::Drying:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        technicalPage = !technicalPage;
      } else if (id == ButtonId::OnOff && type == ButtonEventType::LongPress) {
        enterState(AppState::Standby, nowMs);
      }
      return;

    case AppState::Complete:
      if (id == ButtonId::OnOff && type == ButtonEventType::ShortPress) {
        enterState(AppState::Standby, nowMs);
      }
      return;

    case AppState::Fault:
      if (id == ButtonId::Mode && type == ButtonEventType::ShortPress) {
        buzzer.stop();
        Serial.println("Fault buzzer muted; fault remains active");
      }
      return;
  }
}

void setup() {
  // Establish safe actuator states before display, I2C, Serial, or UI startup.
  pinMode(AppConfig::kPinHeaterPwm, OUTPUT);
  pinMode(AppConfig::kPinFanPwm, OUTPUT);
  enforceSafeBringupOutputs();

  statusLed.begin();
  buttons.begin();

  Serial.begin(115200);
  Serial.println();
  Serial.println("Filament Dryer Monitor - SAFE BRING-UP UI firmware");
  Serial.println("Heater and fan outputs are hard-locked LOW in this build.");

  Wire.begin(AppConfig::kPinSda, AppConfig::kPinScl, AppConfig::kI2cFrequencyHz);

  const uint32_t nowMs = millis();
  ui.begin(nowMs);
  sht45.begin(nowMs);
  buzzer.begin();

  bootStartedMs = nowMs;
  appState = AppState::Boot;
  buzzer.playStartup(nowMs);
  ui.render(nowMs, appState, setupField, cycleSettings, sht45.snapshot(), 0);
}

void loop() {
  const uint32_t nowMs = millis();

  // Safety invariant for this development build.
  enforceSafeBringupOutputs();

  buttons.update(nowMs, handleButtonEvent);
  buzzer.update(nowMs);
  sht45.update(nowMs);

  if (appState == AppState::Boot && (nowMs - bootStartedMs) >= AppConfig::kBootSplashMs) {
    enterState(AppState::Standby, nowMs);
  }

  if (appState == AppState::Drying && remainingCycleSeconds(nowMs) == 0) {
    enterState(AppState::Complete, nowMs);
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
  ui.render(nowMs, appState, setupField, cycleSettings, sht45.snapshot(),
            remainingCycleSeconds(nowMs), technicalPage);
}
