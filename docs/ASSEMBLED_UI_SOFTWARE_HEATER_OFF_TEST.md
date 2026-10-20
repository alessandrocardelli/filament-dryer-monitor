# Assembled eBOX Lite: integrated UI/fan bring-up with heater wired but software locked

**2026-10-20 | Status: ongoing; fan startup/ramp passed after solder repair; overall acceptance not yet completed.**
> **Progress checkpoint — 2026-10-20 (not a complete PASS):** User has reported SETUP **5/5**, SETTINGS **5/5** with persistence after power cycle, DRYING entry/technical page accessible. Physical fan startup blocker was diagnosed as poor solder joint at ESP32 U4 **physical pin 27/GPIO16**: initially probing the pin made fan run; after solder repair fan now runs immediate **100% kickstart then 40%**. Fan start/ramp is therefore **passed on user observation**. Still outstanding: supervised 2–3 min run, physical heater-off check with the actual J5 state, SHT45 C/%RH, noise/airflow and cable clearance, ON/OFF long stop/fan OFF, restart into standby/no autoresume. This is NOT evidence of safe heater switching or thermal qualification. The experimental channel0 PWM firmware was tested at some point, but no evidence establishes it was necessary; do not merge it as a proven fix.

**Authoritative source:** `firmware/firmware.ino`, `firmware/SafetyController.h`, `firmware/FanController.h`, `firmware/Ui.h`, `firmware/AppConfig.h`, and active KiCad wiring. This is a **software-only heater-off** check, not a substitute for the earlier hardware-isolated test `docs/CLOSED_ENCLOSURE_NO_HEAT_TEST.md`.

## User-reported assembly and changed test conditions

User reports new front panel, PCB and whole dryer have now been installed, including **heater-power cable/J5 left connected by choice**, instead of the previously documented J5-unplugged arrangement. Nothing here certifies enclosure wire routing, TCO location/rating, polymer temperature rating, gate measurements or heater physical off-state: these must be checked as test observations.

User wishes to resume the **integrated Slewform logo/menu application**, not the temporary 80/90 C heater diagnostic sketch. No new heater control is requested/approved.

## Verified existing firmware behavior

- `SafetyController::begin()` configures GPIO19 output and drives LOW. `SafetyController::update()` calls `forceHeaterOff()` on each application loop; `heaterPermitted()` always returns `false`; no integrated heater enable logic exists.
- MOSFET Q4 is IRLR3636TRPBF and the KiCad netlist connects 100 ohm R20 from GPIO19/HEATER_PWM to Q4 gate with gate pull-down **R21 100 kohm** to GND. R21 helps keep gate off when GPIO is high-impedance; it does not isolate the heating circuit against MOSFET shorts, firmware mismatches, wiring mistakes or all boot transients.
- Existing `FanController`: 25 kHz PWM, 100% 1 s kickstart, then 40% while simulated DRYING cycle active, stopped in STANDBY/FAULT/COMPLETED.
- Heater NTC J7 must remain connected: electrical open/short fault latches `SafetyController` fault and disables fan/cycle. SHT45 (chamber temp/RH) is a separate sensor, and the heater NTC's absolute conversion is still not validated.
- `Ui.h` explicitly renders `DRYING TEST` and `HEATER LOCKED` on active-cycle pages. Setup target temperatures are prototype UI values, not thermal regulation.
- No integrated heater-control code change is necessary to keep GPIO19 LOW. Do not transplant a heating diagnostic sketch into the integrated app.

## Before first powered UI/fan test

1. **Ensure the actual flash image is the normal integrated `firmware/firmware.ino`** and not a previously loaded diagnostic heat sketch, which *can* energize GPIO19 in response to serial `H`. If uncertain, first restore the integrated image under a safe electrical setup; when uploading requires 24 V and the old diagnostic is loaded, temporary heater isolation is the conservative way to prevent accidental heating during firmware replacement. Connecting J5 while a diagnostic remains loaded is not a software lockout.
2. With all power removed, visually inspect connector polarity, PCB fasteners and clearances, cable strain relief, no pinched conductors near lid/base, fan path free, NTC J7 and SHT45 connected, and series TCO still wired, mechanically secured and thermally mounted. Do not bypass TCO.
3. Recognize that the user has chosen *software-only off with J5 connected* instead of the previous physically isolated initial test. This leaves a real uncontrolled-heat failure mode; keep a directly accessible 24 V disconnect and supervise the test. If zero heater energization is required, use the prior J5-isolated checklist instead.
4. Verify before proceeding that MOSFET drive/heater is actually inactive at power-up and throughout fan running, using an appropriate measurement or confirmation of no heater-current/temperature rise. Do not assume a screen saying `HEATER LOCKED` proves the electrical output is off.

## Initial integrated acceptance flow

1. Power under supervision after restoring `firmware/firmware.ino`; open USB Serial Monitor at 115200. Boot should display stationary Slewform emblem, animated roots, separate wordmark/chime, then STANDBY after approximately 4.15 s. Any unintended heater warming or unusually high idle current => remove 24 V.
2. In STANDBY, fan should be OFF; SHT45 readings should appear plausibly as room/chamber air C/RH. OLED may sleep after 60 s of STANDBY; first key press then only wakes display.
3. M short -> SETUP (target C and duration adjustable, fan AUTO placeholder); M long from STANDBY -> SETTINGS (buzzer volume, key-click), M long in SETTINGS saves and returns. Verify all four button travel with fitted panel.
4. ON/OFF short in STANDBY or SETUP -> DRYING TEST. This is **fan-only** despite the displayed target: 100% for 1 s then 40%, with no intentional heater drive. With M short during DRYING toggle technical page showing SHT45/NTC and fan duty; note NTC temperature is *provisional*, not a validated heater bead or protection threshold.
5. Supervised 2–3 min closed assembly: no fan scraping, stuck button, display corruption, repeated reset, rising heater surface temperature, hot wiring or abnormal supply current. Capture initial and final SHT45 C/%RH, fan sound, and whether displayed heater-locked indication agrees with actual off behavior.
6. ON/OFF long ~1.5 s to return STANDBY with fan OFF. Power-cycle, ensure standby/fan off rather than automatic DRYING restart.
7. If `FAULT` appears, log the serial NTC fault; first check J7/wiring with power removed. M short can mute alarm only, not bypass safety.

## Pass/gates

**This only validates integrated UI/PCB/fan/sensing in assembled geometry.** It does not verify heater safety, absolute NTC calibration, independent cutoff suitability or original eBOX Lite nominal 105/116/118 C plate temperatures.

After reporting outcomes: fix reproducible OLED startup artifacts/UI flaws and button mechanics while preserving heater OFF, then finish SHT45 chamber location and independent 105 C TCO choice. A separate reviewed stage may later integrate safe heater control and perform *closed box, fan ON* qualification. No heater enable until that gate.
