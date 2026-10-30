#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
   "$ROOT/firmware/HeaterControlPolicy.h" "$ROOT/firmware/HeaterPulseCommissioning.h" \
   "$ROOT/firmware/HeaterStep07GuardTrial.h" "$ROOT/firmware/HeaterStep09AirFeedback.h" \
   "$ROOT/firmware/HeaterProductAir.h" "$ROOT/firmware/SafetyController.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_step09_feedback_gpio.cpp" "$TMP/firmware/tests/host/"
# Historical STEP13 test fixture only: enable old heater trial in an ISOLATED
# temp copy, never in the committed Arduino IDE source or actual PCB.
# Legacy STEP13 GPIO regression: ONLY the copied test fixture uses old
# 33/50/30k one-shot settings. Actual source uses attended experimental mode.
sed -i 's/feedback_{HeaterStep09AirFeedback::attendedAir55Parameters()}/feedback_{HeaterStep09AirFeedback::parameters()}/' "$TMP/firmware/SafetyController.h"
cat >"$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT=1,HIGH=1,LOW=0;
extern int highWrites,lowWrites,lastPinState;
inline void pinMode(uint8_t,uint8_t){}
inline void digitalWrite(uint8_t,uint8_t x){
  if(x==HIGH)++highWrites; else ++lowWrites;
  lastPinState=x;
}
HDR
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic \
    -I"$TMP/firmware" "$TMP/firmware/tests/host/test_step09_feedback_gpio.cpp" \
    -o "$TMP/step09b-HEATER-GPIO-MOCK"
"$TMP/step09b-HEATER-GPIO-MOCK"
echo "PASS STEP09B native fixture. NO ESP32 connected or heater energized."
