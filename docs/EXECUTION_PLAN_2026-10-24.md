# Execution plan to V1

Consolidated 2026-10-29. This file describes the route, not current status or permission to heat. Read [PROJECT_STATE](PROJECT_STATE.md) and [TODO](TODO.md) for the current checkpoint and single next activity. The original dated plan and its chronological updates remain available in the pre-consolidation archive tags.

**Product:** standalone OLED/button operation controlling SHT45 chamber AIR temperature and duration, heater-local NTC protection, fan/cooldown, fail-safe faults/STOP and no restart after power loss. Wi-Fi must not be required. Preserve existing branding, settings and validated peripherals. CSV/web/OTA are developed but not yet integrated into main.12h environmental history and material presets are deferred.

| Stage | Work and completion evidence |
| --- | --- |
| Existing foundations | Preserve completed hardware/UI/sensor/fan/switching observations and software tests in the ledger. Do not repeat bring-up by inertia. |
| Current thermal candidate | Readiness review for STEP10A, then at most one separately accepted supervised session under its original protocol. Archive raw CSV, Sanwa plate readings, status/events and limitations before another trial. |
| Thermal qualification | Establish separate chamber-control and heater-local safety envelopes, NTC uncertainty, physical airflow/material/hotspot behavior and installed TCO interaction. No arbitrary Ω→°C assumptions, timeout increases or automatic escalation to OEM118°C plate. |
| V1 controller | Implement approved air regulation, fan AUTO/cooldown, UI ranges, expiry/STOP/fault latch and reset behavior. Host tests and ESP32 build first; separate supervised physical acceptance. |
| Services and resilience | Selectively integrate reviewed Wi-Fi/logging/OTA changes, preserving local autonomy and safe update/storage behavior. No bulk merge of the HEATER experiment. |
| Release | Representative thermal/drying cycles, fault/power-loss/OTA/storage acceptance, reproducible source/image and records. Deliberate PCB V2 only if justified; D020 and fresh hardware checks apply then. |

Every new test starts WHAT/WHY/PREVIOUS/NEXT, has a concrete saved protocol and exact build, and produces evidence distinct from analysis. Software PASS, physical PASS and unresolved qualification are separate results. D001–D029 are unchanged. TCO procurement is optional (D029); the currently installed105°C series component may not be bypassed automatically.

The former completion plan and older STEP protocols remain historical technical records. They must not become competing work queues.
