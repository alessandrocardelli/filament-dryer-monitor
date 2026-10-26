#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
echo "STEP07 UI regression: no physical heat, fake GPIO and LEDC"
mkdir -p "$TMP/firmware/tests/host"
cp "$ROOT/firmware/AppConfig.h" "$ROOT/firmware/AppTypes.h" \
   "$ROOT/firmware/Buzzer.h" "$TMP/firmware/"
cp "$ROOT/firmware/tests/host/test_step07_buzzer.cpp" "$TMP/firmware/tests/host/"
cat >"$TMP/firmware/Arduino.h" <<'HDR'
#pragma once
#include <cstdint>
#include <cmath>
extern uint32_t dutyNow, writeCount;
inline bool ledcAttachChannel(uint8_t, uint32_t, uint8_t, uint8_t){return true;}
inline uint32_t ledcChangeFrequency(uint8_t, uint32_t hz, uint8_t){return hz;}
inline bool ledcWrite(uint8_t, uint32_t duty) {
  dutyNow=duty; ++writeCount; return true;
}
HDR
g++ -std=c++17 -Wall -Wextra -Werror -pedantic -I "$TMP/firmware" \
    "$TMP/firmware/tests/host/test_step07_buzzer.cpp" -o "$TMP/beep"
"$TMP/beep"
python3 "$ROOT/firmware/tests/host/test_step07_ui_source.py"
echo "STEP07: existing real 40k guard suite, physical S6C trace replay, GPIO mock"
bash "$ROOT/firmware/tests/host/run_step07_tests.sh"
