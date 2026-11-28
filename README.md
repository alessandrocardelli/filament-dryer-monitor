# Filament Dryer Monitor

ESP32 controller for eSUN eBOX Lite: SHT45 chamber air/RH, heater-embedded NTC, fan, OLED/buttons, user timer and CSV. The independent Sanwa plate-contact probe was used for historical experiments but **has been removed from the current assembly** (operator confirmation, 26 Nov 2026); future planned tests use SHT45 AIR and the new Vishay heater NTC.

Development continues on [`experiment/step10-heater-diagnostics-air25-2026-10-28`](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/experiment/step10-heater-diagnostics-air25-2026-10-28/docs/PROJECT_STATE.md), [PR #18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). This branch has a real HEATER-capable experimental path; `kHeaterOutputEnabled=false` alone does not disable it. `main` is the non-heating reference. Software CI and operator UI feedback do not qualify autonomous heated operation.

On 26 Nov, the non-heating seven-point J7 resistance verification measured a +0.38% to +3.10% positive mean bias (5/7 within a provisional ±2% goal); 4.69kΩ shows +3.104% and warrants focused low-end ADC/divider validation before heated temperature reliance. Original user readings and uncertainty are preserved in the [thermal experiment ledger](docs/THERMAL_EXPERIMENT_LEDGER.md#26-november-2026--j7-known-resistor-adc-check-physical-non-heating); subsequent UI6 implements a derived correction and passed the limited physical three-point cold check (4.69kΩ +0.461%, 9.96kΩ -0.016%, 99.9kΩ +0.041%). No thermal qualification is claimed.

Use [PROJECT_STATE](docs/PROJECT_STATE.md) for the sole current checkpoint: source verified in software, last physical session, uncertainty about the installed image, open problems and next action. [TODO](docs/TODO.md) contains the priority and the optional passive comparison; it is not a new heating authorization. When an upload is separately agreed, use the correct branch's `firmware/firmware.ino` in Arduino IDE.

The former README's detailed build hashes, chronological results and historical operating text remain in the [pre-correction snapshot](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/95aa47740f1992970135f6278dee8f85a0dfe508/README.md); its NEXT/unflashed/manual-veto text describes earlier checkpoints, not current instructions.

The current experimental branch contains **HOT-261126-UI6**, a J7 -6.8mV readout correction on top of UI5 ([UI6 CI PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/45878786080); focused three-point cold validation PASS; see ledger). UI5 was a HEATER-capable version of the preserved UI3 dashboard/menu/preset firmware. Its actual GPIO19 trial path uses the nominal Vishay NTCS0603E3104FXT R(T) conversion and **four operator-editable thermal thresholds** saved on the OLED in NVS (NTC pause/rearm/hard fault, AIR hard fault). Unset or incoherent settings block START; old OEM/Sanwa raw-ohm limits do not control heating. The **production-only** `kHeaterOutputEnabled=false` is not a global heater lock: `kHeaterStep09TrialMode=true` selects the separate experimental heater path. On 28 Nov operator reported the Vishay installed; HTTP explicitly confirmed UI6, valid NTC17.79°C vs AIR17.02°C, and all four limits UNCONFIGURED. Cold plausibility PASS; thermal limit review and hardware safety remain prerequisites before a heated run.

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
