# Next work

## Only priority: qualify local heater thermal protection for the existing V1 AIR40–55°C controller

**Status:** `V1-261029-CTRL55` source is implemented on the single active experiment branch `experiment/step10-heater-diagnostics-air25-2026-10-28`. Previous STEP13A is archived in history and disabled in `AppConfig.h`; no new physical HEATER test authorized or performed. Software host+ESP32 source-matched [Actions #40965752078 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40965752078) at `222c4cf4df76aa008b457ad9a80aa99bd5cc1ec1`. **This version MUST keep GPIO19 HEATER OFF.**

**The firmware now actually does (software only):** the integrated `HeaterProductAir.h` reads user-selected **40–55°C chamber AIR target**, uses hysteresis, local heater NTC temperature with independent soft/rearm/hard fault positions, max 20s ON/min OFF windows, firmware STOP, SHT45/NTC invalid/stale faults and an ESP32 watchdog 20.5s in the future qualified product GPIO path. The real user cycle `CycleController` controls **15min–12h** timing. **No 300/360s accumulated HEATER ON lifetime cap.** Tested for 12h synthetic air plate traces including late target rearm, faults, and STOP. Sensor calibration and heater safety values are *deliberately unconfigured*, and both normal HEATER approval flags are false; the existing GPIO cannot be enabled by ordinary Arduino IDE uploading.

### Numerical limit provenance (AGENTS.md categories)

| Variable | Present value / purpose | Classification & basis |
| --- | --- | --- |
| Chamber AIR target 40–55°C | user-selectable *design capability*, not a heater fault value | UNKNOWN / UNQUALIFIED physical; user requirement, UI range only; actual ability to reach55 not demonstrated |
| User duration15min–12h | timer selected in existing menu; no cumulative energy budget | APPROVED_OPERATIONAL for UI behavior per D022 design model; physical thermal hold12h remains UNQUALIFIED |
| NTC soft pause/rearm/hard Celsius | **0/unconfigured in AppConfig**; require independent local/plate thermal review | UNKNOWN / UNQUALIFIED; historical 33k/35k/40k/45k/50k/60kΩ are **PROVISIONAL_EXPERIMENT**, not operational protection |
| Chamber AIR fault Celsius | **0/unconfigured** in normal mode; must exceed target and respect actual materials | UNKNOWN / UNQUALIFIED; STEP13 air40°C was an earlier trial-only guard |
| Per-window max ON20s and hardware ISR20.5s | existing bounded thermal trial design; future product controller also uses ISR | PROVISIONAL_EXPERIMENT / software implementation validated, physical watchdog and heat-transfer approval still pending |
| Minimum OFF12s and fan prerun5s | existing previous trial values, future V1 scheduler configurable with gates initially0 | PROVISIONAL_EXPERIMENT, not a necessary permanent duty cycle; host only synthetic 12s/5s |
| Fan command100% | electrically commanded by existing PWM, actual rotation not sensed | MEASURED_CHARACTERIZATION PWM operating point, airflow/rotor failure qualification UNKNOWN |
| Series TCO105°C | physical installation reported in series HEATER+ | ELECTRICAL_OR_HARDWARE physical assembly report; location/body transient/DC switching/material effectiveness UNQUALIFIED; D029 optional addition does not authorize bypass |
| Original OEM plate reference ~118°C | earlier original unit reference, not this controller's measured temperature | MEASURED_CHARACTERIZATION/reference reported for original assembly, modified enclosure safe Tmax UNKNOWN |
| Open-box Sanwa/NTC 7-point pairs, closed S6C NTC30.32k/Sanwa53°C | original data in existing sources | MEASURED_CHARACTERIZATION **under their particular conditions**, not a transferable NTC-bead calibration |

### One actual next engineering gate (no reflexive test)

Review the existing [NTC/Sanwa open/fan-OFF characterization](NTC_CALIBRATION_ANALYSIS.md), [S6C fan100/closed evidence](STEP06B_FAN100_PHYSICAL_RESULTS_2026-10-25.md), [STEP10B](evidence/thermal/2026-10-29-STEP10B-PHYSICAL-SESSION.md), and [STEP12P original CSV](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/evidence/thermal/2026-10-29-STEP12P-raw.csv) plus manual thermometer observations. Determine actual TCO105 physical location and heating path **from the existing records first**, and decide whether a **single independent hot calibration/temperature-protection characterization** is missing to establish qualified heater NTC soft/rearm/hard and material limits; if yes, formulate only that test with preflight/manual stops and actual reference-probe recording. The user is present during tests and already has Sanwa on the documented heater plate contact. Presence is useful but not a replacement for protection. Do not request known facts or repeat already proven AIR25/24 thermostat test. Do not remove/short the installed TCO or auto-escalate toward OEM118°C.

**Acceptance before activating heat:** independent reviewed local sensor thermal calibration/lag/uncertainty, installed TCO and enclosure/material thermal envelope, SHT45 fault ceiling, fan fault handling, post-heat fan/cooldown, source/test source-matched CI, and a separately reviewed attended useful-air-target trial (up to55°C only if the physical envelope warrants it). The new code can then receive actual documented operating limits **without rewriting the controller or inventing another STEP number**.

**Arduino IDE workflow:** update the existing experimental branch locally and open `firmware/firmware.ino`; core ESP32 3.3.12 / U8g2 2.36.19; choose normal board/port and Upload. Build ID `V1-261029-CTRL55`. **This source is cold/HEATER LOCKED; do not interpret compilation or uploading as an invitation to heat.** No ZIP/BIN/browser OTA needed.

## Later only after thermal gate
Independent physical useful-target hold and cooling; then long-duration autonomous verification of fan, timer, STOP, safe reset, fault latch and persistence. `main` stays safe, PR18 DRAFT unmerged. [Current state](PROJECT_STATE.md).
