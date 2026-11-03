# Filament Dryer Monitor

ESP32 controller for eSUN eBOX Lite: SHT45 chamber air/RH, heater-embedded NTC, independent Sanwa plate-contact reference, fan, OLED/buttons, user timer and CSV.

Development continues on [`experiment/step10-heater-diagnostics-air25-2026-10-28`](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/PROJECT_STATE.md), [PR #18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). This branch has a real HEATER-capable experimental path; `kHeaterOutputEnabled=false` alone does not disable it. `main` is the non-heating reference. Software CI and operator UI feedback do not qualify autonomous heated operation.

Use [PROJECT_STATE](docs/PROJECT_STATE.md) for the sole current checkpoint: source verified in software, last physical session, uncertainty about the installed image, open problems and next action. [TODO](docs/TODO.md) contains the priority and the optional passive comparison; it is not a new heating authorization. When an upload is separately agreed, use the correct branch's `firmware/firmware.ino` in Arduino IDE.

The former README's detailed build hashes, chronological results and historical operating text remain in the [pre-correction snapshot](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/95aa47740f1992970135f6278dee8f85a0dfe508/README.md); its NEXT/unflashed/manual-veto text describes earlier checkpoints, not current instructions.

The assembled prototype has functional power, USB/UART (D020 bodge), OLED/buttons, buzzer, sensors, fan and heater switching tests within recorded limits. Local menu, cycle timer, settings persistence and branding are implemented on main. Wi-Fi/OTA, CSV logging and thermal experiments exist on the experimental line; they have not been merged into main.

Read [AGENTS.md](AGENTS.md) before working, then:

| Document | Purpose |
| --- | --- |
| [PROJECT_STATE](docs/PROJECT_STATE.md) | Current code, tested firmware, evidence, limitations |
| [TODO](docs/TODO.md) | One next activity and concise backlog |
| [DECISIONS](docs/DECISIONS.md) | Approved decisions (including later D030–D031) |
| [Thermal experiment ledger](docs/THERMAL_EXPERIMENT_LEDGER.md) | Original protocols, CSV and Sanwa records |
| [Execution plan](docs/EXECUTION_PLAN_2026-10-24.md) | Remaining route to V1 |
| [Consolidation audit](docs/BRANCH_CLEANUP_2026-10-29.md) | Branch/PR disposition and archive refs |
| [Firmware](firmware/README.md) / [UI](docs/UI_SPEC.md) | Software structure and interaction specification |
| [Assembly](docs/ASSEMBLY.md) / [buck design](docs/BUCK_L7987L_DESIGN.md) | Hardware implementation and bring-up records |

Actual `hardware/*.kicad_*` sources define the released board. The prototype's physical D020 collector-cross correction is documented, not retroactively applied to these sources. The installed series TCO105 remains unchanged; D029 treats an additional TCO as optional, not permission to bypass the installed one.
