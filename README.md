# Filament Dryer Monitor

Custom ESP32 controller for an eSUN eBOX Lite filament dryer, with SHT45 chamber AIR, heater-local NTC, fan, OLED/buttons, timer and logging.

**CURRENT EXPERIMENT SOURCE: `V1-261029-CTRL55`** on the existing branch `experiment/step10-heater-diagnostics-air25-2026-10-28`, [PR#18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). **NEW, NEVER FLASHED OR PHYSICALLY TESTED.** The new integrated `HeaterProductAir.h` regulates the **USER'S selectable chamber target 40–55°C** with 1°C air demand hysteresis configured by qualified parameters, the existing **15min–12h selected cycle duration**, independent local NTC soft/rearm/hard Celsius guards, sensors and fan command fault latch, and per-window max20s ON/min OFF, with ESP32 GPIO watchdog20.5s added to the future qualified product mode. **There is no cumulative HEATER ON budget.**

**Critical distinction:** The actual source now sets `AppConfig::kHeaterStep09TrialMode=false`, `kHeaterOutputEnabled=false`, `kHeaterProfileApproved=false`, `kHeaterNtcModelApproved=false`. Unqualified product heater temperatures and safety parameters remain **zero/unconfigured**, not invented. **On the actual PCB GPIO19 must stay OFF**. This is working **COLD thermostat architecture**, not approval to heat. The old STEP13A 33/50/30k and arbitrary 360s/900s experiment is archived in its original controller source and is **not active**. `main` stays SAFE NONHEATING, unchanged code.

**Why this step:** The original unit has a reported heater-plate reference about **118°C**, but our modified prototype has a reportedly installed series **105°C TCO**, and open-box Sanwa/NTC pairs or the S6C peak Sanwa53°C/NTC30.32kΩ do not calibrate the bead or establish a safe local cut-off for a 55°C AIR setpoint. Choosing 33/31/29kΩ would again be arbitrary. Firmware correctly refuses heater enable until the local thermal model, TCO physical location/body temperature and material/fault envelope are justified. A supervised Sanwa reading helps but is not a hardware safety interlock.

**Latest real completed thermal evidence:** [STEP12P 29 Oct physical run](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/61ef20b2e1b2b8cde145509f2a0c5ff0d306dd5d/docs/evidence/thermal/2026-10-29-STEP12P-PHYSICAL-SESSION.md) and [unaltered 1089-row CSV](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/61ef20b2e1b2b8cde145509f2a0c5ff0d306dd5d/docs/evidence/thermal/2026-10-29-STEP12P-raw.csv) on experimental branch. AIR25°C at +400.232s, peak25.43°C, genuine AIR24°C rearm+684.979s, heater commanded ON231.114s, min NTC34.4829kΩ, no recorded faults. The two-column manual table has no headers; do not invent Sanwa identities. No new heater test has been run since.

**Firmware upload workflow:** **Arduino IDE** → update the *existing experimental branch* locally → open `firmware/firmware.ino` → select already used ESP32 board/port → compile/upload. **No ZIP/BIN/browser OTA required.** This version is **COLD, heater OFF** until independently approved limits are added in a later reviewed source change. [Current state](docs/PROJECT_STATE.md) · [Single next gate](docs/TODO.md) · [mandatory rules](AGENTS.md).

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
