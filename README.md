# Filament Dryer Monitor

Custom ESP32 controller for an eSUN eBOX Lite filament dryer: OLED/buttons, SHT45 chamber temperature/humidity, heater NTC, fan/heater drivers and 24V input.

**Firmware source:** `main` stays SAFE BRING-UP with HEATER GPIO19 forced LOW. Only `experiment/step10-heater-diagnostics-air25-2026-10-28` / [PR#18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18) contains heater-capable experiments.

**NEW STEP12P `HOT-261029-S12P25` / `H29-S12P`: SOURCE CANDIDATE ONLY. NOT flashed, NOT physically validated, **CI PASS** on source commit `f0da9308ad4e3f3f95d8dfcb4b6e2b330576c70b`, [Actions run #40942531127](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40942531127), including GPIO mock, native NTC diagnostics, STEP08 replay and ESP32 build; **not flashed, no physical thermal qualification**.** Responding to user-identified design failure: STEP07's early stop at40kΩ was NEVER a calibrated heater limit. STEP12P replaces STEP10B 45k SOFT /60k resume/40k trip with **provisional test settings 35k SOFT /50k resume/30k backup stop**, selected as a bounded proposal relative to previously observed STEP06C Sanwa53°C with NTC30.32k, NOT as a qualified material/plate Tmax. AIR25/24 demand latch, hardware ISR20.5s, ON<=20s, OFF>=12s, cumulative<=300s, session<=720s, independent sensor/fan/STOP protections, series TCO105 remain unchanged. **No new heater test or installation authorized automatically.**

**OTA app artifact:** [S12P25-HEAT-CAPABLE-OTA-APP.zip](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40942531127/artifacts/12926354606) contains `HOT-261029-S12P25-OTA-app.bin` SHA256 `328480e4d70754f96573db862fed1aae476edc491cffd8131befdaedaf1b4a35`. Firmware is HEATER-capable with revised provisional protection; the artifact being published is not independent evidence of hardware safety. **No hot test authorized merely by CI.**

**Last physical result:** STEP10B runtime `HOT-261029-S10B25` reached AIR25.10°C in720s, 21 pulses /213.939s commanded ON, manual Sanwa plate38–39°C, original 976-row CSV saved. [CSV](docs/evidence/thermal/2026-10-29-STEP10B-raw.csv), [physical run and interpretation](docs/evidence/thermal/2026-10-29-STEP10B-PHYSICAL-SESSION.md). Target was reached after last heater OFF; AIR24 restart unobserved. Heating to40–55°C AIR and normal V1 autonomy not yet qualified.

**ANTI-REGRESSION**: [AGENTS.md](AGENTS.md) now prohibits carrying any early-trial resistance threshold into production or a new test without written measurement/source provenance and a critical review. Original [NTC/Sanwa characterization](docs/NTC_CALIBRATION_ANALYSIS.md) recorded heater sketch90°C indication but Sanwa85°C contact at NTC7.12kΩ, open covers/fan OFF, which does NOT set enclosed continuous Tmax. [S6C](docs/STEP06B_FAN100_PHYSICAL_RESULTS_2026-10-25.md) observed NTC30.32k/Sanwa53°C closed/fan100. Read [PROJECT_STATE](docs/PROJECT_STATE.md) and [TODO](docs/TODO.md) for one actual next gate; historical values are not automatic decisions.


The assembled prototype has functional power, USB/UART (D020 bodge), OLED/buttons, buzzer, sensors, fan and heater switching tests within recorded limits. Local menu, cycle timer, settings persistence and branding are implemented on main. Wi-Fi/OTA, CSV logging and thermal experiments exist on the experimental line; they have not been merged into main.

Read [AGENTS.md](AGENTS.md) before working, then:

| Document | Purpose |
| --- | --- |
| [PROJECT_STATE](docs/PROJECT_STATE.md) | Current code, tested firmware, evidence, limitations |
| [TODO](docs/TODO.md) | One next activity and concise backlog |
| [DECISIONS](docs/DECISIONS.md) | Approved decisions D001–D029 |
| [Thermal experiment ledger](docs/THERMAL_EXPERIMENT_LEDGER.md) | Original protocols, CSV and Sanwa records |
| [Execution plan](docs/EXECUTION_PLAN_2026-10-24.md) | Remaining route to V1 |
| [Consolidation audit](docs/BRANCH_CLEANUP_2026-10-29.md) | Branch/PR disposition and archive refs |
| [Firmware](firmware/README.md) / [UI](docs/UI_SPEC.md) | Software structure and interaction specification |
| [Assembly](docs/ASSEMBLY.md) / [buck design](docs/BUCK_L7987L_DESIGN.md) | Hardware implementation and bring-up records |

Actual `hardware/*.kicad_*` sources define the released board. The prototype's physical D020 collector-cross correction is documented, not retroactively applied to these sources. The installed series TCO105 remains unchanged; D029 treats an additional TCO as optional, not permission to bypass the installed one.
