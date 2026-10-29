# Filament Dryer Monitor

Custom ESP32 controller for an eSUN eBOX Lite filament dryer: local OLED/buttons, SHT45 chamber temperature/humidity, heater NTC sensing and separate fan/heater drivers, powered from 24 V.

**The dryer V1 thermostat is not qualified.** `main` runs the SAFE BRING-UP application with GPIO19 forced LOW. The HEATER-capable experimental **STEP11D read-only plate-diagnostics source** (superseding STEP10B source without changing any HEATER limits) remains isolated in [draft PR #18](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). The old STEP10A software PASS does not validate STEP10B or STEP11D. **Latest actually run hardware remains STEP10B; STEP11D has NOT been flashed or physically tested.** **Runtime ID HOT-261029-S10B25 was physically reported in a new completed supervised trial on 29 October, but exact binary hash/CI PASS for that source was not independently verified.** No further HEATER ON test is authorized solely by this success. [One definitive new protocol](docs/STEP10B_AIR_DEMAND_AND_CSV_TEST_2026-10-29.md) describes the revised chamber-demand logic and safe CSV checkpoint. Firmware changes only on the experimental line; main remains heater-off. **STEP11D `HOT-261029-S11D25`/`H29-S11D` adds raw NTC minimum, margins to SOFT45k/HARD40k, and a clearly marked historical open-cover/fan-off Sanwa-contact proxy. These are DIAGNOSTIC ONLY; SOFT/HARD/ON duration/air target/fan command are byte-for-byte identical in behavior to STEP10B. It will NOT make the plate hotter.** **[Physical STEP10B session](docs/evidence/thermal/2026-10-29-STEP10B-PHYSICAL-SESSION.md): AIR25.10°C reached in a12min bounded trial, 21 HEATER ON transitions /213.939s commanded ON, Sanwa plate38–39°C, no recorded fault, 15-column raw CSV byte-verified.** Thermostat rearm at24°C and useful dryer operating temperatures are not yet qualified. **Previous STEP10A report 29 Oct:** operator observed persistent `HEATER OFF` near cycle 19 and powered down during an apparent STEP10A session (installed firmware ID and CSV not yet verified). [Factual evidence](docs/evidence/thermal/2026-10-29-STEP10A-OPERATOR-STOP.md). No rerun authorized or needed to recover data.

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
