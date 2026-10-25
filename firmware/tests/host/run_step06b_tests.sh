#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
echo "STEP06B: all previously passing native policy, pulse and cooldown regressions"
bash "$ROOT/firmware/tests/host/run_step06_tests.sh"
echo "STEP06B: real new 100%-fan heater-enabled config on mock GPIO"
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/SafetyController.h" "$ROOT/firmware/HeaterControlPolicy.h" \
   "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
   "$ROOT/firmware/HeaterPulseCommissioning.h" \
   "$ROOT/firmware/HeaterStep06Candidate.h" \
   "$ROOT/firmware/HeaterStep06Extended.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_step06b_enabled_gpio.cpp" "$TMP/firmware/tests/host/"
cat >"$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT = 1, HIGH = 1, LOW = 0;
extern int highWrites, lowWrites, lastPinState;
inline void pinMode(uint8_t, uint8_t){}
inline void digitalWrite(uint8_t, uint8_t v){
  if(v==HIGH)++highWrites;else ++lowWrites;
  lastPinState=v;
}
HDR
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP/firmware" \
    "$TMP/firmware/tests/host/test_step06b_enabled_gpio.cpp" -o "$TMP/extended"
"$TMP/extended"
echo "STEP06B: inline dashboard stale/phase string tests"
node "$ROOT/firmware/tests/host/test_web_freshness.js"
echo "PASS: full Step06B FAN100 + 6x20s native fixture suite; no physical heat"
