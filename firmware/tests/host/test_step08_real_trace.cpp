// STEP08: NEW host-only replay of REAL S7U CSV through real C++ commissioning
// and thermostat REQUEST policy, with explicit mock arbitration.
// NO Arduino / GPIO / ESP32 output or calibrated heater temperature.
// This does NOT claim the production firmware already integrates both modes.
#include "../../HeaterStep07GuardTrial.h"
#include "../../HeaterControlPolicy.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Sample {
  uint32_t ms;
  float air, rh, ntc;
  int air_valid, ntc_valid, phase, pulses, heater, fan;
};

static std::vector<Sample> readCsv(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("Cannot open real S7U CSV: " + path);
  std::string line;
  std::getline(in, line);
  if (!line.empty() && line.back() == '\r') line.pop_back();
  if (line !=
      "ms,air_c,rh,ntc_ohms,air_valid,ntc_valid,phase,pulses,heater_command,fan_percent")
    throw std::runtime_error("Unexpected CSV header");
  std::vector<Sample> result;
  while (std::getline(in, line)) {
    if (line.empty()) continue;
    for (char& c : line) if (c == ',') c = ' ';
    std::istringstream ss(line);
    Sample p{};
    if (!(ss >> p.ms >> p.air >> p.rh >> p.ntc >> p.air_valid >>
          p.ntc_valid >> p.phase >> p.pulses >> p.heater >> p.fan))
      throw std::runtime_error("Invalid data row");
    std::string extra;
    if (ss >> extra) throw std::runtime_error("Unexpected extra field");
    result.push_back(p);
  }
  return result;
}

// The only fictitious heater temperature is a HOST TEST STUB, NOT a
// calibration of actual NTC resistance. Its purpose is to isolate
// "air still demands heat" from "real raw-NTC guard must veto".
static HeaterControl::Limits syntheticThermostatLimits() {
  HeaterControl::Limits limits{};
  limits.approved = true;  // ONLY inside this deleted-on-exit host executable
  limits.maximumHeaterC = 90.0f;   // SYNTHETIC, NOT safety threshold
  limits.maximumChamberC = 60.0f;  // SYNTHETIC, NOT operating approval
  limits.hysteresisC = 1.0f;       // TEST fixture
  limits.maximumNtcAgeMs = 1500;
  limits.maximumChamberAgeMs = 5000;
  limits.minimumFanPrerunMs = 500;
  limits.maximumContinuousHeatMs = 500000;
  limits.maximumCycleMs = 600000;
  return limits;
}

static void injectedFaultTests() {
  const auto params = HeaterStep07GuardTrial::parameters();
  auto good = []() {
    HeaterCommissioning::Inputs x{};
    x.cycleActive = true; x.fanRunning = true;
    x.ntcValid = true; x.ntcResistanceOhms = 136000;
    x.ntcAgeMs = 0; x.airValid = true; x.chamberC = 19; x.airAgeMs = 0;
    return x;
  };
  auto assertFault = [&](HeaterCommissioning::Fault expected, auto mutate) {
    HeaterCommissioning::Sequence seq(params);
    auto input = good();
    assert(!seq.update(0, input));       // fan observed, begin 5s prerun
    assert(seq.update(5000, input));     // bounded ON pulse in software
    mutate(input);
    assert(!seq.update(5250, input));
    assert(seq.fault() == expected);
    assert(seq.phase() == HeaterCommissioning::Phase::Aborted);
    assert(seq.cooldownNeeded(5250));   // hot hardware still needs fan cooldown
    assert(!seq.update(5500, good()));  // never auto-clear by healthy readings
    assert(seq.fault() == expected);
  };
  using F = HeaterCommissioning::Fault;
  assertFault(F::NtcResistanceLimit, [](auto& x){ x.ntcResistanceOhms = 39000; });
  assertFault(F::NtcInvalid, [](auto& x){ x.ntcValid = false; });
  assertFault(F::NtcStale, [](auto& x){ x.ntcAgeMs = 1501; });
  assertFault(F::AirInvalid, [](auto& x){ x.airValid = false; });
  assertFault(F::AirStale, [](auto& x){ x.airAgeMs = 5001; });
  assertFault(F::AirLimit, [](auto& x){ x.chamberC = 40.0f; });
  assertFault(F::FanLost, [](auto& x){ x.fanRunning = false; });
  assertFault(F::ManualStop, [](auto& x){ x.cycleActive = false; });
  std::cout << "PASS: 8 NEW injected abort cases (latched + cooldown)\n";
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: step08-real-replay path/to/S7U-original.csv\n";
    return 2;
  }
  auto rows = readCsv(argv[1]);
  if (rows.size() != 717) throw std::runtime_error("S7U sample count changed");
  assert(rows.front().ms == 6785 && rows.back().ms == 185806);
  HeaterCommissioning::Sequence sequence(HeaterStep07GuardTrial::parameters());
  HeaterControl::Policy thermostat;
  const auto simulatedLimits = syntheticThermostatLimits();

  const uint32_t firstMs = rows.front().ms;
  int physicalOnCommands = 0, predictedOn = 0, aborts = 0, airStillWantsHeatAtTrip = 0;
  uint32_t tripMs = 0;
  for (const auto& row : rows) {
    // Fan Running is NOT logged, only commanded 100%: deterministic 1s
    // kickstart stub matching source; this is an assumed mock, not tach data.
    const bool fanRunningStub = (row.ms >= firstMs + 1000);
    HeaterCommissioning::Inputs ci{};
    ci.cycleActive = true;
    ci.fanRunning = fanRunningStub;
    ci.ntcValid = row.ntc_valid == 1;
    ci.ntcResistanceOhms = row.ntc;
    ci.ntcAgeMs = 0;  // CSV has no sample-age field; do not invent actual age
    ci.airValid = row.air_valid == 1;
    ci.chamberC = row.air;
    ci.airAgeMs = 0;  // same artificial freshness assumption
    const bool pulseRequest = sequence.update(row.ms, ci);

    HeaterControl::Inputs ai{};
    ai.cycleRequested = true;
    ai.fanCommandedRunning = fanRunningStub; // command-state proxy only
    ai.ntcElectricalValid = row.ntc_valid == 1;
    ai.ntcThermallyCalibrated = true; // **HOST FIXTURE ONLY**, false in firmware
    ai.heaterTemperatureC = 35.0f;  // **SYNTHETIC**, not inferred from NTC
    ai.ntcAgeMs = 0; ai.chamberValid = row.air_valid == 1;
    ai.chamberTemperatureC = row.air; // real SHT45 captured
    ai.chamberAgeMs = 0; ai.targetChamberC = 40.0f;
    ai.elapsedCycleMs = row.ms - firstMs;
    const bool airWantsHeat = thermostat.evaluate(row.ms, simulatedLimits, ai);

    // SIMULATED integration priority, not present as two active modes
    // in the actual S7U image. Original commissioning trace is authoritative.
    const bool finalMockRequest =
        pulseRequest && airWantsHeat &&
        sequence.fault() == HeaterCommissioning::Fault::None;
    assert(static_cast<int>(sequence.phase()) == row.phase);
    assert(static_cast<int>(sequence.pulsesDone()) == row.pulses);
    if (static_cast<int>(pulseRequest) != row.heater ||
        static_cast<int>(finalMockRequest) != row.heater) {
      std::cerr << "Replay mismatch ms=" << row.ms
                << " pulse=" << pulseRequest
                << " combined=" << finalMockRequest
                << " observed heater_command=" << row.heater << "\n";
      return 1;
    }
    physicalOnCommands += row.heater;
    predictedOn += finalMockRequest;
    if (sequence.fault() == HeaterCommissioning::Fault::NtcResistanceLimit) {
      ++aborts;
      if (!tripMs) {
        tripMs = row.ms;
        assert(row.ntc <= 40000.0f);
        assert(row.air < 40.0f);
        if (airWantsHeat) ++airStillWantsHeatAtTrip;
        assert(!finalMockRequest);
      }
    }
  }
  assert(physicalOnCommands == 237);
  assert(predictedOn == 237);
  assert(tripMs == 96056);
  assert(aborts == 360);
  assert(airStillWantsHeatAtTrip == 1);
  assert(sequence.fault() == HeaterCommissioning::Fault::NtcResistanceLimit);
  assert(sequence.phase() == HeaterCommissioning::Phase::Aborted);
  assert(sequence.pulsesDone() == 2);
  std::cout << "PASS: 717 REAL samples; sequence and mock-AND commands match "
            << "237 heater_command=1 rows; raw NTC guard trips at ms=" << tripMs
            << " despite AIR request still ON below40C. NO GPIO.\n";
  injectedFaultTests();
  std::cout << "PASS: STEP08 HOST ONLY; no firmware flags/thresholds changed, "
            << "no heater temperature calibration claimed.\n";
}
