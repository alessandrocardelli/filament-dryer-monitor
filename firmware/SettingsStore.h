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
    // Backwards compatible with existing schema 1: missing key is MANUALE.
    settings.cycle.material = MaterialPresets::fromStored(prefs_.getUChar("mat", 0));

    uint8_t volume = prefs_.getUChar(
        "buzzVol", static_cast<uint8_t>(settings.buzzerVolume));
    if (volume > static_cast<uint8_t>(BuzzerVolume::High)) {
      volume = static_cast<uint8_t>(BuzzerVolume::Low);
    }
    settings.buzzerVolume = static_cast<BuzzerVolume>(volume);
    settings.keyClick = prefs_.getBool("keyClick", settings.keyClick);
    // Additive schema-1 NVS keys; preserve existing cycle, profile, sound.
    settings.thermal.ntcPauseC=prefs_.getShort("nPause",0);
    settings.thermal.ntcRearmC=prefs_.getShort("nRearm",0);
    settings.thermal.ntcFaultC=prefs_.getShort("nFault",0);
    settings.thermal.airFaultC=prefs_.getShort("aFault",0);

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
    prefs_.putUChar("mat", static_cast<uint8_t>(settings.cycle.material));
    prefs_.putUChar("buzzVol", static_cast<uint8_t>(settings.buzzerVolume));
    prefs_.putBool("keyClick", settings.keyClick);
    prefs_.putShort("nPause",settings.thermal.ntcPauseC);
    prefs_.putShort("nRearm",settings.thermal.ntcRearmC);
    prefs_.putShort("nFault",settings.thermal.ntcFaultC);
    prefs_.putShort("aFault",settings.thermal.airFaultC);

    cached_ = settings;
    cacheValid_ = true;
  }

 private:
  Preferences prefs_;
  bool opened_ = false;
  bool cacheValid_ = false;
  PersistentSettings cached_{};

  static void clamp(PersistentSettings &settings) {
    settings.cycle.material = MaterialPresets::fromStored(
        static_cast<uint8_t>(settings.cycle.material));
    settings.cycle.targetTempC = constrain(
        settings.cycle.targetTempC,
        AppConfig::kPrototypeTempMinC,
        AppConfig::kPrototypeTempMaxC);

    settings.cycle.durationMinutes = static_cast<uint16_t>(constrain(
        static_cast<int32_t>(settings.cycle.durationMinutes),
        static_cast<int32_t>(AppConfig::kPrototypeDurationMinMinutes),
        static_cast<int32_t>(AppConfig::kPrototypeDurationMaxMinutes)));
    ThermalSettings::sanitize(settings.thermal);
  }

  static bool equal(const PersistentSettings &a,
                    const PersistentSettings &b) {
    return a.cycle.targetTempC == b.cycle.targetTempC &&
           a.cycle.durationMinutes == b.cycle.durationMinutes &&
           a.cycle.material == b.cycle.material &&
           a.buzzerVolume == b.buzzerVolume &&
           a.keyClick == b.keyClick &&
           ThermalSettings::equal(a.thermal,b.thermal);
  }
};
