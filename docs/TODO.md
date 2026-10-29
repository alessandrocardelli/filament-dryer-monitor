# Next work

## One priority: verify STEP11D firmware and qualify the heater-plate thermal envelope — NO NEW HEAT

**Current source:** branch `experiment/step10-heater-diagnostics-air25-2026-10-28` on the single draft [PR #18](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). New build identity **`HOT-261029-S11D25` / OLED `H29-S11D`**. This is the next firmware source, **NOT yet compiled with verified CI, flashed or run physically**. `main` remains HEATER OFF.

**What it actually changes:** new `firmware/HeaterPlateDiagnostics.h` (read-only NTC margin and historical 7-point open-box Sanwa contact proxy), `/api/status` raw NTC margin above SOFT45k and HARD40k, session minimum NTC ohms, historical proxy Celsius when in measured range, and OLED technical page raw NTC distance to HARD40k. Host test in `firmware/tests/host/test_heater_plate_diagnostics.cpp`. **No changed heater actuator, limits, watchdog or TCO**; under the same air/NTC conditions STEP11D cannot heat the plate beyond the previous STEP10B envelope. The historical proxy MUST NOT be used as a safety thermometer or HEATER command. Do not confuse it with Sanwa actual measurement in a closed dryer.

**Why:** STEP10B reached AIR25.10°C after21 ON fronts, but Sanwa PLATE plateau38–39°C and SOFT45kΩ caused brief ON windows. NTC min43.738kΩ gives just3.738kΩ above HARD40kΩ. S7U previously tripped at40k with Sanwa43°C; S6C reached53°C around30k in different setup; historical static open/fan-OFF calibration gives NTC42.27k↔Sanwa39°C and28.72k↔47°C but is not suitable to safely select CLOSED/fan100 thresholds. [Original STEP10B CSV and Sanwa analysis](evidence/thermal/2026-10-29-STEP10B-PHYSICAL-SESSION.md). 25°C reached *after* final heater OFF, no controlled ON→OFF at25 or rearm at24 observed. Repeat of identical STEP10B not useful.

**NOW (software test gate):** `bash firmware/tests/host/run_step09_feedback.sh`, `bash firmware/tests/host/run_step08_real_trace.sh` plus
`g++ -std=c++17 -Wall -Wextra -Werror -pedantic firmware/tests/host/test_heater_plate_diagnostics.cpp -o /tmp/step11d-plate-test && /tmp/step11d-plate-test`; compile actual sketch `arduino-cli compile --fqbn esp32:esp32:esp32 firmware` with ESP32 core3.3.12 and U8g2 2.36.19 using [existing STEP11D workflow](../.github/workflows/step10-heater-diagnostic-ci.yml). The new workflow verifies unchanged guard NTC HARD40k/SOFT45k/REARM60k, ON20s/ISR20.5s, OFF12s, total300s, 720s, fan100%. Host g++ **small diagnostic source mirror** passed locally; source-matched workflow/ESP32 CI result **NOT independently confirmed**, and must be checked before firmware installation. Save actual run ID/logs/firmware SHA if available and distinguish PASS from projected.

**THEN (one engineering decision):** choose any higher plate candidate ONLY after comparing NTC-Sanwa thermal lag/dynamics from archived data, enclosure/materials/fan100, hard trip safety margin and installed TCO105. A higher plateau cannot be achieved by an arbitrary soft threshold change near an unvalidated hard40k guard; changing a protection or physical HEATER behavior requires a reviewed numeric proposal and explicit agreement. **No physical test, no new HEATER-on flash, no TCO bypass authorized yet.** If accepted, create a bounded replacement on this SAME branch with preflight + Sanwa on actual PLATE (known position), independent physical STOP and captured 15-col raw, not a repeat of STEP10B.

## Backlog

- Fully qualified heater temperature vs chamber40/50/55°C, thermostat OFF-at-target and ON-after-air-hysteresis while cycle remains active, independent cutoff.
- Autonomous V1 user duration, fan AUTO/fault/STOP, robustness of CSV/OTA and post-heat cooling.
- PCB hardware revision only with new verified evidence, not by rewriting old fabricated KiCad.

See [PROJECT_STATE](PROJECT_STATE.md) and [THERMAL_EXPERIMENT_LEDGER](THERMAL_EXPERIMENT_LEDGER.md).
