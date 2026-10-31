# One priority — review HOT-261031-N8580 physical result before any thermal envelope change

**New real evidence (31 October 2026):** [complete Sanwa and CSV crosscheck](evidence/thermal/2026-10-31-AIR55-N8580-PHYSICAL-SESSION.md) plus [unedited747-row CSV](evidence/thermal/2026-10-31-AIR55-N8580-raw.csv). Target AIR55°C **FAILED** at peak45.84°C despite continuous HEATER ON commanded for **3594s**, Sanwa max83°C and NTC min7920.4Ω. The stop at60min and90s fan cooldown **did work**; no NTC>= or AIR target thermostat transitions seen, all logged fault counters zero. The original CSV has no flashed firmware ID; image attribution is based on context and match with source. Previous30 Oct AIR55 failed under forced20s cuts; new31 Oct removed that limiter yet still did not reach target. No identical further thermal test is justified.

**Current source:** `HOT-261031-N8580` / OLED `H31-8580`, experiment branch `experiment/step10-heater-diagnostics-air25-2026-10-28` / PR#18 DRAFT. Exact [CI#41142714373 PASS](https://github.com/alessandrocardelli/filament-dryer-monitor/actions/runs/41142714373) at `1cd96dd76500dfb4d161e1b445d8df6c9d8e524b`. Firmware was not changed by archiving this test. `main` stays heater-OFF.

**OEM independent cross-check (found after 31 Oct run):** despite manufacturer claiming level3 hot plate118°C / inside55°C, a [2021 UNI-T two-thermocouple test](https://mysku.club/blog/aliexpress/87837.html) of the original eBOX Lite at level3 observed **36W total consumed, AIR52°C under lid and aluminum plate65°C** after one hour; AIR reportedly plateaued in30min. Separate original eBOX Lite reviews observed **53°C above fan exhaust** ([Burariweb](https://burariweb.info/gadget/3d-printer/esun-ebox-lite-review.html),2021) and **54°C within filament on the spool** ([DNS Club](https://club.dns-shop.ru/amp/review/t-609-aksessuaryi-dlya-3d-pechati/171333-obzor-sushilnoi-kameryi-dlya-filamenta-esun-ebox-lite/),2026). **Different probe sites, ambient not specified, coil/spool load possibly different; no direct matching Sanwa-to-OEM plate readings.** User confirmed only PCB substantially differs from OEM assembly. This changes engineering prioritization: **do not presume new mechanical air leaks, altered heater element or OEM fan100% speed** without observations; investigate supply/load power and AIR sensor location first. OEM hot plate118°C cannot be automatically equated with our Sanwa position.

**WHAT / WHY / prior evidence / next gate:** Review whether the real electric power, heater/fan geometry and chamber heat losses support the target and what the independent TCO and nearby materials can withstand. This builds on the fully observed continuous-ON experiment and replaces repeated short tests with an evidence-driven decision. Only after that review may a separately authorized new experiment be proposed with explicit proven limits.

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


## One next engineering gate

1. **Power and heat path:** reconcile the as-built heater supply and actual operating voltage/current/power (not present in telemetry), the eBOX Lite 35W nominal OEM heater reference, fan's actual airflow and placement, insulation and chamber leaks, closed-box geometry, and today's ambient16–17°C estimate. A near-stationary AIR45–46°C under continuous command does not by itself distinguish insufficient electrical power from losses/airflow; no invented electrical measurements.
2. **Local temperature and independent safety:** use measured 31 Oct Sanwa83°C ↔ NTC roughly7.92–8.08kΩ and 80°C ↔8.35–8.42kΩ; compare with the **different open/fan-off 17 Oct** 85°C ↔7.12kΩ. Do not treat7.12kΩ as a certified85°C shutdown in current geometry. Confirm independent 105°C series TCO actual location, safe operating material ratings, DC cutoff suitability, possible hotspots and margins *before any proposal to raise heater exposure*. Preserve TCO and main heater lockout.
3. **Decision:** produce one justified target-capable thermal/power and local-protection proposal, with provenance table of old/new limits, independent safety verification and software mocks/compile before any further physical run. Physical heater activation remains a distinct operator safety authorization with preflight and Sanwa veto. Do NOT change firmware thresholds or bypass guards merely to chase55°C.
4. **Secondary backlog only:** local web UI should display fresh status while HEATER ON without blocking the safety loop, flag stale readings and keep OTA excluded during heating; test software timing before release. Do not turn this into a new branch/PR.

**Completion criterion:** one technically supported decision on heater power/thermal feasibility and independently qualified protection margins; not a cosmetic doc checkpoint or yet another arbitrary target/NTC increment.
