#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include "AppTypes.h"

// Bounded RAM recorder: no flash writes while the heater can be commanded ON.
// After a test and its cooldown, save a CSV snapshot in LittleFS. The next
// flash or OTA update preserves it if the partition layout stays unchanged.
class ThermalLog {
 public:
  struct Sample {
    uint32_t ms = 0;
    float airC = 0, humidity = 0, ntcOhms = 0;
    uint8_t phase = 0, pulses = 0, heater = 0, fan = 0, valid = 0;
  };
  // 350 seconds x 4 samples/s: capture the entire ~276-second STEP06B
  // sequence + 90-second cooldown, even when callbacks delay some records.
  // Do not write flash while the heater can be commanded HIGH.
  static constexpr size_t kCapacity = 1400; // 350 s at 250 ms
  void begin() {
    // On the first clean prototype install there is no LittleFS signature.
    mounted_ = LittleFS.begin(false);
    if (!mounted_) mounted_ = LittleFS.begin(true);
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
              uint8_t fanPercent, uint8_t phase, uint8_t pulses) {
    if (count_ > 0 && nowMs - lastSampleMs_ < 250) return;
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
    head_ = (head_ + 1) % kCapacity;
    if (count_ < kCapacity) ++count_;
  }
  bool saveWhenSafe() {
    if (!dirty_ || !mounted_ || count_ == 0) return false;
    // A failed or interrupted save never destroys the previous completed
    // snapshot: write temp, close, then replace only after success.
    File f = LittleFS.open("/pending.csv", "w");
    if (!f) return false;
    f.println("ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent");
    char row[112];
    for (size_t i = 0; i < count_; ++i) {
      const Sample &s = at(i);
      const int n = snprintf(row, sizeof(row),
          "%lu,%.2f,%.2f,%.1f,%u,%u,%u,%u,%u,%u\n",
          (unsigned long)s.ms, s.airC, s.humidity, s.ntcOhms,
          (unsigned)(s.valid & 1), (unsigned)((s.valid >> 1) & 1),
          (unsigned)s.phase, (unsigned)s.pulses, (unsigned)s.heater, (unsigned)s.fan);
      if (n <= 0 || n >= (int)sizeof(row) || f.write((const uint8_t*)row, n) != (size_t)n) {
        f.close(); LittleFS.remove("/pending.csv"); return false;
      }
    }
    f.close();
    LittleFS.remove("/last-test.csv");
    if (!LittleFS.rename("/pending.csv", "/last-test.csv")) return false;
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
    return "ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent\n";
  }
  // Used for optional live HTTP CSV export. Memory-only, bounded.
  void writeRow(Print &p, size_t i) const {
    if (i >= count_) return;
    const Sample &s = at(i);
    p.printf("%lu,%.2f,%.2f,%.1f,%u,%u,%u,%u,%u,%u\n",
      (unsigned long)s.ms,s.airC,s.humidity,s.ntcOhms,
      (unsigned)(s.valid & 1),(unsigned)((s.valid >> 1) & 1),
      (unsigned)s.phase,(unsigned)s.pulses,(unsigned)s.heater,(unsigned)s.fan);
  }
 private:
  Sample records_[kCapacity]{};
  size_t head_ = 0, count_ = 0;
  uint32_t lastSampleMs_ = 0;
  bool mounted_ = false, dirty_ = false;
};
