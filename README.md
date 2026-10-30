# Filament Dryer Monitor

ESP32 controller for eSUN eBOX Lite: SHT45 chamber air/RH, heater-embedded NTC, Sanwa independent heater PLATE contact reference, fan, OLED/buttons, user timer and CSV.

**CURRENT firmware: `HOT-261030-SANWA` — HEATER-capable supervised experimental source, NOT physically tested.** Existing branch `experiment/step10-heater-diagnostics-air25-2026-10-28` / [PR #18 DRAFT](https://github.com/alessandrocardelli/filament-dryer-monitor/pull/18). The `main` code remains heater-OFF. The active experiment opens in **Arduino IDE: `firmware/firmware.ino`**, directly from the correct branch. No downloaded Actions ZIP/BIN/browser OTA required.

This firmware uses the actual **user-selected AIR 40–55°C** and **user-selected 15min–12h** cycle: fan commanded100%, AIR thermostat target/rearm1°C below target, max20s heater command per window and 20.5s independent GPIO timer, minimum OFF2s to rearm hardware timer. **No total 300/360s heater ON budget and no artificial 12/15min test deadline.** STOP, stale/invalid NTC/SHT45, AIR fault60°C, raw NTC/open/short fault, fan-command fault, TCO105 unchanged, 90s postheat cooling and logging retained. Fan PWM100 is not proof of real fan rotation (no tachometer).

**Attended RESEARCH NTC envelope (not calibrated plate Celsius or approved production protections):** soft pause `R_NTC <= 9.81 kΩ`, rearm after `R_NTC >= 13.73 kΩ`, latched backup `R_NTC <= 7.12 kΩ`. These are three *actually measured* R values from the [seven-point OPEN-box/fan-OFF Sanwa/contact characterization](docs/NTC_CALIBRATION_ANALYSIS.md): nearby Sanwa64/74/85°C at13.73/9.81/7.12kΩ, respectively. **The same resistance cannot be assumed to imply the same PLATE temperature in a closed box with FAN100; even this larger range may be unsafe for materials/hotspots.** The original ~118°C OEM plate reference is not a verified allowable thermal limit for the modified prototype. Its presently installed 105°C series TCO is neither bypassed nor rerated; TCO body position/DC cutoff capability remains to be physically assessed.

**Sanwa operator measurement reminders (new; software only):** the existing buzzer gives one short, distinguishable beep at **1,2,3,...,10 minutes after START**, then at **12,14,... minutes** until cycle end. On manual STOP or normal user-timer completion, separate beeps occur at **+30, +60, +90, +120 seconds** from the explicit STOP/COMPLETE event (not from a random intervening OFF pulse). During DRYING, press MODE once to view the technical screen with `SANWA NEXT mm:ss` and AIR/NTC/heater status, press MODE again to return. Post-completion the OLED also shows the remaining Sanwa countdown. `SETTINGS > BUZZER` must not be OFF; LOW/MED/HIGH set the reminder volume. Beeps are nonblocking, postpone rather than overwrite any in-progress buzzer signal, and stop completely on FAULT; **the external Sanwa is not connected to ESP32 and has no software threshold trip**. A RAM-only thermal log checkpoint is forced at each beep for time correlation; final CSV persists after cooldown and the fourth +120s prompt, without writing flash during heater ON. No thermal, fan, HEATER GPIO or TCO control parameters were changed by this feature. [Exact session protocol](docs/TODO.md).

**Engineering purpose:** one meaningful, supervised heat-up and AIR-target/hold characterization with the Sanwa already installed on the heater PLATE near the NTC, rather than repeating arbitrary 5°C steps or slight NTC reductions. The user is present to observe; the thermocouple is external and CANNOT trigger an automatic cutoff. **Programming a HEATER-capable sketch does not itself authorize powering the heater**; check the independently reviewed physical preflight and manual abort conditions in [TODO](docs/TODO.md) before any hot run. Software PASS does not mean physical/material safety qualification or permission for unattended operation.

**Last PHYSICAL run: STEP12P** — [original 1,089-row CSV](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/61ef20b2e1b2b8cde145509f2a0c5ff0d306dd5d/docs/evidence/thermal/2026-10-29-STEP12P-raw.csv) and [full chronology](https://github.com/alessandrocardelli/filament-dryer-monitor/blob/61ef20b2e1b2b8cde145509f2a0c5ff0d306dd5d/docs/evidence/thermal/2026-10-29-STEP12P-PHYSICAL-SESSION.md). AIR25°C at+400.232s, actual AIR24 rearm+684.979s, peak25.43°C, commanded total ON231.114s, minR34.4829kΩ, no logged faults. That proof is NOT a physical test of the new AIR55 firmware.

[Current state](docs/PROJECT_STATE.md) · [single next gate / operating protocol](docs/TODO.md) · [safety, traceability and source rules](AGENTS.md).

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
