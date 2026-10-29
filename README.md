# Filament Dryer Monitor

Custom ESP32 controller for an eSUN eBOX Lite filament dryer: local OLED/buttons, SHT45 chamber temperature/humidity, heater NTC, fan/heater drivers and 24V input.

**Current line:** `main` SAFE BRING-UP continues to force GPIO19 HEATER LOW. Exactly one experiment line `experiment/step10-heater-diagnostics-air25-2026-10-28` / [PR#18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). Source is opened and uploaded using **Arduino IDE, `firmware/firmware.ino`**, from the correct Git branch. Do not confuse GitHub Actions OTA-app ZIP/firmware BIN with the Arduino IDE sketch procedure.

**NEW STEP13A `HOT-261029-S13A30` / `H29-S13A`: EXPERIMENTAL SOURCE — NOT YET PHYSICALLY RUN.** A bounded hotter-plate trial responding to the now archived STEP12P experiment: target SHT45 chamber **30°C**, rearm after reaching target at **29°C**; raw heater NTC soft pause **33kΩ**, rearm **50kΩ**, backup abort **30kΩ**. Maximum 20s ON window / 20.5s independent hardware ISR, min OFF12s, total heater ON360s, run900s, fan command100%, chamber fault40°C, sensor stale/invalid/STOP/cooldown and installed TCO105 unchanged. **All NTC values remain provisional, not guaranteed plate temperatures or production protection.** STEP13A may heat more than STEP12P, so no new HEATER ON physical trial is automatically approved by compilation.

**Most recent completed physical evidence:** [STEP12P 29 Oct physical session](docs/evidence/thermal/2026-10-29-STEP12P-PHYSICAL-SESSION.md) and [byte-exact 1089-row CSV](docs/evidence/thermal/2026-10-29-STEP12P-raw.csv) (source of links on experimental branch until merged; permanent [source session](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment%2Fstep10-heater-diagnostics-air25-2026-10-28/docs/evidence/thermal/2026-10-29-STEP12P-PHYSICAL-SESSION.md)). Warmup to AIR25°C at **+400.232s**, vs STEP10B **+563.196s** (~29% faster); max AIR25.43°C, 15 commanded ON windows including final 3s, **231.114s total commanded ON**, NTC minimum34.4829kΩ, no faults. AIR thermostat demand genuinely returned after AIR24°C at+684.979s but 12-minute overall cutoff truncated the second new HEATER window. The operator transcribed a two-column manual temperature-like series reaching41/44, but column headers and exact timestamps were not specified, so never silently assign values to Sanwa or heater bead.

**SOFTWARE PASS:** [actual GitHub Actions push-run #40958481940](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/40958481940), source SHA `9c650d5d4ab95ea5affe11cc75737ef23679f341`, passed real SafetyController GPIO mock, native diagnostics, STEP08 replay, air-demand/CSV invariants and **ESP32 core3.3.12/U8g2 2.36.19 build/link**. Program1,181,650B (90%), RAM108,936B (33%); compiled OTA app SHA256 `a0cf45fef9c805f7b6b264c9e9389ea908ced43420ed65050e002e478811ceef`. Subsequent workflow-label and docs-only edits do not change firmware source. **No STEP13A physical test has occurred.**

**NEXT:** independently qualify the more permissive STEP13A physical test conditions and observe plate/SHT45, actual fan and cooling. See [single project state](docs/PROJECT_STATE.md), [TODO](docs/TODO.md) and mandatory experimental-safety/provenance [AGENTS](AGENTS.md). Previously installed 105°C series TCO is neither bypassed nor rerated.


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
