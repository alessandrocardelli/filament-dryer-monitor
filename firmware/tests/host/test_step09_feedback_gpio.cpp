// STEP10A: real SafetyController, GPIO mocked; new bounded local envelope.
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
         settings.ntcSoftPauseOhms == 45000.0f &&
         settings.ntcResumeOhms == 60000.0f &&
         settings.ntcHardTripOhms == 40000.0f &&
         settings.maximumOnWindowMs == 20000 &&
         settings.minimumOffMs == 12000 &&
         settings.maximumTotalHeatMs == 300000 &&
         settings.maximumRunMs == 720000 &&
         settings.cooldownMs == 90000);
  {
    SafetyController s; start(s);
    tick(s, 12500, 45000.0f); // SOFT pause, 5k above unchanged hard 40k
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.commissioningPhase() ==
           static_cast<uint8_t>(HeaterCommissioning::Phase::Gap));
    tick(s, 24499, 90000);
    assert(!s.heaterOn()); // 12s minimum OFF
    tick(s, 24500, 60000);
    assert(s.heaterOn() && !s.faultActive());
    tick(s, 25000, 64000, 25.0f); // chamber reaches target
    assert(!s.heaterOn() && !s.faultActive());
    tick(s, 37000, 70000, 24.5f);
    assert(!s.heaterOn()); // hysteresis inhibits restart
    tick(s, 37250, 70000, 24.0f);
    assert(s.heaterOn()); // chamber has cooled to <=24C
    s.stop();
    assert(s.commissioningFault() == static_cast<uint8_t>(HeaterCommissioning::Fault::ManualStop));
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
    // STEP10A regression: 300s cumulative budget, one-shot guard.
    // Fifteen 20s windows with >=12s OFF, never 300s continuous.
    // MOST IMPORTANT: reaching former S9A budget120s MUST NOT prematurely
    // start cooldown. At180s, no auto restart and 90s fan cooling.
    const uint32_t on[] = {
      6000,38000,70000,102000,134000,166000,198000,230000,
      262000,294000,326000,358000,390000,422000,454000
    };
    for (unsigned index = 0; index < 15; ++index) {
      const uint32_t t = on[index];
      assert(s.heaterOn());
      tick(s, t + 20000);
      assert(!s.heaterOn() && !s.faultActive());
      if (index == 5) { // exactly 120s HEATER commanded in 6 full windows
        assert(s.commissioningPhase() ==
               static_cast<uint8_t>(HeaterCommissioning::Phase::Gap));
        assert(!s.commissioningComplete());
      }
      if (index != 14) {
        tick(s, t + 32000);
        assert(s.heaterOn());
      }
    }
    assert(s.commissioningPhase() ==
           static_cast<uint8_t>(HeaterCommissioning::Phase::Cooldown));
    assert(!s.commissioningComplete());
    assert(s.coolingRequired(474001));
    tick(s, 563999);
    assert(!s.commissioningComplete());
    tick(s, 564001);
    assert(s.commissioningComplete() && !s.heaterOn());
    assert(!s.commissioningCanStart());
  }
  // New S9B regression: SHT45 25C must actually remove a live HEATER
  // command AFTER more than the old 120s S9A total budget were used.
  // This reuses only the original GPIO-mock SafetyController, no device heat.
  {
    SafetyController s; start(s);
    const uint32_t firstSevenOn[] = {
      6000, 38000, 70000, 102000, 134000, 166000, 198000
    };
    for (uint32_t t : firstSevenOn) {
      assert(s.heaterOn());
      tick(s, t + 20000, 90000, 22.0f);
      assert(!s.heaterOn() && !s.faultActive());
      if (t != 198000) {
        tick(s, t + 32000, 90000, 22.0f);
        assert(s.heaterOn());
      }
    }
    assert(!s.commissioningComplete());
    // HEATER can still activate past 120s cumulative command budget.
    tick(s, 230000, 90000, 24.0f);
    assert(s.heaterOn() && s.commissioningPhase() ==
        static_cast<uint8_t>(HeaterCommissioning::Phase::Pulse));
    // NTC is a healthy 90kOhm, well above the 50k SOFT pause:
    // only SHT45 25C can explain this immediate ON->OFF transition.
    tick(s, 230250, 90000, 25.0f);
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.commissioningPhase() ==
        static_cast<uint8_t>(HeaterCommissioning::Phase::Gap));
    tick(s, 242249, 90000, 24.5f);
    assert(!s.heaterOn());
    tick(s, 242500, 90000, 24.0f);
    assert(s.heaterOn() && !s.faultActive());
    s.stop();
    assert(!s.heaterOn() && s.coolingRequired(242501));
  }
  {
    SafetyController s; start(s);
    tick(s, 726000); // hard overall time bound regardless of air
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.coolingRequired(726001));
  }
  std::cout << "PASS STEP09B actual SafetyController GPIO-MOCK: real ON, "
      "SHT45 25C feedback/hysteresis, NTC soft45k/hard40k, "
      "20s/12s windows, late 25C SHT45 cutoff after >120s ON, new300s cap, STOP, "
      "cooldown, stale/fault/no restart. NO REAL GPIO.\n";
}
