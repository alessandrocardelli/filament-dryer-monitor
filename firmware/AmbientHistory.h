#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <esp_system.h>
#include <cstdio>
#include "AppTypes.h"

// 12h x 1 sample/minute, independent of the bounded 250ms pulse trace.
// Every 10 new samples (plus the first one), persist a *full* ring snapshot
// into one of TWO alternating LittleFS banks. A torn/partial write cannot
// invalidate the preceding completed bank. No FS writes during a cycle.
class AmbientHistory {
 public:
  struct Row {
    uint32_t bootId = 0, uptimeMs = 0;
    float airC = 0, humidity = 0, ntcOhms = 0;
    uint8_t valid = 0, fan = 0, heater = 0;
  };
  static constexpr size_t kCapacity = 720;
  static constexpr uint32_t kPeriodMs = 60000UL;
  static constexpr uint8_t kSaveEvery = 10;

  void begin(bool filesystemMounted) {
    mounted_ = filesystemMounted;
    bootId_ = esp_random();
    if (!mounted_) return;
    uint32_t a = 0, b = 0;
    const bool okA = readBank("/ambient-a.csv", false, a);
    const bool okB = readBank("/ambient-b.csv", false, b);
    if (okA || okB) {
      const char *chosen = (!okB || (okA && newer(a, b))) ?
                            "/ambient-a.csv" : "/ambient-b.csv";
      uint32_t gen = 0;
      if (readBank(chosen, true, gen)) generation_ = gen;
    }
    Serial.printf("AMBIENT history: %u rows, snapshot generation %lu\n",
                  (unsigned)count_, (unsigned long)generation_);
  }

  void capture(uint32_t nowMs, const SensorSnapshot &air,
               const NtcSnapshot &ntc, uint8_t fan, bool heater) {
    if (!air.valid && !ntc.valid) return;
    if (sampledBoot_ && (nowMs - lastSampleMs_) < kPeriodMs) return;
    sampledBoot_ = true;
    lastSampleMs_ = nowMs;
    Row &r = rows_[head_];
    r.bootId = bootId_;
    r.uptimeMs = nowMs;
    r.airC = air.valid ? air.temperatureC : 0;
    r.humidity = air.valid ? air.humidityRh : 0;
    r.ntcOhms = ntc.valid ? ntc.resistanceOhms : 0;
    r.valid = (air.valid ? 1 : 0) | (ntc.valid ? 2 : 0);
    r.fan = fan;
    r.heater = heater ? 1 : 0;
    head_ = (head_ + 1) % kCapacity;
    if (count_ < kCapacity) ++count_;
    if (dirtyCount_ < kSaveEvery) ++dirtyCount_;
  }

  bool saveIfDue(bool safeToWrite) {
    if (!safeToWrite || !mounted_ || !dirtyCount_ ||
        (generation_ && dirtyCount_ < kSaveEvery)) return false;
    const uint32_t nextGen = generation_ + 1;
    const char *target = (nextGen & 1u) ? "/ambient-a.csv" : "/ambient-b.csv";
    File f = LittleFS.open(target, "w");
    if (!f) return false;

    char line[128];
    const int firstLen = snprintf(line, sizeof(line), "#GEN,%lu\n",
                                  (unsigned long)nextGen);
    bool ok = firstLen > 0 && firstLen < (int)sizeof(line) &&
              f.write((const uint8_t*)line, firstLen) == (size_t)firstLen;
    uint32_t hash = 2166136261u;
    for (size_t i=0; ok && i<count_; ++i) {
      const Row &r = at(i);
      const int n = snprintf(line, sizeof(line),
          "%lu,%lu,%.2f,%.2f,%.1f,%u,%u,%u\n",
          (unsigned long)r.bootId, (unsigned long)r.uptimeMs,
          r.airC, r.humidity, r.ntcOhms,
          (unsigned)r.valid, (unsigned)r.fan, (unsigned)r.heater);
      if (n <= 0 || n >= (int)sizeof(line) ||
          f.write((const uint8_t*)line,n) != (size_t)n) {
        ok = false;
        break;
      }
      for (int j=0; j<n; ++j) hash = (hash ^ (uint8_t)line[j]) * 16777619u;
    }
    const int tail = snprintf(line, sizeof(line),
        "#END,%lu,%u,%lu\n",
        (unsigned long)nextGen, (unsigned)count_, (unsigned long)hash);
    if (!ok || tail <= 0 || tail >= (int)sizeof(line) ||
        f.write((const uint8_t*)line, tail) != (size_t)tail) ok = false;
    f.flush();
    if (f.getWriteError()) ok = false;
    f.close();

    uint32_t verifiedGen=0;
    if (!ok || !readBank(target, false, verifiedGen) || verifiedGen != nextGen) {
      LittleFS.remove(target);
      return false;
    }
    generation_ = nextGen;
    dirtyCount_ = 0;
    return true;
  }

  static const char *csvHeader() {
    return "boot_id,uptime_ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,fan_percent,heater_command\n";
  }
  size_t size() const { return count_; }
  bool saved() const { return mounted_ && generation_ != 0; }
  uint32_t generation() const { return generation_; }
  uint8_t pending() const { return dirtyCount_; }
  const Row &at(size_t i) const {
    return rows_[(head_ + kCapacity - count_ + i) % kCapacity];
  }
  void writeCsvRow(Print &p, size_t i) const {
    const Row &r = at(i);
    p.printf("%lu,%lu,%.2f,%.2f,%.1f,%u,%u,%u,%u\n",
       (unsigned long)r.bootId, (unsigned long)r.uptimeMs,
       r.airC, r.humidity, r.ntcOhms,
       (unsigned)(r.valid & 1u), (unsigned)((r.valid >> 1u) & 1u),
       (unsigned)r.fan, (unsigned)r.heater);
  }

 private:
  Row rows_[kCapacity]{};
  size_t head_ = 0, count_ = 0;
  uint32_t bootId_ = 0, lastSampleMs_ = 0, generation_ = 0;
  uint8_t dirtyCount_ = 0;
  bool mounted_ = false, sampledBoot_ = false;

  static bool newer(uint32_t a, uint32_t b) {
    return (int32_t)(a - b) > 0;
  }
  bool readBank(const char *path, bool apply, uint32_t &gen) {
    File f = LittleFS.open(path, "r");
    if (!f) return false;
    String line = f.readStringUntil('\n');
    unsigned long g = 0;
    bool ok = sscanf(line.c_str(), "#GEN,%lu", &g) == 1 && g != 0;
    uint32_t hash = 2166136261u;
    size_t count = 0;
    bool foundTail = false;
    // Always validate the entire file before selecting a bank.
    if (apply) { head_ = 0; count_ = 0; }
    while (ok && f.available()) {
      line = f.readStringUntil('\n');
      if (line.endsWith("\r")) line.remove(line.length()-1);
      if (line.startsWith("#END,")) {
        unsigned long tailGen = 0, tailCount = 0, tailHash = 0;
        foundTail = sscanf(line.c_str(), "#END,%lu,%lu,%lu",
                           &tailGen,&tailCount,&tailHash) == 3 &&
                    tailGen == g && tailCount == count &&
                    tailHash == hash;
        // A valid bank must terminate exactly here.
        if (f.available()) foundTail = false;
        break;
      }
      unsigned long bid=0,ms=0;
      float tc=0,rh=0,ohms=0;
      unsigned valid=0,fan=0,heater=0;
      if (count>=kCapacity ||
          sscanf(line.c_str(), "%lu,%lu,%f,%f,%f,%u,%u,%u",
                 &bid,&ms,&tc,&rh,&ohms,&valid,&fan,&heater) != 8 ||
          valid>3u || fan>100u || heater>1u) {
        ok = false;
        break;
      }
      // The writer hashes the complete wire line, including LF.
      for (size_t i=0; i<line.length(); ++i)
        hash = (hash ^ (uint8_t)line[i]) * 16777619u;
      hash = (hash ^ (uint8_t)'\n') * 16777619u;
      if (apply) {
        Row &r=rows_[head_];
        r.bootId=bid; r.uptimeMs=ms; r.airC=tc; r.humidity=rh;
        r.ntcOhms=ohms; r.valid=(uint8_t)valid;
        r.fan=(uint8_t)fan; r.heater=(uint8_t)heater;
        head_=(head_+1)%kCapacity;
        ++count_;
      }
      ++count;
    }
    f.close();
    if (!ok || !foundTail) {
      if (apply) { count_=0; head_=0; }
      return false;
    }
    gen=(uint32_t)g;
    return true;
  }
};
