// STEP12P: real SafetyController, GPIO mocked; explicitly revised bounded
// PROVISIONAL NTC envelope 35k/50k/30k, NOT an approved safety thermostat.
// Fixed 25C SHT45 target; old STEP07 40k was merely an early test guard.
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
         settings.ntcSoftPauseOhms == 35000.0f &&
         settings.ntcResumeOhms == 50000.0f &&
         settings.ntcHardTripOhms == 30000.0f &&
         settings.maximumOnWindowMs == 20000 &&
         settings.minimumOffMs == 12000 &&
         settings.maximumTotalHeatMs == 300000 &&
         settings.maximumRunMs == 720000 &&
         settings.cooldownMs == 90000);
  {
    SafetyController s; start(s);
    tick(s, 12500, 35000.0f); // SOFT pause, 5k above candidate backup cutoff 30k
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.commissioningPhase() ==
           static_cast<uint8_t>(HeaterCommissioning::Phase::Gap));
    tick(s, 24499, 90000);
    assert(!s.heaterOn()); // 12s minimum OFF
    tick(s, 24500, 50000);
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
  // STEP10B demand-latch regression retained after STEP12P profile change.
  // Air at 24.6C has NEVER reached the 25C target. After a local NTC
  // pause, air 24.6C must NOT require falling all the way to 24C.
  {
    SafetyController s; start(s);
    tick(s, 12500, 35000.0f, 23.7f); // local soft pause
    assert(!s.heaterOn() && !s.faultActive());
    assert(s.feedbackWaitReason(12501, 35000.0f) == 1); // OFF timer
    tick(s, 24500, 49000.0f, 24.6f); // >=12s, NTC not recovered
    assert(!s.heaterOn());
    assert(s.feedbackWaitReason(24500, 49000.0f) == 2); // NTC
    tick(s, 24600, 50000.0f, 24.6f); // NTC now recovered
    assert(s.heaterOn() && !s.faultActive()); // 24C gate was incorrect
    tick(s, 24700, 60000.0f, 25.0f); // first genuine target cutoff
    assert(!s.heaterOn());
    tick(s, 36700, 70000.0f, 24.5f); // demand off until <=24C
    assert(!s.heaterOn());
    assert(s.feedbackWaitReason(36700, 70000.0f) == 3); // AIR latch
    tick(s, 36800, 70000.0f, 24.0f);
    assert(s.heaterOn() && !s.faultActive());
    s.stop();
    assert(!s.heaterOn() && !s.commissioningCanStart());
  }
  // The regression that MUST have caught the original project mistake:
  // R_NTC40k is no longer an irreversible fault or a soft cut-off.
  // A physically tested previous S6C region is now reachable in the NEW,
  // explicitly unapproved candidate. This is GPIO mock, not real heating.
  {
    SafetyController s; start(s);
    tick(s, 6100, 40000.0f); // old arbitrary STEP07 trip
    assert(s.heaterOn() && !s.faultActive());
    tick(s, 7000, 34999.0f); // NEW soft pause, without persistent fault
    assert(!s.heaterOn() && !s.faultActive());
    tick(s, 18999, 49999.0f);
    assert(!s.heaterOn());
    tick(s, 19000, 50000.0f);
    assert(s.heaterOn() && !s.faultActive());
    s.stop();
    assert(!s.heaterOn());
  }
  {
    SafetyController s; start(s);
    tick(s, 6100, 30000.0f);
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
  // Original S9B regression: SHT45 25C must actually remove a live HEATER
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
    // NTC is a healthy 90kOhm, well above the new35k SOFT pause:
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
  std::cout << "PASS STEP12P actual SafetyController GPIO-MOCK: real ON, "
      "SHT45 25C feedback/hysteresis, NTC soft35k/rearm50k/backup30k, "
      "20s/12s windows, late 25C SHT45 cutoff after >120s ON, new300s cap, STOP, "
      "cooldown, stale/fault/no restart. NO REAL GPIO.\n";
}
