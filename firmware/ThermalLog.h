#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "AppTypes.h"
#include "AppConfig.h"

// Bounded RAM recorder: no flash writes while the heater can be commanded ON.
// At irreversible HEATER shutdown save a checkpoint in LittleFS, then
// overwrite it with full cooldown evidence at normal session completion. The next
// flash or OTA update preserves it if the partition layout stays unchanged.
class ThermalLog {
 public:
  struct Sample {
    uint32_t ms = 0;
    float airC = 0, humidity = 0, ntcOhms = 0;
    uint32_t airAgeMs = UINT32_MAX, ntcAgeMs = UINT32_MAX, loopGapMs = 0;
    uint8_t phase = 0, pulses = 0, heater = 0, fan = 0, valid = 0;
    uint8_t pulseFault = 0, ntcFault = 0;
  };
  // Keep the original 1400-entry RAM budget, REQUIRED for ESP32 DRAM link.
  // AIR55 attended study: baseline every 5s plus HEATER/phase/fault edges.
  // A first <=1h session usually fits 1400 RAM records. Longer sessions
  // are a tail-retaining ring, NOT falsely reported as full-session evidence.
  // NTC safety ADC remains 500ms; logs never drive heater protection.
  // Prior S7U standalone logging remains at 250ms.
  // Never write flash while the heater may be commanded ON again.
  static constexpr size_t kCapacity = 1400;
  void begin() {
    // On the first clean prototype install there is no LittleFS signature.
    mounted_ = LittleFS.begin(false);
    if (!mounted_) mounted_ = LittleFS.begin(true);
    // Recover previous known-good file if power failed during replacement.
    if (mounted_ && !LittleFS.exists("/last-test.csv") &&
        LittleFS.exists("/previous-test.csv"))
      LittleFS.rename("/previous-test.csv", "/last-test.csv");
    Serial.println(mounted_ ? "LittleFS mounted" : "LittleFS unavailable");
  }
  void start() {
    count_ = 0;
    head_ = 0;
    lastSampleMs_ = 0;
    dirty_ = true;
  }
  void sample(uint32_t nowMs, const SensorSnapshot &air,
              const NtcSnapshot &ntc, bool heaterOn,
              uint8_t fanPercent, uint8_t phase, uint8_t pulses,
              uint8_t pulseFault, uint8_t ntcFault, uint32_t loopGapMs) {
    const uint32_t intervalMs = AppConfig::kHeaterStep09TrialMode ? 5000U : 250U;
    if (count_ > 0 && nowMs - lastSampleMs_ < intervalMs) {
      const Sample &prev = at(count_ - 1);
      if (prev.phase == phase && prev.heater == (heaterOn ? 1 : 0) &&
          prev.pulses == pulses && prev.pulseFault == pulseFault &&
          prev.ntcFault == ntcFault) return;
    }
    lastSampleMs_ = nowMs;
    Sample &s = records_[head_];
    s.ms = nowMs;
    s.airC = air.valid ? air.temperatureC : 0;
    s.humidity = air.valid ? air.humidityRh : 0;
    s.ntcOhms = ntc.valid ? ntc.resistanceOhms : 0;
    s.phase = phase; s.pulses = pulses;
    s.heater = heaterOn ? 1 : 0;
    s.fan = fanPercent;
    s.valid = (air.valid ? 1 : 0) | (ntc.valid ? 2 : 0);
    s.airAgeMs = air.lastSuccessMs ? nowMs - air.lastSuccessMs : UINT32_MAX;
    s.ntcAgeMs = ntc.sampled ? nowMs - ntc.lastSampleMs : UINT32_MAX;
    s.loopGapMs = loopGapMs;
    s.pulseFault = pulseFault;
    s.ntcFault = ntcFault;
    dirty_ = true; // a post-checkpoint cooldown sample requires final resave
    head_ = (head_ + 1) % kCapacity;
    if (count_ < kCapacity) ++count_;
  }
  bool saveWhenSafe() {
    if (!dirty_ || !mounted_ || count_ == 0) return false;
    // A failed or interrupted save never destroys the previous completed
    // snapshot: write temp, close, then replace only after success.
    File f = LittleFS.open("/pending.csv", "w");
    if (!f) return false;
    f.print(header());
    char row[192];
    for (size_t i = 0; i < count_; ++i) {
      const Sample &s = at(i);
      const int n = snprintf(row, sizeof(row),
          "%lu,%.2f,%.2f,%.1f,%u,%u,%u,%u,%u,%u,%u,%u,%lu,%lu,%lu\n",
          (unsigned long)s.ms, s.airC, s.humidity, s.ntcOhms,
          (unsigned)(s.valid & 1), (unsigned)((s.valid >> 1) & 1),
          (unsigned)s.phase, (unsigned)s.pulses, (unsigned)s.heater, (unsigned)s.fan,
          (unsigned)s.pulseFault, (unsigned)s.ntcFault,
          (unsigned long)s.airAgeMs, (unsigned long)s.ntcAgeMs,
          (unsigned long)s.loopGapMs);
      if (n <= 0 || n >= (int)sizeof(row) || f.write((const uint8_t*)row, n) != (size_t)n) {
        f.close(); LittleFS.remove("/pending.csv"); return false;
      }
    }
    f.close();
    // Replace with rollback: never silently destroy the previous verified
    // session if the new rename fails or power is lost between two renames.
    const bool hadOld = LittleFS.exists("/last-test.csv");
    if (hadOld) {
      LittleFS.remove("/previous-test.csv");
      if (!LittleFS.rename("/last-test.csv", "/previous-test.csv"))
        return false;
    }
    if (!LittleFS.rename("/pending.csv", "/last-test.csv")) {
      if (hadOld) LittleFS.rename("/previous-test.csv", "/last-test.csv");
      return false;
    }
    if (hadOld) LittleFS.remove("/previous-test.csv");
    dirty_ = false;
    return true;
  }
  bool mounted() const { return mounted_; }
  bool hasSaved() const { return mounted_ && LittleFS.exists("/last-test.csv"); }
  bool dirty() const { return dirty_; }
  size_t size() const { return count_; }
  const Sample &at(size_t i) const {
    return records_[(head_ + kCapacity - count_ + i) % kCapacity];
  }
  String header() const {
    return "ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent,pulse_fault,ntc_fault,air_age_ms,ntc_age_ms,loop_gap_ms\n";
  }
  // Used for optional live HTTP CSV export. Memory-only, bounded.
  void writeRow(Print &p, size_t i) const {
    if (i >= count_) return;
    const Sample &s = at(i);
    p.printf("%lu,%.2f,%.2f,%.1f,%u,%u,%u,%u,%u,%u,%u,%u,%lu,%lu,%lu\n",
      (unsigned long)s.ms,s.airC,s.humidity,s.ntcOhms,
      (unsigned)(s.valid & 1),(unsigned)((s.valid >> 1) & 1),
      (unsigned)s.phase,(unsigned)s.pulses,(unsigned)s.heater,(unsigned)s.fan,
      (unsigned)s.pulseFault,(unsigned)s.ntcFault,
      (unsigned long)s.airAgeMs,(unsigned long)s.ntcAgeMs,(unsigned long)s.loopGapMs);
  }
 private:
  Sample records_[kCapacity]{};
  size_t head_ = 0, count_ = 0;
  uint32_t lastSampleMs_ = 0;
  bool mounted_ = false, dirty_ = false;
};
