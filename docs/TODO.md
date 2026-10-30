# One priority — validate HOT-261031-N8580 as the next ATTENDED HEATER firmware

**Code source:** single existing branch `experiment/step10-heater-diagnostics-air25-2026-10-28`, PR#18 DRAFT. `main` stays HEATER-OFF. User upload path: **Arduino IDE → `firmware/firmware.ino`** after fetching/pulling the experimental branch. Firmware displays full `HOT-261031-N8580` / short OLED `H31-8580`.

**WHAT / WHY / previous result / next gate:** On 30 October `HOT-261030-SANWA` really ran for60min with target AIR55°C; the [original850-row CSV](evidence/thermal/2026-10-30-AIR55-SANWA-raw.csv) and [all37 independent Sanwa readings](evidence/thermal/2026-10-30-AIR55-SANWA-PHYSICAL-SESSION.md) are archived. AIR reached only41.16°C (last41.12°C), external Sanwa plate max69°C; **145 of146 heater ON windows were artificially cut at20s** and every restart required old NTC13.73kΩ despite no soft9.81kΩ reached (minimum actually11.727kΩ). The new run must test whether a **real NTC-local 85/80 thermostat plus AIR55/54**, rather than a repeated energy-pulse controller, allows the chamber to reach target and maintain it. No unchanged-retest is required. This implementation is an **explicit supervised experiment, not production V1 thermal qualification**.

## Saved flowchart — CURRENT implemented HEATER-capable experiment

```mermaid
flowchart TD
    A([User presses START]) --> B[Start cycle timer and fan prerun 5s]
    B --> C{STOP or selected duration over?}
    C -->|Yes| Z[GPIO19 LOW; fan cooldown; complete]
    C -->|No| D{NTC / AIR invalid or stale? Fan command lost? Watchdog expired?}
    D -->|Yes| F[GPIO19 LOW; latched FAULT]
    D -->|No| E{AIR >= 60C or NTC < 7120 ohm?}
    E -->|Yes| F
    E -->|No| G[Update AIR latch: at target OFF; at target-1C ON; between keep previous]
    G --> H{AIR heat-demand latch active?}
    H -->|No| O[GPIO19 LOW; wait AIR target minus1C]
    O --> C
    H -->|Yes| I{Local NTC thermal pause latched?}
    I -->|Yes| J{R_NTC >= 8236.6 ohm?}
    J -->|No| K[GPIO19 LOW; wait for actual NTC cooling]
    K --> C
    J -->|Yes| L[Clear local NTC pause]
    I -->|No| L
    L --> M{R_NTC <= 7120 ohm?}
    M -->|Yes| N[GPIO19 LOW; latch NTC pause]
    N --> C
    M -->|No| Q[GPIO19 HIGH continuously; feed independent20.5s timer after validated checks]
    Q --> C
    W[ESP32 hardware timer ISR] -.->|Firmware loop stops for20.5s| F
    X[Physical TCO 105C in series] -.->|One-shot fuse operates| Y[Heater circuit independently interrupted]
```

**Thermostat latch semantics:** AIR reaching target shuts heat OFF; only AIR <= target−1°C rearms air demand. Local NTC reaching its upper threshold shuts HEATER OFF; only R_NTC >=8.2366kΩ rearms local heat. **Both conditions must allow heating.** A locally hot start (7.12–8.2366kΩ) never bypasses rearm. There is **no 20s software ON window and no minimum OFF time**; hardware watchdog is refreshed on *each validated responsive loop* but physically cuts the GPIO to LOW on stalled code. GPIO may also be forced LOW immediately by STOP, expiry or FAULT. The existing 90s cooling fan behavior is retained.

## Numbers and provenance — do not confuse independent physical sites

| Function | Implemented value | Classification and evidence |
| --- | --- | --- |
| AIR target | User-selectable40..55°C, trial target55°C | USER GOAL; SHT45 actual chamber measurement, not Sanwa |
| AIR rearm | AIR target minus1°C (e.g.55→54) | PROVISIONAL_EXPERIMENT; STEP12P real AIR25/24 thermostat already observed |
| Local NTC stop | R_NTC **<=7,120Ω** | MEASURED_CHARACTERIZATION **Sanwa PLATE85°C paired with embedded NTC7.12kΩ on17 Oct**, OPEN covers/FAN OFF; PROVISIONAL_EXPERIMENT as attended regulator |
| Local NTC rearm | R_NTC **>=8,236.6Ω** | **CALCULATED INTERPOLATION** between measured Sanwa74°C→9.81kΩ and85°C→7.12kΩ, equivalent to Sanwa PLATE80°C in **that old geometry only**. NOT a new physical measurement, not bead calibration |
| Independent latched hot-region fault | R_NTC **<7,120Ω** | PROVISIONAL_EXPERIMENT conservative outside-measured-endpoint guard; a normal post-OFF thermal undershoot can intentionally latch FAULT |
| AIR invalid/hot fault | Fault if invalid/stale or AIR >=60°C | PROVISIONAL_EXPERIMENT, not proven safe material ceiling |
| NTC invalid/stale | Electrical invalid/open, age>1500ms; AIR age>5000ms | ELECTRICAL_AND_PROVISIONAL source safety, independently fail-OFF |
| Fan command | start before heat, 5s prerun, command100%, 90s post-heater cooling | PROVISIONAL_EXPERIMENT; PWM100% does not verify actual rotor rotation |
| User duration | 15min..12h, no cumulative heater ON budget | User cycle timer; physical use of long sessions NOT qualified |
| Repeated20s ON / forced OFF | **REMOVED from currently selected hot controller** | Historical commissioning artefact, demonstrably limited last AIR55 trial |
| Independent hardware fail-OFF | ESP32 ISR timer **20.5s** refreshed by healthy controller loop, not by power-off cycles | ELECTRICAL_OR_HARDWARE implementation; software compile/host invariants required, **physical stalled-loop GPIO verification not yet measured** |
| Installed 105°C TCO | Independent one-shot series fuse, **not bypassed** | ELECTRICAL_OR_HARDWARE user report and assembly; physical body temperature, DC interrupt suitability and assembly material margin still unqualified |
| OEM heater plate roughly118°C | Original OEM historical reference | DIFFERENT assembly/controller; not a certification of modified prototype |

**Crucial historical correction:** Sanwa64°C→13.73kΩ was the *old* too-wide rearm, **not Sanwa74°C**. We have already tested Sanwa85°C→NTC7.12kΩ; do not ask the operator to repeat those seven old points. Current firmware *does not measure Sanwa*: it reads NTC resistance, so **85°C on the actual external Sanwa is not guaranteed** by the proxy, particularly in a changed fan/enclosure geometry. No user was asked to provide a new thermistor catalog part number to proceed with this experimental profile.

## Exact next supervised validation — separate physical gate

**Before energizing**, user/operator must confirm the installed independent TCO105 still physically in series, undamaged wiring/insulation/heat-adjacent plastics, Sanwa firmly at its documented near-NTC PLATE contact, actual fan rotation/airflow, access to power-off/STOP and ability to continuously attend the run. If there is doubt about the real TCO location/body/DC suitability or heater-adjacent materials, resolve that first. This software CI does not establish safe exposure. Do not change/bypass any hardware cutoff.

1. Pull the **experimental branch** into GitHub Desktop; open `firmware/firmware.ino` in **Arduino IDE**; ESP32 core3.3.12, U8g2 2.36.19, the actual board port. Verify `HOT-261031-N8580` at start / `H31-8580` on screen before selecting an AIR goal. Do NOT use old `HOT-261030-SANWA`, main or an Actions ZIP/OTA BIN.
2. Only after independent physical preflight/approval, select the useful AIR target and operator-attended duration (the prior user chose AIR55,60min). Confirm the real fan is rotating throughout, and record external Sanwa at the existing beep prompts (1..10min then 12/14/... until duration). The existing [one-sheet register](https://docs.google.com/spreadsheets/d/1I5utkzGP1zvD168jLghpYB_y_myGjxMb1ccLens4Cks/edit) is the operator input. **Do not treat the beep as a protection**; Sanwa isn't connected to firmware.
3. **Manually STOP/interrupt power at or before an actual independent Sanwa contact reading of85°C**, or earlier for fan loss, smell, unusual housing/component heating, dislodged probe, unexpectedly rapid temperature rise or electrical fault. The empirical NTC cutoff may not correspond to precisely85°C under the running fan; monitor continuously, not only at beeps. A physical TCO105 firing is a protection failure requiring investigation, **not an expected routine thermostat event**.
4. Keep fan cooling when safe, obtain original final `/api/last-test.csv` and all measured post-STOP Sanwa temperatures at+30/+60/+90/+120s if the run reaches the stop stage. An incomplete CSV or poststop values are recorded as missing, never guessed. User may provide numbers and CSV normally in successive chat messages; assistant consolidates them in existing `docs/evidence/thermal` and ledger **once after the completed session**, updates current `PROJECT_STATE` and one TODO, and preserves original bytes/hash.

**Evaluation:** actual AIR55 reached/held or not; independent Sanwa peak including after-OFF; real local NTC upper stop and lower rearm if reached; GPIO timer stability and FAULT, actual fan behavior and timer STOP. No CI PASS, sketch compile, or programming event constitutes an automatic physical heater qualification.

**Next only after first real evidence:** classify the local sensor/probe gradient, independent hard fault and installed TCO/material protection for actual autonomous dryer operation. Maintain one experiment branch and DRAFT PR, do not reopen old pulse-control tests.
