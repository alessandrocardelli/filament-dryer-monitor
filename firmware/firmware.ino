#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "AppConfig.h"
#include "AppTypes.h"
#include "ButtonManager.h"
#include "Buzzer.h"
#include "CycleController.h"
#include "FanController.h"
#include "FirmwareBuild.h"
#include "NtcSensor.h"
#include "ThermalLog.h"
#include "NetworkService.h"
#include "SafetyController.h"
#include "SanwaReminder.h"
#include "SettingsStore.h"
#include "Sht45Sensor.h"
#include "StatusLed.h"
#include "Ui.h"

U8G2_SSD1309_128X64_NONAME0_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

ButtonManager buttons;
Buzzer buzzer;
CycleController cycle;
FanController fan;
NtcSensor ntc;
SafetyController safety;
SanwaReminder sanwaReminder;
SettingsStore settingsStore;
Sht45Sensor sht45(Wire);
StatusLed statusLed;
Ui ui(oled);
ThermalLog thermalLog;
NetworkService network(thermalLog);

AppState appState = AppState::Boot;
SetupField setupField = SetupField::Temperature;
SettingsField settingsField = SettingsField::BuzzerVolume;
PersistentSettings settings;

uint32_t bootStartedMs = 0;
uint32_t lastNtcLogMs = 0;
uint32_t lastCommissionCsvMs = 0;
uint32_t previousLoopStartMs = 0;
bool technicalPage = false;
bool loggingSession = false;
// The previous /last-test.csv belongs to an earlier trial until this session
// has actually finished cooldown and been saved successfully.
bool csvSavedThisRun = false;
bool csvFinalSavedThisRun = false;
bool csvCheckpointAttempted = false;
bool csvSaveFailed = false;
uint8_t previousPhaseReported = 255;
// Read-only telemetry: capture raw minimum during the active session.
// The historical contact-temperature proxy NEVER drives this controller.
float minSessionNtcOhms = NAN;

static void applyBuzzerSettings() {
  buzzer.setVolume(settings.buzzerVolume);
}

static void enterState(AppState next, uint32_t nowMs) {
  // Operator measurement clock is completely independent of heater commands.
  // STOP and normal timer expiry start the 30/60/90/120s post-OFF prompts.
  if (AppConfig::kHeaterStep09TrialMode) {
    if (next == AppState::Fault || next == AppState::Boot)
      sanwaReminder.cancel();
    else if (next == AppState::Drying)
      sanwaReminder.start(nowMs);
    else if (appState == AppState::Drying &&
             (next == AppState::Standby || next == AppState::Complete))
      sanwaReminder.stop(nowMs);
  }
  if (next == AppState::Complete && appState == AppState::Drying)
    safety.completeUserCycle(nowMs); // timer expired: clean end + cooldown
  else if (next != AppState::Drying)
    safety.stop(); // manual STOP, FAULT or BOOT: GPIO19 immediately LOW
  if (appState == AppState::Drying && next != AppState::Drying) {
    cycle.stop();
    fan.stop();
  }

  appState = next;
  technicalPage = false;
  ui.noteInteraction(nowMs);

  switch (next) {
    case AppState::Standby:
      if (safety.coolingRequired(nowMs)) fan.startAuto(nowMs);
      else fan.stop();
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
      thermalLog.start();
      loggingSession = true;
      csvSavedThisRun = false;
      csvFinalSavedThisRun = false;
      csvCheckpointAttempted = false;
      csvSaveFailed = false;
      minSessionNtcOhms = NAN;
      // Legacy STEP13 trial remains available in archived history only;
      // current V1 source runs user-configured 40..55C and 15min..12h.
      settingsStore.save(settings);
      cycle.start(settings.cycle, nowMs); // user-selected air temperature/time
      fan.startAuto(nowMs);
      buzzer.playCycleStart(nowMs);
      if (AppConfig::kHeaterCommissioningMode) {
        Serial.println("STATE -> HEATER STEP07 GUARD: 6 x 20 s MAX, NTC 40k abort, fan 100%");
        Serial.println("HOT_CSV,ms,phase,pulses,heater,NTC_kOhm,air_C,RH,fan_pct");
      } else if (AppConfig::kHeaterStep09TrialMode) {
        Serial.print("ATTENDED HOT: AIR "); Serial.print(settings.cycle.targetTempC);
        Serial.print("C duration "); Serial.print(settings.cycle.durationMinutes);
        Serial.println("min; NTC SOFT9.81/REARM13.73/HARD7.12k, FAN100, NO energy budget");
        Serial.println("HOT_CSV,ms,phase,windows,heater,NTC_kOhm,air_C,RH,fan_pct");
      } else {
        Serial.println(safety.heaterLocked()
            ? "STATE -> DRYING (fan active; heater output locked OFF)"
            : "STATE -> DRYING (heater regulated by air and NTC limits)");
      }
      break;
    case AppState::Complete:
      if (safety.coolingRequired(nowMs)) fan.startAuto(nowMs);
      else fan.stop();
      buzzer.playComplete(nowMs);
      Serial.println("STATE -> COMPLETE");
      break;
    case AppState::Fault:
      if (safety.coolingRequired(nowMs)) fan.startAuto(nowMs);
      else fan.stop();
      // Supervised experiment: beep once and stay electrically FAULT-latched.
      // The normal product-mode repeating D022 alarm is unchanged.
      if (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode)
        buzzer.playCommissionFaultOnce(nowMs);
      else
        buzzer.startFaultAlarm(nowMs);
      Serial.println("STATE -> FAULT: HEATER OFF; watch Sanwa during cooldown");
      break;
    case AppState::Boot:
      fan.stop();
      break;
  }
  static const char *const kStateText[] = {
    "STATE BOOT", "STATE STANDBY", "STATE SETUP", "STATE SETTINGS",
    "STATE DRYING", "STATE COMPLETE", "STATE FAULT"
  };
  network.note(kStateText[static_cast<uint8_t>(next)]);
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
        if ((AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode) &&
            !safety.commissioningCanStart(nowMs)) {
          Serial.println("Heater pulse test already used. Power-cycle to retry.");
        } else {
          enterState(AppState::Drying, nowMs);
        }
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
        if ((AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode) &&
            !safety.commissioningCanStart(nowMs)) {
          Serial.println("Heater pulse test already used. Power-cycle to retry.");
        } else {
          enterState(AppState::Drying, nowMs);
        }
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
  Serial.println("Filament Dryer Monitor - integrated heater-control firmware");
  Serial.print("FW BUILD: ");
  Serial.println(FirmwareBuild::kId);
  Serial.print("FW MODE: ");
  Serial.println(FirmwareBuild::kMode);
  if (AppConfig::kHeaterCommissioningMode) {
    Serial.println("EXPERIMENTAL 6-PULSE STEP07 NTC-GUARD TEST; no auto-repeat");
    Serial.println("NTC absolute degrees NOT calibrated; experimental raw R_NTC abort at 40k");
  } else if (AppConfig::kHeaterStep09TrialMode) {
    Serial.println("ATTENDED HOT: AIR40..55 selected; LOCAL NTC 9.81k/13.73k/7.12k research-only, fan100, selected duration");
  } else {
    Serial.println(safety.heaterLocked()
        ? "Heater OUTPUT LOCKED: configuration/calibration not approved."
        : "Heater regulation ENABLED: supervised qualification required.");
  }

  const bool settingsOk = settingsStore.begin(settings);
  if (!settingsOk) {
    Serial.println("WARNING: Preferences/NVS unavailable; using defaults");
  }

  ntc.begin(nowMs);
  sht45.begin(nowMs);
  buzzer.begin();
  applyBuzzerSettings();
  thermalLog.begin();  // mounts CSV partition, does not write while heating
  network.setIdeOtaStartSafetyCallback([]() {
    // Defense in depth even though IDE OTA is serviced only in STANDBY.
    // Never allow the updater to operate with GPIO19 enabled.
    safety.stop();
    fan.stop();
  });
  network.begin();     // non-blocking Wi-Fi STA or random-key setup AP
  network.note(FirmwareBuild::kId);

  appState = AppState::Boot;  // Roots grow in silence.

  if (!fan.attached()) {
    Serial.println("WARNING: fan LEDC attachment failed; fan remains OFF");
  }

  // ui.begin() already displayed the initial rootless frame. Do not start
  // the BOOT timer before OLED initialization or Preferences setup: doing so
  // consumed most of the nominal initial hold before the user could see it.
  bootStartedMs = millis();
}

void loop() {
  const uint32_t nowMs = millis();
  const uint32_t loopGapMs = previousLoopStartMs ? nowMs - previousLoopStartMs : 0;
  previousLoopStartMs = nowMs;

  ntc.update(nowMs);
  sht45.update(nowMs);

  // Once an OTA image is accepted, no new heating cycle may start before reboot.
  if (!network.updateBusy()) buttons.update(nowMs, handleButtonEvent);
  buzzer.update(nowMs);
  fan.update(nowMs, safety.fanPermitted());
  // After STOP / COMPLETE / FAULT keep the fan only for required post-heat
  // cooldown. Previously it could remain ON indefinitely in standby, or
  // stop immediately after a continuous-heater fault. No fan on cold boot.
  if (appState != AppState::Drying && !safety.coolingRequired(nowMs) &&
      fan.phase() != FanPhase::Off) {
    fan.stop();
  }

  if (appState == AppState::Boot &&
      (nowMs - bootStartedMs) >= AppConfig::kBootSplashMs) {
    enterState(AppState::Standby, nowMs);
  }

  if (cycle.update(nowMs) == CycleEvent::Completed) {
    enterState(AppState::Complete, nowMs);
  }

  // Raw NTC low-water mark is observable even at post-ON thermal overshoot.
  // Does not influence output or soften the independent hard NTC cutoff.
  if (loggingSession && ntc.snapshot().valid &&
      std::isfinite(ntc.snapshot().resistanceOhms) &&
      (std::isnan(minSessionNtcOhms) ||
       ntc.snapshot().resistanceOhms < minSessionNtcOhms)) {
    minSessionNtcOhms = ntc.snapshot().resistanceOhms;
  }

  // V1 target is the user's AIR setpoint; local NTC temperature limits
  // need independent calibration and approval before GPIO19 is unlocked.
  safety.update(nowMs, ntc.snapshot(), sht45.snapshot(),
                appState == AppState::Drying && cycle.isRunning(),
                fan.attached() && fan.phase() == FanPhase::Running,
                cycle.elapsedMs(nowMs),
                static_cast<float>(settings.cycle.targetTempC));

  if (safety.faultActive() && appState != AppState::Fault) {
    char event[60];
    snprintf(event, sizeof(event), "FAULT ntc=%u pulse=%u",
             static_cast<unsigned>(safety.ntcFault()),
             static_cast<unsigned>(safety.commissioningFault()));
    network.note(event);
    if (AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode) {
      Serial.print("HEATER_TEST_ABORT fault=");
      Serial.println(safety.commissioningFault());
    }
    enterState(AppState::Fault, nowMs);
  }
  if ((AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode) &&
      safety.commissioningComplete() && appState == AppState::Drying) {
    Serial.println("HEATER_TEST_DONE: all pulses and cooldown completed");
    enterState(AppState::Complete, nowMs);
  }

  // Nonblocking prompt: defer behind any currently active buzzer sound.
  // FAULT already cancelled reminders in enterState; never mask its alarm.
  bool sanwaCue = false;
  if (AppConfig::kHeaterStep09TrialMode &&
      appState != AppState::Fault && sanwaReminder.due(nowMs) &&
      buzzer.tryPlaySanwaReminder(nowMs)) {
    sanwaReminder.consume(nowMs);
    sanwaCue = true;
    Serial.print("SANWA_READ @ms=");
    Serial.println(nowMs);
    network.note("SANWA READ NOW");
  }

  if (loggingSession &&
      (appState == AppState::Drying || safety.coolingRequired(nowMs) ||
       sanwaReminder.active() || sanwaCue)) {
    thermalLog.sample(nowMs, sht45.snapshot(), ntc.snapshot(),
                      safety.heaterOn(), fan.dutyPercent(),
                      safety.commissioningPhase(),
                      safety.commissioningPulsesDone(),
                      safety.commissioningFault(),
                      static_cast<uint8_t>(safety.ntcFault()),
                      loopGapMs, sanwaCue);
  } else if (loggingSession) {
    // Finalize after cooldown, preserving all samples. The first checkpoint
    // already replaced the old CSV at irrevocable heater shutdown.
    loggingSession = false;
    // If no checkpoint was possible (e.g. STOP before heating), do one
    // final safe write. If checkpoint previously failed, successful final
    // write is still a success. Never infer it from an OLD saved file.
    csvFinalSavedThisRun = thermalLog.dirty()
        ? thermalLog.saveWhenSafe() : csvSavedThisRun;
    csvSavedThisRun = csvSavedThisRun || csvFinalSavedThisRun;
    csvSaveFailed = !csvFinalSavedThisRun;
    Serial.println(csvFinalSavedThisRun ?
        "FINAL CSV stored: /last-test.csv" :
        "Final CSV save failed; previous checkpoint may still exist");
    network.note(csvFinalSavedThisRun ? "NEW CSV FINAL READY" : "NEW CSV FINAL SAVE FAILED");
  }
  // Persist a bounded snapshot ONCE when the heater has irreversibly stopped,
  // not during a normal NTC/air OFF gap when it may energize again. No FLASH
  // writes are allowed while HEATER can restart within this session.
  const auto completedHeatPhase = static_cast<HeaterCommissioning::Phase>(
      safety.commissioningPhase());
  const bool heaterIrreversiblyOff =
      completedHeatPhase == HeaterCommissioning::Phase::Cooldown ||
      completedHeatPhase == HeaterCommissioning::Phase::Aborted ||
      completedHeatPhase == HeaterCommissioning::Phase::Complete;
  if (loggingSession && !csvCheckpointAttempted && !safety.heaterOn() &&
      heaterIrreversiblyOff) {
    csvCheckpointAttempted = true;
    csvSavedThisRun = thermalLog.saveWhenSafe();
    if (csvSavedThisRun) {
      network.note("NEW CSV CHECKPOINT (HEATER LATCHED OFF)");
      Serial.println("CSV checkpoint persisted before cooldown completion");
    } else {
      network.note("CSV CHECKPOINT FAILED (RAM still available)");
      csvSaveFailed = true;
    }
  }

  if ((AppConfig::kHeaterCommissioningMode || AppConfig::kHeaterStep09TrialMode) &&
      appState == AppState::Drying &&
      (nowMs - lastCommissionCsvMs) >= 250) {
    lastCommissionCsvMs = nowMs;
    const NtcSnapshot &n = ntc.snapshot();
    const SensorSnapshot &a = sht45.snapshot();
    Serial.print("HOT_CSV,"); Serial.print(nowMs);
    Serial.print(','); Serial.print(safety.commissioningPhase());
    Serial.print(','); Serial.print(safety.commissioningPulsesDone());
    Serial.print(','); Serial.print(safety.heaterOn() ? 1 : 0);
    Serial.print(',');
    if (n.valid) Serial.print(n.resistanceOhms / 1000.0f, 2);
    else Serial.print("NA");
    Serial.print(',');
    if (a.valid) Serial.print(a.temperatureC, 2);
    else Serial.print("NA");
    Serial.print(',');
    if (a.valid) Serial.print(a.humidityRh, 2);
    else Serial.print("NA");
    Serial.print(','); Serial.println(fan.dutyPercent());
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

  if ((nowMs - lastNtcLogMs) >= 2000) {
    lastNtcLogMs = nowMs;
    const NtcSnapshot &n = ntc.snapshot();
    Serial.print("NTC adc=");
    Serial.print(n.reportedMv, 1);
    Serial.print("mV corr=");
    Serial.print(n.correctedMv, 1);
    Serial.print("mV R=");
    if (n.valid) {
      Serial.print(n.resistanceOhms / 1000.0f, 2);
      // No NTC Celsius estimate is shown: its local thermal model is unverified.
      Serial.print(" kOhm heater=");
      Serial.print(safety.heaterLocked() ? "LOCKED" :
                   (safety.heaterOn() ? "ON" : "OFF"));
      Serial.print(" safetyFault=");
      Serial.println(static_cast<int>(safety.heaterFault()));
    } else {
      Serial.print("-- fault=");
      Serial.print(static_cast<int>(n.fault));
      Serial.print(" heater=OFF safetyFault=");
      Serial.println(static_cast<int>(safety.heaterFault()));
    }
  }

  const uint8_t currentPhase = safety.commissioningPhase();
  if (currentPhase != previousPhaseReported) {
    previousPhaseReported = currentPhase;
    static const char *const kPhaseText[] = {
      "PULSE IDLE", "FAN PRERUN", "HEATER ON", "HEATER GAP",
      "COOLDOWN", "PULSE COMPLETE", "PULSE ABORTED"
    };
    if (currentPhase < sizeof(kPhaseText) / sizeof(kPhaseText[0])) {
      if (AppConfig::kHeaterStep09TrialMode) {
        static const char* const kFeedbackText[] = {
          "AIR FEEDBACK IDLE", "FAN PRERUN", "AIR HEAT ON",
          "NTC/AIR PAUSE", "COOLDOWN", "FEEDBACK COMPLETE", "FEEDBACK ABORTED"
        };
        network.note(kFeedbackText[currentPhase]);
      } else network.note(kPhaseText[currentPhase]);
    }
  }

  NetworkService::Status telem{};
  telem.ms = nowMs;
  const auto &airNow = sht45.snapshot();
  const auto &ntcNow = ntc.snapshot();
  telem.airValid = airNow.valid;
  telem.airC = airNow.valid ? airNow.temperatureC : 0;
  telem.rh = airNow.valid ? airNow.humidityRh : 0;
  telem.ntcValid = ntcNow.valid;
  telem.ntcOhms = ntcNow.valid ? ntcNow.resistanceOhms : 0;
  telem.heaterCommand = safety.heaterOn();
  telem.fanPercent = fan.dutyPercent();
  telem.cycle = appState == AppState::Drying;
  telem.fault = appState == AppState::Fault;
  telem.phase = safety.commissioningPhase();
  telem.pulses = safety.commissioningPulsesDone();
  telem.pulseFault = safety.commissioningFault();
  telem.waitReason = safety.feedbackWaitReason(nowMs, ntcNow.resistanceOhms);
  telem.cumulativeHeatMs = safety.feedbackTotalHeatMs();
  telem.sessionCsvSaved = csvSavedThisRun;
  telem.sessionCsvFinal = csvFinalSavedThisRun;
  telem.minSessionNtcOhms = minSessionNtcOhms;
  network.publish(telem);
  const bool canOta = appState == AppState::Standby && !safety.faultActive() &&
       !cycle.isRunning() && !safety.coolingRequired(nowMs) && !safety.heaterOn();
  network.tick(nowMs, !safety.heaterOn(), canOta);

  ui.updateSleep(nowMs, appState);
  const bool wordmarkJustRevealed =
      ui.render(nowMs, appState, setupField, settingsField, settings,
                sht45.snapshot(), ntc.snapshot(),
                cycle.remainingSeconds(nowMs), fan.dutyPercent(),
                technicalPage, nowMs - bootStartedMs,
                safety.heaterLocked(), safety.heaterOn(),
                safety.commissioningPhase(),
                safety.commissioningPulsesDone(),
                safety.commissioningFault(),
                network.setupApActive() ? network.setupSsid() : nullptr,
                network.setupApActive() ? network.setupPass() : nullptr,
                network.connected() ? network.address().c_str() : nullptr,
                safety.coolingSecondsRemaining(nowMs),
                csvSavedThisRun, csvSaveFailed,
                safety.feedbackWaitReason(nowMs, ntc.snapshot().resistanceOhms),
                sanwaReminder.active(),
                sanwaReminder.secondsUntilNext(nowMs),
                sanwaReminder.stage() == SanwaReminder::Stage::AfterStop);
  if (wordmarkJustRevealed) {
    // Trigger the first note only after the fully refreshed wordmark is visible.
    buzzer.playStartup(millis());
  }
}
