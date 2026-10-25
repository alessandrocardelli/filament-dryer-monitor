#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/firmware/tests/host"

echo "STEP05: run all previous heater and cycle tests without physical outputs"
bash "$ROOT/firmware/tests/host/run_step03_tests.sh"

echo "STEP05: separately simulate disabled 4-pulse candidate envelope"
g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  "$ROOT/firmware/tests/host/test_step05_candidate.cpp" \
  -o "$TMP/step05-candidate-OFFLINE"
"$TMP/step05-candidate-OFFLINE"

echo "STEP05: mock real SafetyController in cold and synthetic-thermal fixture"
cp "$ROOT/firmware/SafetyController.h" "$ROOT/firmware/HeaterControlPolicy.h" \
  "$ROOT/firmware/HeaterPulseCommissioning.h" \
  "$ROOT/firmware/HeaterStage05Candidate.h" \
  "$ROOT/firmware/HeaterStep06Candidate.h" \
  "$ROOT/firmware/HeaterStep06Extended.h" \
  "$ROOT/firmware/HeaterStep07GuardTrial.h" \
  "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_step05_cooldown.cpp" "$TMP/firmware/tests/host/"
# Historical Step05 cold-fixture test still requires LOCKED mode.
# Isolate the change to the disposable mock copy only; the experiment's
# actual compiled firmware and separate test_step06_enabled_gpio.cpp are HEAT mode.
sed -i 's/kHeaterCommissioningMode = true/kHeaterCommissioningMode = false/' "$TMP/firmware/AppConfig.h"
cat > "$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT = 1, HIGH = 1, LOW = 0;
extern int highWrites, lowWrites, lastPinState;
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t v) {
  if (v == HIGH) ++highWrites;
  else ++lowWrites;
  lastPinState = v;
}
HDR
FLAGS=(-std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP/firmware")
g++ "${FLAGS[@]}" "$TMP/firmware/tests/host/test_step05_cooldown.cpp" \
  -o "$TMP/step05-physically-locked"
"$TMP/step05-physically-locked"

# Synthetic permissions and numbers exist only in deleted temp workspace.
# Never edit approval flags in the checked-out source.
python3 - "$TMP/firmware/AppConfig.h" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])
s=p.read_text()
replacements={
  "kHeaterOutputEnabled = false":"kHeaterOutputEnabled = true",
  "kHeaterProfileApproved = false":"kHeaterProfileApproved = true",
  "kHeaterNtcModelApproved = false":"kHeaterNtcModelApproved = true",
  "kHeaterMaxMeasuredC = 0.0f":"kHeaterMaxMeasuredC = 90.0f",
  "kChamberMaxMeasuredC = 0.0f":"kChamberMaxMeasuredC = 60.0f",
  "kChamberControlHysteresisC = 0.0f":"kChamberControlHysteresisC = 1.0f",
  "kHeaterNtcMaxAgeMs = 0":"kHeaterNtcMaxAgeMs = 2000",
  "kHeaterChamberMaxAgeMs = 0":"kHeaterChamberMaxAgeMs = 10000",
  "kHeaterFanPrerunMs = 0":"kHeaterFanPrerunMs = 1000",
  "kHeaterMaxContinuousOnMs = 0":"kHeaterMaxContinuousOnMs = 5000",
  "kHeaterMaxCycleMs = 0":"kHeaterMaxCycleMs = 60000",
}
for old,new in replacements.items():
    if s.count(old)!=1:
        raise SystemExit(f"Missing/duplicate fixture anchor: {old}")
    s=s.replace(old,new)
p.write_text(s)
PY
g++ "${FLAGS[@]}" -DSTEP05_HOST_APPROVED_FIXTURE \
  "$TMP/firmware/tests/host/test_step05_cooldown.cpp" \
  -o "$TMP/step05-synthetic-GPIO-only"
"$TMP/step05-synthetic-GPIO-only"

echo "PASS: Step05 cold build mocks, candidate sequence and after-heat cooling"
