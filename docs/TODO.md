# Next work

## One priority: STEP10A readiness review

**Action:** review the already prepared `HOT-261028-S10A25` candidate and [existing STEP10A protocol](STEP10A_HEATER_AND_LOGGING_TEST_2026-10-28.md) with the operator. No new firmware/profile, repeated S9B run or repeated fault recovery is needed.

**Reason:** S9A missed25°C on its120s heat budget; S9B180 also missed25°C and ended in confirmed AirStale. STEP10A combines bounded local-NTC feedback with logging/stall diagnostics; software PASS does not establish physical thermal safety or the cause of the prior loop stalls.

**Prerequisites:** exact source/image identity and current CI PASS; explicit acceptance of the changed STEP10A exposure; operator checks Sanwa on PLATE near NTC, actual rotating fan, installed series TCO105 unchanged, intact materials/cables and accessible24V isolation. See PROJECT_STATE for current refs and limits. No prototype access or physical test is part of repository consolidation.

**Completion criterion:** record a clear readiness outcome (accepted profile and physical preflight, or the specific unresolved condition). Only if all gates pass, the next physical action is ONE supervised STEP10A session under the existing protocol. That session must then have original CSV/hash, status/events, chronological Sanwa readings and an outcome separating25°C target feedback, faults and remaining uncertainty. Do not call readiness or CI a physical PASS.

## Backlog

- Interpret STEP10A data before another thermal change; qualify separate chamber control and heater-local protection, NTC uncertainty, materials/hotspots and actual TCO configuration.
- Split/review general Wi-Fi/OTA/logging improvements from experimental heater integration; resolve archived cold branch CI mismatch before importing runtime code to main.
- Implement/qualify V1 thermostat, FAN AUTO/cooldown, approved UI ranges, STOP/fault/power-loss behavior and local autonomy.
- Storage/OTA/network fault resilience, representative drying cycles and final acceptance. Environmental12h history and material presets remain deferred.
- OLED startup artifact investigation and deliberate future PCB revision if justified (D020; fresh ERC/DRC/exports only then).

Current facts and passed tests belong in [PROJECT_STATE](PROJECT_STATE.md); immutable records belong in the [ledger](THERMAL_EXPERIMENT_LEDGER.md); approved engineering choices remain in [DECISIONS](DECISIONS.md).
