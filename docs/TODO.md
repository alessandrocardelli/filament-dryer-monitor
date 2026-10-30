# Next work

## One priority — supervised, meaningful AIR40–55°C heater characterization using HOT-261030-AIR55

**Firmware CREATED on existing experiment branch and HEATER-capable, not physically executed.** Arduino IDE sketch `firmware/firmware.ino` now identifies `HOT-261030-AIR55` / `H30-A55`, with `kHeaterStep09TrialMode=true` on the **experiment branch only**, normal V1 product approval flags remain false, and `main` remains heater-OFF. This is an **ATTENDED EXPERIMENTAL research controller**, not automatic unattended product certification. No new branch or PR. Software test/build [GitHub Actions run #41015088463](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/41015088463) and later source-matched run following normal-expiry correction must be verified PASS before any flashing for heat. Never cite a former cold V1 PASS as proof of this hot build.

**WHAT / WHY:** In previous physically completed STEP12P, chamber AIR25°C reached+400.232s, thermostat cut off and rearmed at AIR24°C+684.979s, min NTC34.48kΩ, heater ON231.114s total; its NTC35/50k and 720s/300s budgets were provisional test bounds, NOT heater capability. The open-box/fan-OFF [seven-point Sanwa study](NTC_CALIBRATION_ANALYSIS.md) already recorded *near-heater PLATE contact* 64°C at R13.73k, 74°C at R9.81k, 85°C at R7.12k. These points DO NOT define bead thermistor Celsius nor guarantee closed-box thermals. The aim is ONE real, attended characterization at a useful **user-selected AIR target up to55°C**, observing heat-up, plate Sanwa dynamic, AIR plateau, NTC response and later AIR54°C rearm, rather than five-degree increments and repeated short budgets. After this evidence, select an actual safe heater-control envelope before unattended operation.

**HOW firmware works (exact current experimental source):**
- AIR target is selected by OLED user menu, 40–55°C; AIR thermostat OFF at target, rearm after drop1°C; fan PWM100% command, fan5s prerun and 90s postheat cooling; duration selected15min–12h (for a first supervised run, a **60min user-setting** provides a useful logging horizon and is NOT an automatic arbitrary heater ON cap).
- Raw NTC research profile: SOFT pause `<=9.81kΩ`, REARM `>=13.73kΩ`, latched HARD backup `<=7.12kΩ`, open/invalid/stale checks. These points are *measured characterization resistances*, **provisional experimental control values**, not calibrated local temperature/safety limits.
- ON window at most20s; independent ESP32 ISR GPIO LOW at20.5s even if loop stalls; OFF at least2s to re-arm window safety; **NO cumulative heater-ON time budget**, no artificial12min/15min session cutoff. Manual STOP, sensor invalid/stale, FAN PWM unavailable/lost command, chamber AIR60°C fault and installed independent series TCO105 unchanged.
- Thermocouple Sanwa RD701/K remains **independently attached at the previously documented PLATE contact near the heater NTC**. Its display is **not connected to ESP32**, so MCU cannot use it as an autonomous fault sensor. Fan command100% is not physical fan RPM verification. `main` stays HEATER LOW and the fully qualified normal `HeaterProductAir` model remains separate/unapproved.

### Provenance of EVERY new and inherited limit

| Limit | Value | Classification and evidence |
| --- | --- | --- |
| AIR user target |40–55°C| USER GOAL / PROVISIONAL_EXPERIMENT UI; physical 55°C not yet validated |
| AIR thermostat hysteresis |1°C| PROVISIONAL_EXPERIMENT, actual 25/24°C cut-off/rearm demonstrated STEP12P |
| AIR independent fault |60°C| PROVISIONAL_EXPERIMENT chosen above maximum requested55°C, no enclosure material-rating claim |
| NTC SOFT/REARM/HARD |9.81/13.73/7.12kΩ| MEASURED_CHARACTERIZATION Sanwa contact paired R at OPEN/fanOFF (74/64/85°C); use as PROVISIONAL_EXPERIMENT thresholds, not qualified closed-box Celsius; electronics separately tested |
| Per-window ON / hardware ISR|20s /20.5s| PROVISIONAL_EXPERIMENT + ESP32 hardware timer implementation from previous GPIO tests, current physical ISR timing NOT independently measured |
| Minimum OFF|2s| PROVISIONAL_EXPERIMENT: controller watchdog restart scheduling, not a claim that 2s thermally cools plate |
| Total heater ON and fixed experiment runtime|**NONE**| user-duration controlled; intentionally eliminates former 300/360s/720/900s experimental bounds |
| Fan prerun / command |5s /100%| PROVISIONAL_EXPERIMENT, previously controlled and fan100 physically observed; physical rotor airflow not sensed |
| Postheat fan|90s| PROVISIONAL_EXPERIMENT; Sanwa must be monitored after OFF to check adequacy |
| NTC/SHT45 stale|1500ms /5000ms| PROVISIONAL_EXPERIMENT; original sample periods500/2000ms and fault-injection host tests |
| NTC open upper|250kΩ| PROVISIONAL_EXPERIMENT from previous sensor fault window |
| Installed TCO|105°C in series HEATER+| ELECTRICAL_OR_HARDWARE existing assembly report, body temperature/DC breaking capability UNKNOWN/UNQUALIFIED; MUST NOT bypass/rerate |
| OEM original PLATE|~118°C reference| MEASURED_CHARACTERIZATION of original unit only per project, not authorization for this altered prototype |
| Independent Sanwa manual observation|actual near-plate °C| MEASURED_CHARACTERIZATION external contact, not a firmware protection; upper observed study point85°C is **NOT a certified safe cap** |

### Physical gate — one ATTENDED session, NOT automatically authorized by a CI PASS

Before applying heater power verify actual status now: existing TCO105 is still properly wired and uncompromised, intact supply/heater connectors and nearby insulation, enclosure airflow path, **fan physically rotates**, Sanwa contact remains firmly at the already documented near-NTC PLATE location, and a way to safely interrupt heater power is immediately accessible. The original OEM heater plate ~118°C does not automatically clear materials/TCO105 in our configuration. If the TCO location/body/DC suitability or material exposure is uncertain, do not begin a higher-temperature heat run until physically evaluated. Do not bypass it.

The initial independent **proposed manual veto** is Sanwa **85°C** at the known contact — the highest actually observed point in the previous open/fanOFF study, NOT a qualified closed-case maximum. This is an *operator experimental stop*, not a firmware sensor or material guarantee. Stop earlier for unexplained rapid temperature rise, loss of actual airflow, thermocouple dislodged, component/shell/insulation heating or smell, a TCO trip, MCU sensor fault, or any unsafe behavior. Immediately stop/interrupt heater power; allow fan cooling while safely powered. Do NOT automatically raise the manual veto based on OEM118°C or software PASS; decide an appropriate independent envelope before hot trial.

For one approved attended characterization: compile/upload the **exact source on this experiment branch in Arduino IDE**, verify `HOT-261030-AIR55` on splash/serial and cold initial Sanwa/AIR/R_NTC/fan, then select desired target up to55°C and a 60-minute user duration (adjust only for actual diagnostic purpose). Confirm actual fan rotation before first ON. Record manual Sanwa contact °C **at least each minute in initial warmup, near local SOFT pauses, first AIR target, after each major OFF and cooling peak**; note timestamp relative to MCU/test start. Compare SHT45 AIR/RH and raw NTC; firmware RAM logger preserves a bounded last 1400 records (~first hour with5s baseline and ON/OFF events, longer runs may lose earliest records) and saves a final `/api/last-test.csv` only after irreversible heater OFF/cooldown. Download CSV at session end; report actual data normally in chat, with no user-made technical report and no commit per individual reading.

**Success/failure decision:** record whether target55 (if selected) is ever reached and held, Sanwa plate contact max/after-OFF overshoot, R_NTC min and any NTC hard fault, genuine AIR54 rearm, external TCO behavior, full STOP/timer/cooldown, and retained CSV completeness. A physical run is NOT successful simply because Arduino IDE/CI compiled; if plate/TCO/material margin is unsatisfactory, next action is a specific guard/hardware change, **not a 5°C step firmware**. Consolidate original CSV, manual observations and conclusions to existing evidence and PROJECT_STATE at end.

**Arduino IDE ONLY:** update local checkout of `experiment/step10-heater-diagnostics-air25-2026-10-28`; open `firmware/firmware.ino`, ESP32 Dev Module (core3.3.12) and U8g2 2.36.19, choose the normal USB serial board port, Upload and verify unique build ID. No ZIP/BIN/browser OTA. **Programming is not itself a physical HEATER ON authorization.**

## Next only after evidence
Update the existing normal `HeaterProductAir` implementation with measured and approved thermal limits, retain all existing UI, fan, network, STOP/timer/cooling and fault logic; verify autonomous long-duration behavior only after an accepted physical envelope. [PROJECT_STATE](PROJECT_STATE.md) · [AGENTS](../AGENTS.md).
