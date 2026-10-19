# First closed-enclosure integration check — HEATER DISCONNECTED

**Date:** 2026-10-19  
**Status:** READY TO PERFORM; no measurement or pass result yet.  
**Authority:** current `main` source `firmware/firmware.ino`, `firmware/SafetyController.h`, `firmware/FanController.h`, `firmware/AppConfig.h`, `docs/UI_SPEC.md`, `docs/PROJECT_ROADMAP.md`.

## Why this is the next test

The user reports that the revised front panel has been **printed and fits correctly**. This is **user-reported mechanical fit**, not independent CAD verification or proof of final print material heat resistance. The previously open-case, fan-OFF thermal tests are complete through nominal 90 C NTC / 85 C Sanwa; they are not representative of the assembled dryer. We can now validate **closed-box airflow, PCB/UI, low-power sensing, and mechanical routing without ever energizing the heater**. Do not use another heater-power diagnostic for this step.

The current production application always forces heater GPIO19 LOW; the DRYING UI test only runs the fan. The fan's deliberately provisional 25 kHz policy is: 100% for 1 second at start, then 40%; stop, completion or fault => fan OFF. This is **not** final drying/fan AUTO.

## Before powering — no-heater gate

1. Fully remove 24 V input and USB power. **Disconnect the heater-power connector J5**, individually insulate/secure the unplugged heater power harness and ensure it cannot touch the PCB, 24 V conductors, plastic or moving fan. Keep the series 105 C thermal cutoff in place in the heater harness; neither cut it out nor short it.
2. Check fit of the printed panel and actual PCB: mounting holes/fasteners secure without PCB flex, OLED glass not pressed, four buttons return freely, cable/USB clearance, no metal or loose conductor against either side of the PCB. The specific filament/polymer and suitable maximum assembled temperature are **not yet verified**.
3. Check the closed lower panel and upper spool lid do not pinch any of the PCB/SHT45/NTC/fan harnesses; no wire can enter the fan impeller or heater area. Ensure fan inlet/outlet paths are unobstructed.
4. Ensure **NTC/J7 remains properly connected even though heater power/J5 is unplugged**. The existing `SafetyController` latches an NTC open/short fault and inhibits the fan, so an open J7 is **not** a valid fan-test setup. Also connect SHT45/J3 and OLED/J4. If J7 cannot physically reach, stop and route the cable; do not defeat the NTC fault logic.
5. Choose an accessible way to cut the 24 V source at any time, and leave the heater source electrically disconnected through the entire test. No bench heater control sketch should be loaded for this check.

## Execute with current integrated application

1. With J5 isolated, flash or restore `firmware/firmware.ino` using ESP32 Arduino core 3.x, U8g2, 115200 serial monitor. If this firmware is already loaded, do not flash again unnecessarily.
2. Turn on the 24 V supply. Confirm initial boot screen/start sound (if enabled), then **STANDBY**; fan stationary and heater disconnected. Verify OLED visibility and correct panel-window/button mechanics with both lids closed.
3. Confirm SHT45 temperature/RH readings are live and plausible for the chamber. Do **not** interpret them as heater-plate temperature.
4. Start a **DRYING test cycle** with a short ON/OFF press from STANDBY. Observe the fan starts promptly at ~100% for 1 s then visibly/audibly settles to its existing 40% duty; it should not scrape the case, stall or cause the supply to reset.
5. Run for **2–3 minutes**, supervised, with both covers closed. Observe OLED, SHT45, fan sound/vibration and supply stability. Heater remains physically disconnected and software-locked.
6. Stop with an ON/OFF **long press ~1.5 s**, then verify fan stops and UI returns to STANDBY. If the OLED has entered sleep, the first press may only wake it and must not also operate the cycle.
7. Perform one controlled power-off / power-on with J5 still disconnected and confirm restart in STANDBY with fan OFF; no automatic drying restart. Check buttons are not stuck/misaligned after assembly.
8. On any wiring issue, FAULT, fan stall, excessive electrical heating of the PCB, or blocked airflow: cut 24 V and record the observed cause. Do not modify heater safety to get the fan test running.

## What to report and acceptance

| Result | Expected / record |
|---|---|
| Panel fit and access | Lower and upper panels latch fully; no connector pressure; all four buttons and OLED functional |
| Safe boot | STANDBY, fan OFF, heater J5 electrically disconnected |
| SHT45 | Report chamber temperature C and RH % at test start and after 2–3 min; plausible and no sensor fault |
| Fan | One-second startup, sustained 40% PWM, no rubbing or stall in closed case |
| Stop/reset | Long ON/OFF returns to STANDBY; fan OFF after stop and power-cycle; no auto-resume |
| Electrical | Stable 24 V/3V3 behavior, no unexplained reset, FAULT or overheating; note supply current if practical |
| Print material | Note actual filament type (PLA, PETG, ASA, etc.). **Successful room-temperature fit does not qualify hot-service durability** |

**Pass:** move to full wire/sensor placement and safety review.  
**Fail:** resolve the specific mechanical/electrical/firmware issue first. No hot heating test follows a no-heat failure.

## After the no-heat test: hot qualification is separately gated

1. Position SHT45 for true chamber-air sampling away from direct heater radiant heating and draft bias; define reliable heater-mounted Sanwa access without pinched leads.
2. Resolve the independent **105 C single-use series TCO** location/rating relative to eBOX Lite's nominal 105/116/118 C plate reference. No bypass, unwitnessed substitution or automatic higher target.
3. Review actual NTC type/uncertainty: seven reference points map measured resistance to **nearby Sanwa probe in open/fan-OFF conditions**, not intrinsic bead temperature; cold DMM 122.3 kOhm at nominal 20 C remains to be reconciled. Optional heater-OFF cold resistance and co-located body-temperature measurement is useful. `docs/NTC_CALIBRATION_ANALYSIS.md` and the **passive** `firmware/diagnostics/ntc_reference_viewer/ntc_reference_viewer.ino` are already available.
4. Only then prepare separately approved **closed, fan-ON heater testing** with staged chamber-temperature targets, real NTC and SHT45 acquisition, independent cutoff, checked hot plastics/wiring and actual fault shutdown. Do not infer that 118 C nominal plate reference must equal NTC indicated 118 C.
