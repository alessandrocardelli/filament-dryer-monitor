#!/usr/bin/env bash
set -euo pipefail
# Run from repository root: bash firmware/tests/host/run_heater_tests.sh
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/SafetyController.h" "$ROOT/firmware/HeaterControlPolicy.h" \
   "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
   "$ROOT/firmware/HeaterPulseCommissioning.h" \
   "$ROOT/firmware/HeaterStage05Candidate.h" \
   "$ROOT/firmware/HeaterStep06Candidate.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_safety_integration.cpp" "$TMP/firmware/tests/host/"
# The original regression tests exercise the normal (heater-locked) profile.
# The new experimental pulse controller has its OWN dedicated 12-test suite.
sed -i 's/kHeaterCommissioningMode = true/kHeaterCommissioningMode = false/' "$TMP/firmware/AppConfig.h"

cat > "$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT = 1, HIGH = 1, LOW = 0;
extern int highWrites, lowWrites, lastPinState;
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t value) {
  if (value == HIGH) ++highWrites;
  else ++lowWrites;
  lastPinState = value;
}
HDR
FLAGS=(-std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP/firmware")
g++ "${FLAGS[@]}" "$TMP/firmware/tests/host/test_safety_integration.cpp" \
    -o "$TMP/integrated-default"
"$TMP/integrated-default"

# Synthetic thermal data in a TEMPORARY COPY only, never production config.
python3 - "$TMP/firmware/AppConfig.h" <<'PY'
from pathlib import Path
import sys
p = Path(sys.argv[1])
s = p.read_text()
replacements = {
    "kHeaterOutputEnabled = false": "kHeaterOutputEnabled = true",
    "kHeaterProfileApproved = false": "kHeaterProfileApproved = true",
    "kHeaterNtcModelApproved = false": "kHeaterNtcModelApproved = true",
    "kHeaterMaxMeasuredC = 0.0f": "kHeaterMaxMeasuredC = 90.0f",
    "kChamberMaxMeasuredC = 0.0f": "kChamberMaxMeasuredC = 60.0f",
    "kChamberControlHysteresisC = 0.0f": "kChamberControlHysteresisC = 1.0f",
    "kHeaterNtcMaxAgeMs = 0": "kHeaterNtcMaxAgeMs = 2000",
    "kHeaterChamberMaxAgeMs = 0": "kHeaterChamberMaxAgeMs = 10000",
    "kHeaterFanPrerunMs = 0": "kHeaterFanPrerunMs = 1000",
    "kHeaterMaxContinuousOnMs = 0": "kHeaterMaxContinuousOnMs = 5000",
    "kHeaterMaxCycleMs = 0": "kHeaterMaxCycleMs = 60000",
}
for old, new in replacements.items():
    if s.count(old) != 1:
        raise SystemExit("Expected exactly one fixture anchor: " + old)
    s = s.replace(old, new)
p.write_text(s)
PY
g++ "${FLAGS[@]}" -DENABLE_APPROVED_FIXTURE \
    "$TMP/firmware/tests/host/test_safety_integration.cpp" \
    -o "$TMP/integrated-approved-SIMULATION"
"$TMP/integrated-approved-SIMULATION"

g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
    "$ROOT/firmware/tests/host/test_heater_policy.cpp" -o "$TMP/policy"
"$TMP/policy"

g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
    "$ROOT/firmware/tests/host/test_heater_pulse.cpp" -o "$TMP/pulse"
"$TMP/pulse"
