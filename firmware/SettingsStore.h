#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "AppConfig.h"
#include "AppTypes.h"

class SettingsStore {
 public:
  bool begin(PersistentSettings &settings) {
    opened_ = prefs_.begin("dryer", false);
    if (!opened_) return false;

    const uint16_t schema = prefs_.getUShort("schema", 0);
    if (schema != AppConfig::kSettingsSchemaVersion) {
      settings = PersistentSettings{};
      clamp(settings);
      save(settings);
      return true;
    }

    settings.cycle.targetTempC =
        prefs_.getShort("tempC", settings.cycle.targetTempC);
    settings.cycle.durationMinutes =
        prefs_.getUShort("durMin", settings.cycle.durationMinutes);

    uint8_t volume = prefs_.getUChar(
        "buzzVol", static_cast<uint8_t>(settings.buzzerVolume));
    if (volume > static_cast<uint8_t>(BuzzerVolume::High)) {
      volume = static_cast<uint8_t>(BuzzerVolume::Low);
    }
    settings.buzzerVolume = static_cast<BuzzerVolume>(volume);
    settings.keyClick = prefs_.getBool("keyClick", settings.keyClick);

    clamp(settings);
    cached_ = settings;
    cacheValid_ = true;
    return true;
  }

  void save(const PersistentSettings &settingsIn) {
    if (!opened_) return;

    PersistentSettings settings = settingsIn;
    clamp(settings);

    if (cacheValid_ && equal(settings, cached_)) return;

    prefs_.putUShort("schema", AppConfig::kSettingsSchemaVersion);
    prefs_.putShort("tempC", settings.cycle.targetTempC);
    prefs_.putUShort("durMin", settings.cycle.durationMinutes);
    prefs_.putUChar("buzzVol", static_cast<uint8_t>(settings.buzzerVolume));
    prefs_.putBool("keyClick", settings.keyClick);

    cached_ = settings;
    cacheValid_ = true;
  }

 private:
  Preferences prefs_;
  bool opened_ = false;
  bool cacheValid_ = false;
  PersistentSettings cached_{};

  static void clamp(PersistentSettings &settings) {
    settings.cycle.targetTempC = constrain(
        settings.cycle.targetTempC,
        AppConfig::kPrototypeTempMinC,
        AppConfig::kPrototypeTempMaxC);

    settings.cycle.durationMinutes = static_cast<uint16_t>(constrain(
        static_cast<int32_t>(settings.cycle.durationMinutes),
        static_cast<int32_t>(AppConfig::kPrototypeDurationMinMinutes),
        static_cast<int32_t>(AppConfig::kPrototypeDurationMaxMinutes)));
  }

  static bool equal(const PersistentSettings &a,
                    const PersistentSettings &b) {
    return a.cycle.targetTempC == b.cycle.targetTempC &&
           a.cycle.durationMinutes == b.cycle.durationMinutes &&
           a.buzzerVolume == b.buzzerVolume &&
           a.keyClick == b.keyClick;
  }
};
