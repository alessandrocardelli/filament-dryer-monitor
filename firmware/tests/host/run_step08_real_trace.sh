#!/usr/bin/env bash
set -euo pipefail
# STEP08 host replay. Does NOT build/flash heater-enabled firmware or connect GPIO.
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
INPUT="$ROOT/docs/evidence/thermal/2026-10-26-S7U-retest-raw.csv"
echo "STEP08 real S7U CSV + real HeaterPulseCommissioning and HeaterControlPolicy headers"
echo "RAW INPUT: $INPUT"
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic \
  "$ROOT/firmware/tests/host/test_step08_real_trace.cpp" \
  -o "$TMP/step08-replay-NO-GPIO"
"$TMP/step08-replay-NO-GPIO" "$INPUT"
echo "PASS: STEP08 host replay completed; thermal safety G3 still OPEN"
