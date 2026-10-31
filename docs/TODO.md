# One priority — correct discovered reversed fan mounting and verify cold airflow

**New real physical observation (operator, 31 October 2026):** after comparing original eBOX Lite photos, operator reported **the centrifugal fan was mechanically installed backwards** during the failed 31 October AIR55 / `HOT-261031-N8580` run. This observation supersedes power/heat-loss speculation as the FIRST physical issue to resolve, but the **effect on airflow and chamber temperature has not yet been measured**. The CSV's `fan_percent=100` was only commanded duty and could not detect the mounting error. The heater was ON for 3594s continuously, AIR max45.84°C vs target55, external Sanwa PLATE max83°C, NTC min7920.4Ω, timer/cooldown completed without logged faults. [Full physical chronology](evidence/thermal/2026-10-31-AIR55-N8580-PHYSICAL-SESSION.md), [byte-identical raw CSV](evidence/thermal/2026-10-31-AIR55-N8580-raw.csv). User has **not yet confirmed correction or a new airflow test**.

**Current code remains unchanged:** `HOT-261031-N8580` / `H31-8580`, experimental source `1cd96dd76500dfb4d161e1b445d8df6c9d8e524b`, [CI#41142714373](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/41142714373). Existing branch `experiment/step10-heater-diagnostics-air25-2026-10-28`; `main` remains HEATER-OFF. No automatic release of hot physical trials.

**WHAT / WHY / prior result / next gate:** Re-establish the original mechanical orientation of the fan and check airflow **without heater energization**. An inverted blower mounting is now an actual observed assembly discrepancy, whereas the earlier hypotheses of low heater power, changed enclosure loss or altered original airflow were unconfirmed. The cold verification is needed to know whether the 31 October results were obtained with the correct flow path; only then can a safe subsequent thermal comparison be justified. No new long heating test is authorized by this discovery.

## Saved flowchart — HEATER-capable experiment physically tested 31 October; not V1-qualified

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


## One next engineering gate — no heater power

1. **Mechanical correction, power disconnected and cold:** restore blower orientation to match genuine OEM photographs/duct. Check the circular intake, tangential outlet and duct alignment; verify impeller is free and cables do not obstruct it. Examine that the heater/TCO105 series connection and nearby insulating parts were not disturbed. Operator has not reported having finished this yet.
2. **Cold functional verification only:** confirm the airflow direction and that the rotor turns under a **guaranteed heater-disabled** firmware/test condition. The currently installed N8580 firmware is HEATER-capable: starting a normal DRYING cycle may energize the heater and is not an appropriate fan-only test. If no safely isolated cold run is available, do not substitute an unattended hot run; review and prepare a heater-locked-off test separately.
3. **Only after confirmed flow and separate physical permission:** decide on a specifically reviewed supervised thermal comparison with equivalent sensor locations and ambient, real Sanwa supervision, independent TCO/material inspection and explicit STOP at or before the authorized plate-contact veto. The 31 October run used an incorrect fan assembly, so it cannot qualify the intended airflow configuration. Do not loosen NTC thresholds or raise exposure automatically.
4. **Secondary backlog:** nonblocking real-time web dashboard while HEATER ON, retaining control-loop safety and stale-data reporting; independent power/heat-loss investigation if correct airflow still does not reach the desired AIR temperature.

**Completion criterion for this next task:** fan installed in the OEM orientation and correct airflow positively checked with heater disabled, documented; then make one evidence-based decision about whether a supervised comparison is warranted.
