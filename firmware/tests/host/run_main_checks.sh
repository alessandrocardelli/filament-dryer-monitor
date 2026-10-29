#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cat > "$TMP/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
constexpr uint8_t OUTPUT=1, HIGH=1, LOW=0;
extern int highWrites,lowWrites,lastPinState;
inline void pinMode(uint8_t,uint8_t){}
inline void digitalWrite(uint8_t,uint8_t v){ if(v==HIGH)++highWrites;else ++lowWrites;lastPinState=v; }
HDR
for name in test_heater_policy test_thermostat_cycle test_main_locked_gpio; do
  g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP" \
    "$ROOT/firmware/tests/host/$name.cpp" -o "$TMP/$name"
  "$TMP/$name"
done
