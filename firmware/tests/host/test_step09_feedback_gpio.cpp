// STEP09B: actual SafetyController HEADERS, GPIO mocked. No physical heater.
// Fixed 25C SHT45 air target; NTC-ohm soft pause above unchanged 40k fault.
#include <cassert>
#include <cstdint>
#include <iostream>
int highWrites = 0, lowWrites = 0, lastPinState = -1;
#include "../../SafetyController.h"

static NtcSnapshot ntc(uint32_t t, float r = 100000.0f) {
  NtcSnapshot n{};
  n.sampled = true; n.valid = true; n.fault = NtcFault::None;
  n.lastSampleMs = t; n.resistanceOhms = r;
  return n;
}
static SensorSnapshot air(uint32_t t, float c = 18.0f) {
  SensorSnapshot a{};
  a.valid = true; a.temperatureC = c; a.lastSuccessMs = t;
  return a;
}
static void tick(SafetyController &s, uint32_t t, float r = 100000.0f,
                 float c = 18.0f, bool fan = true) {
  s.update(t, ntc(t, r), air(t, c), true, fan, t, 25.0f);
}
static void start(SafetyController &s) {
  s.begin();
  assert(!s.heaterOn() && lastPinState == LOW);
  tick(s, 0, 100000, 18, false);
  tick(s, 1000);
  tick(s, 5999);
  assert(!s.heaterOn());
  tick(s, 6000);
  assert(s.heaterOn());
}

int main() {
  static_assert(!AppConfig::kHeaterCommissioningMode,
                "Do not run old fixed pulse mode simultaneously");
  static_assert(AppConfig::kHeaterStep09TrialMode,
                "Expected isolated HEAT-capable STEP09 trial branch");
  static_assert(!AppConfig::kHeaterOutputEnabled &&
                !AppConfig::kHeaterNtcModelApproved &&
                !AppConfig::kHeaterProfileApproved,
                "Production regulator MUST remain disabled");
  static_assert(AppConfig::kFanBringupRunPercent == 100,
                "Supervised STEP09 fan baseline must be 100%");
  static_assert(HeaterStep09AirFeedback::kHardwareWindowCeilingUs == 20500000UL,
                "Do not loosen the 20.5s ISR ceiling");
  const auto settings = HeaterStep09AirFeedback::parameters();
  assert(settings.targetAirC == 25.0f &&
         settings.ntcSoftPauseOhms == 50000.0f &&
         settings.ntcResumeOhms == 65000.0f &&
         settings.ntcHardTripOhms == 40000.0f &&
         settings.maximumOnWindowMs == 20000 &&
         settings.minimumOffMs == 12000 &&
         settings.maximumTotalHeatMs == 200000 &&
         settings.maximumRunMs == 600000 &&
         settings.cooldownMs == 90000);
  {
    SafetyController s; start(s);
    tick(s, 12500, 50000.0f); // SOFT pause, still >= hard 40k
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.commissioningPhase() ==
           static_cast<uint8_t>(HeaterCommissioning::Phase::Gap));
    tick(s, 24499, 90000);
    assert(!s.heaterOn()); // 12s minimum OFF
    tick(s, 24500, 65000);
    assert(s.heaterOn() && !s.faultActive());
    tick(s, 25000, 64000, 25.0f); // chamber reaches target
    assert(!s.heaterOn() && !s.faultActive());
    tick(s, 37000, 70000, 24.5f);
    assert(!s.heaterOn()); // hysteresis inhibits restart
    tick(s, 37250, 70000, 24.0f);
    assert(s.heaterOn()); // chamber has cooled to <=24C
    s.stop();
    assert(!s.heaterOn() && !s.commissioningCanStart());
    assert(s.coolingRequired(37251) && s.fanPermitted());
  }
  {
    SafetyController s; start(s);
    tick(s, 6100, 40000.0f);
    assert(!s.heaterOn() && s.faultActive());
    assert(s.commissioningFault() ==
        static_cast<uint8_t>(HeaterCommissioning::Fault::NtcResistanceLimit));
    assert(s.coolingRequired(6101) && s.fanPermitted());
    const int n = highWrites;
    tick(s, 19000, 100000);
    assert(highWrites == n && !s.heaterOn()); // persistent FAULT
  }
  {
    SafetyController s; start(s);
    tick(s, 6010, 90000, 40.0f);
    assert(s.faultActive() && !s.heaterOn()); // chamber ceiling
  }
  {
    SafetyController s; start(s);
    tick(s, 6010, 90000, 18, false);
    assert(s.faultActive() && !s.heaterOn()); // fan command lost
  }
  {
    SafetyController s; start(s);
    NtcSnapshot n = ntc(6010);
    n.valid = false; n.fault = NtcFault::InvalidReading;
    s.update(6010, n, air(6010), true, true, 6010, 25);
    assert(s.faultActive() && !s.heaterOn()); // sensor invalid
  }
  {
    SafetyController s; start(s);
    NtcSnapshot n = ntc(6010);
    n.lastSampleMs = 4000; // > 1.5s stale
    s.update(6010, n, air(6010), true, true, 6010, 25);
    assert(s.faultActive() && !s.heaterOn()); // stale NTC
  }
  {
    SafetyController s; start(s);
    SensorSnapshot a = air(6010);
    a.lastSuccessMs = 1; // > 5s stale
    s.update(6010, ntc(6010), a, true, true, 6010, 25);
    assert(s.faultActive() && !s.heaterOn()); // stale SHT45
  }
  {
    SafetyController s; start(s);
    // S9B 10 bounded windows * 20s = 200s maximum ON, separated
    // by >=12s rests. This is the ONLY increased heat limit from S9A.
    const uint32_t on[] = {6000,38000,70000,102000,134000,166000,
                           198000,230000,262000,294000};
    for (uint32_t t : on) {
      assert(s.heaterOn());
      tick(s, t + 20000);
      assert(!s.heaterOn() && !s.faultActive());
      if (t != 294000) {
        tick(s, t + 32000);
        assert(s.heaterOn());
      }
    }
    assert(!s.commissioningComplete());
    assert(s.coolingRequired(314001));
    tick(s, 403999);
    assert(!s.commissioningComplete());
    tick(s, 404001);
    assert(s.commissioningComplete() && !s.heaterOn());
    assert(!s.commissioningCanStart());
  }
  // The 25C air cutoff is reached *after* >120s cumulative HEAT,
  // the situation S9A could not test before its old 120s budget expired.
  {
    SafetyController s; start(s);
    const uint32_t firstSevenOn[] = {6000,38000,70000,102000,134000,
                                     166000,198000};
    for (uint32_t t : firstSevenOn) {
      assert(s.heaterOn());
      tick(s, t + 20000, 90000, 22.0f);
      assert(!s.heaterOn() && !s.faultActive());
      if (t != 198000) {
        tick(s, t + 32000, 90000, 22.0f);
        assert(s.heaterOn());
      }
    }
    tick(s, 230000, 90000, 24.0f);
    assert(s.heaterOn() && s.commissioningPhase() == 2);
    // This OFF must be caused by the SHT45 chamber reaching 25 C,
    // rather than local NTC or end-of-budget shutdown.
    tick(s, 230250, 90000, 25.0f);
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.commissioningPhase() == 3);
    tick(s, 242249, 90000, 24.5f);
    assert(!s.heaterOn());
    tick(s, 242500, 90000, 24.0f);
    assert(s.heaterOn() && !s.faultActive());
    s.stop();
    assert(!s.heaterOn() && s.coolingRequired(242501));
  }
  {
    SafetyController s; start(s);
    tick(s, 606000); // hard overall time bound regardless of air
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.coolingRequired(606001));
  }
  std::cout << "PASS STEP09 actual SafetyController GPIO-MOCK: real ON, "
      "SHT45 25C feedback/hysteresis, NTC soft50k/hard40k, "
      "20s/12s windows, 200s cumulative heat cap, STOP, "
      "cooldown, stale/fault/no restart. NO REAL GPIO.\n";
}
