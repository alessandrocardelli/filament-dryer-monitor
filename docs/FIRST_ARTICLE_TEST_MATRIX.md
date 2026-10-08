# First-article staged population matrix — working draft

Status: **first-article assembly and staged bring-up in progress**. Active branch `main`; released hardware source checkpoint `74a3000127371ac6c3b3b7197f9036dec5b58eef`. Derived from the stored 2026-09-17 KiCad netlist, current BOM, `docs/ASSEMBLY.md` and source sheets. This is **not yet a solder-by-reference instruction**: the released PCB has now been read successfully through the GitHub contents endpoint and key pad/net locations below are source-verified, but the exhaustive 94-reference population grouping and bench acceptance limits still require completion before assembly.

## Verified net relationships relevant to the staging

- `/Power/3V3_BUCK`: U5, L1, C10, C20, C21, R36 and **FB1**.
- `3V3_MCU`: FB1, **C11**, TP2, U4 (ESP32), U2 (CP2102-GM), J3, J4 and multiple local bypass/passive components. FB1 is the series connection between the buck output and MCU rail: downstream loads are not automatically isolated when FB1 is fitted.
- `24V_PROT`: Q1, U5, U1, TP1 and the 24 V input protection/load connections; TP3 is on GND.
- `/Power/EN`: U5, Q7, C7, R31 and R32. R5 connects `24V_PROT` to the U1 output/R6 node. **R5 stays DNP until the buck is verified** (D014); R5 alone is not a complete isolation switch for all downstream circuitry.
- U5 support nets include `Net-(D7-K)` (D7, L1, U5, C6), `Net-(U5-BOOT)` (C6, U5), `Net-(U5-FB)` (C8, C9, R34, R35, R36, U5), `Net-(U5-FSW)` (R4, U5), `Net-(U5-ILIM)` (R29, U5) and `Net-(U5-SS)` (C4, U5). Do not power a buck with any required support circuit absent.
- MCU UART: U4/U2/TP6/TP7; I2C SDA: U4/R14/R17; SCL: U4/R15/R16. NTC: U4/R28/C19/J7. Fan control: U4/R25; heater control: U4/R20; heater switched node: Q4/D4/J5.

## PCB points now verified from the released board source

All of the following are on **B.Cu** in the released PCB:

- TP1 at approximately (160.000, 108.023), pad 1 = `24V_PROT`.
- TP2 at approximately (160.000, 103.500), pad 1 = `3V3_MCU`.
- TP3 at approximately (137.160, 107.950), pad 1 = `GND`.
- FB1 at approximately (160.171, 84.862): pad 1 = `/Power/3V3_BUCK`, pad 2 = `3V3_MCU`. Leaving FB1 unpopulated therefore provides a deliberate electrical break between the regulator output and the MCU rail.
- C10 at approximately (163.219, 84.862): pad 1 = `/Power/3V3_BUCK`, pad 2 = GND. C10 pad 1 is a direct physical buck-output measurement candidate while FB1 is absent.
- C21 at approximately (185.180, 83.592): pad 1 = `/Power/3V3_BUCK`, pad 2 = GND; it is another direct buck-output node.
- U5 at approximately (174.649, 85.116); pad 1 is `/Power/3V3_BUCK`, pads 2-4 are `24V_PROT`, pads 16-17 are GND.
- R5 at approximately (188.482, 90.958): pad 1 = `24V_PROT`, pad 2 = AutoEN/U1-output path. It remains absent for the initial bring-up.
- J1 at approximately (186.952, 111.993): pad 1 = GND, pad 3 = raw 24 V input; pad 2 is intentionally unconnected.

These coordinates establish connectivity/location from the KiCad source, not probe ergonomics. Confirm physical access on the received board before attaching clips or probes.

## Planned assembly gates

| Gate | Intended population | Verify / hold point |
|---|---|---|
| 0 — inventory and unpowered inspection | No soldering; reconcile actual parts, C11, bare board, off-board TCO, connectors and modules. | Inspect for shorts and manufacturing defects; document instruments and current limit before power-up. |
| 1 — protected input + complete buck | Input connector/protection and **all necessary** U5, L1, D7, C1/C3, feedback/compensation/soft-start/limit and output components; leave **R5 absent**. **Leave FB1 unpopulated at this gate** to isolate `3V3_MCU`; measure the regulator output on a verified direct node such as C10 pad 1 (or C21 pad 1) relative to GND. Exact complete buck reference list still requires exhaustive grouping. | Measure input at TP1 relative to TP3; measure buck output directly at a PCB-accessible node; verify current-limited startup. No downstream module or external load attached. |
| 2 — 3V3 distribution + AutoEN | Complete necessary MCU-rail decoupling (including **C11**) and the AutoEN circuitry; fit R5 **after** verified buck output. After the isolated buck gate passes, populate FB1 together with the required `3V3_MCU` decoupling/load group; downstream rail loads must be accounted for before energizing. | Check TP2 relative to TP3, confirm 3.3 V before/after R5. Plan separate controlled AutoEN recovery/fault test; no arbitrary fault injection. |
| 3 — MCU + USB/UART | U4, U2 and their complete required local bypass, reset/boot, USB-C/ESD and serial-support parts, based on full schematic population map. | Power-up default outputs OFF; USB enumeration, serial, boot and flash diagnostic firmware. |
| 4 — low-power peripherals | I2C connector/pull-ups and SHT45/OLED, buttons, status LED, buzzer and NTC front end, as complete testable subgroups. | Diagnostic serial tests for each fitted interface. Heater and fan disabled. |
| 5 — power outputs | Fan and heater MOSFET/driver/support parts, then connect off-board loads **separately**. | Verify OFF defaults first; test fan. Heater load remains prohibited until valid NTC conversion/fault gates and independently installed/tested series thermal cutoff are verified. |

## Firmware deliverable before components arrive

Prepare/compile a **separate diagnostic build**, after reading current firmware source and actual MCU sheet. At startup set both power outputs to safe OFF, report boot/serial and sensor/NTC/button status, and expose explicit individual low-power tests. Do not expose an unrestricted heater ON command or claim that firmware alone can substitute for the independent thermal cutoff. The normal application firmware is a later milestone.

## Unresolved before using this document at the bench

1. Confirm probe/clip ergonomics on the **received physical PCB** for the source-verified nodes above; source connectivity and coordinates are now mapped, but physical access cannot be proven before receipt.
2. Generate an exhaustive 94-reference population table against the released BOM and current schematic/netlist, including every required local decoupler and all dependencies; distinguish DNP-at-gate from permanently unpopulated.
3. Establish a bench supply current limit, measured voltage/ripple/current/thermal acceptance ranges and abort criteria from device data and completed load inventory. Do not invent these values.
4. Inspect current firmware tree and compile diagnostic build. None of these tasks is marked completed merely by creating this draft.

See `docs/ASSEMBLY.md` and `docs/TODO.md` for the accepted handoff and open gates.


## Reference-by-reference staged population plan

This grouping covers the **94 BOM-mounted references**. Test points TP1-TP7 and board-only pads are not BOM purchase items and remain available as designed. The purpose is fault isolation during manual first-article assembly, not a redesign.

### Gate 1A — raw input and protection

Populate:

`J1, F1, Q1, D1, D2, R3, C5`

Hold:

- Do not populate FB1 or R5.
- Do not connect external heater/fan/sensor/display loads.
- U5 buck group is added only at Gate 1B.

Rationale from source/netlist: J1/F1/Q1 form the raw-to-protected 24 V path; D1/R3 are the Q1 gate network; D2 is the protected-rail TVS and C5 is the protected-rail bulk capacitor. TP1 and TP3 are bare-PCB test pads, not parts to populate. C1 is `Cin_buck1` and belongs with the buck stage; C2 is `C_autoen_vdd1` and belongs with the AutoEN stage.

### Gate 1B — complete isolated L7987L buck

Add:

`U5, L1, D7, C1, C3, C4, C6, C8, C9, C10, C20, C21, R4, R29, R33, R34, R35, R36`

Also populate the EN default network needed with AutoEN disabled:

`C7, Q7, R30, R31, R32`

**Gate 1B population is exactly the 23 references listed above. Do not populate any Gate 2 parts yet. In particular leave unpopulated:** `FB1, R5, U1, R1, R2, R6, C2`.

Reasoning:

- C1 is the local buck input capacitor (`Cin_buck1`) on `24V_PROT`/GND.
- C3 is the U5 VCC bypass on `24V_PROT`/GND.
- C4 = soft-start, R4 = FSW, R29 = ILIM.
- C6/D7/L1 form the bootstrap/switch/output path.
- C8/C9/C20 and R33-R36 implement compensation/feedback/output sensing.
- C10/C21 are directly on `3V3_BUCK`.
- With R5 absent, R30 holds Q7 base low; the documented D014 behavior keeps AutoEN from suppressing the initial buck start.
- FB1 absent isolates `3V3_BUCK` from `3V3_MCU`.

Initial measurement: `3V3_BUCK` at C10 pad 1 or C21 pad 1 relative to TP3/GND. Do not use TP2 yet: TP2 is downstream of FB1 on `3V3_MCU`.

### Gate 2 — AutoEN, then 3V3_MCU distribution

First add the complete AutoEN sensing/supply-decoupling path:

`U1, R1, R2, R6, C2`

Then, **only after the isolated buck has passed**, add `R5` and repeat the buck-start check. This preserves D014.

After AutoEN behavior is confirmed sufficiently for normal rail bring-up, populate the rail link and bulk capacitor:

`C11, FB1, TP2`

At this point TP2 is the intended `3V3_MCU` measurement node. The device-local decouplers `C12, C13, C14, C16, C17, C18, C22` are populated with Gate 3 together with U2/U4; do not energize U2/U4 without their local decoupling fitted.

### Gate 3 — ESP32 + boot/reset + USB/UART programming

Populate:

`U4, U2, U3, J2, Q2, Q3, R7, R8, R9, R10, R11, R12, R13, C12, C13, C14, C15, C16, C17, C18, C22, SW1, SW2`

Test points already present in the PCB design:

`TP4, TP5, TP6, TP7`

Dependencies confirmed by netlist:

- J2/U3/U2 = USB-C data/ESD/USB-UART chain.
- R10/R11 = USB-C CC resistors.
- R7/R8 = CP2102 VBUS sensing/divider path.
- R9 = CP2102 reset pull-up.
- Q2/Q3 + R12/R13 + C15 + SW1/SW2 implement the ESP32 EN/IO0 auto/manual boot network.
- U2 and U4 share `3V3_MCU`; U2/U4 UART is accessible at TP6/TP7.

Gate condition: flash and run the diagnostic firmware before adding the remaining functional I/O groups. Firmware must set fan/heater control pins to safe OFF immediately.

### Gate 4A — I2C sensor/display

Populate:

`J3, R14, R15, R16, R17`; J4 is the actual 1.54-inch SSD1309 OLED module, not a PCB connector, and should be added as an off-board/module load only after the unloaded I2C bus checks pass

R14/R15 are the 3V3_MCU-side I2C pull-ups; R16/R17 are the series connections into the J3/J4 SDA/SCL wiring according to the stored netlist. Connect off-board modules only after their actual pinout/fit is checked.

### Gate 4B — buttons and status LED

Populate:

`SW3, SW4, SW5, SW6, R19, R22, R23, R24, D3, R18`

The four button nets terminate at U4 and have their corresponding 10 kΩ rail resistors. D3/R18 form the status LED path.

### Gate 4C — buzzer

Populate:

`BZ1, Q6, D6, R27, R37`

This is the complete buzzer driver/flyback/control subgroup from the stored netlist. Test independently from the power outputs.

### Gate 4D — NTC input

Populate:

`J7, R28, C19`

This completes the NTC divider/filter input to U4. Use measured/known resistor substitutes as appropriate for diagnostic ADC checks before heater operation; the real heater NTC calibration and fault limits remain a separate firmware/safety task.

### Gate 5A — fan output

Populate:

`J6, Q5, D5, R25, R26`

Verify safe OFF first with no fan connected; then test the external fan separately.

### Gate 5B — heater output

Populate:

`J5, Q4, D4, R20, R21`

Do **not** energize the heater merely because this group is populated. Heater load testing remains gated by validated NTC conversion/fault handling and the independent off-board thermal cutoff in series with HEATER+.

### Coverage check

The staged list above assigns all 94 production-BOM references exactly once:

- Gate 1A: 7
- Gate 1B: 23
- Gate 2: 8
- Gate 3: 23
- Gate 4A: 5 production-BOM refs + J4 (schematic/PCB display footprint, excluded from production BOM/position files)
- Gate 4B: 10
- Gate 4C: 5
- Gate 4D: 3
- Gate 5A: 5
- Gate 5B: 5

Total production-BOM coverage: **94** references, plus **J4** as the intentional non-production-BOM display footprint.

Before using the list physically, perform one final cross-check against the delivered components and received PCB. Population order inside each gate should favor low-profile passives before large/hot-air parts where practical, but electrical completeness at the gate boundary is mandatory.

## Bare-board preflight result — 2026-10-01

Reported passed before soldering: `JACK_24V_RAW`, `24V_PROT`, `3V3_BUCK` and `3V3_MCU` showed no short to GND; FB1 pad 1-to-pad 2 was open with FB1 absent; J1 pad 3-to-TP1 was open with F1/Q1 absent. Proceed to Gate 1A.

## Gate 1A powered result — 2026-10-02

Gate 1A powered test passed on 2026-10-02: supply current decayed to ~0 after capacitor charging at 5 V, 12 V and 24 V; TP1 tracked the applied input voltage; Q1 gate measured 0 V at 5 V, 0 V at 12 V and 9.31 V at 24 V, consistent with D1 clamping Q1 |VGS| to about 14.7 V.


## Gate 1B isolated buck result — 2026-10-03

Initial no-load L7987L bring-up passed with R5 and FB1 unpopulated. C11 was also still unpopulated on the downstream MCU rail.

Measured results:

- 8 V input: TP1 = 8 V, EN = 1.00 V, 3V3_BUCK = ~3.3 V, bench-supply indication ~4 mA.
- 12 V input: TP1 = 12 V, EN = 1.53 V, 3V3_BUCK = ~3.3 V, measured input current ~1.5 mA with DMM in series.
- 24 V input: TP1 = 24 V, EN = 3.08 V, 3V3_BUCK = ~3.3 V, measured input current ~1.5 mA with DMM in series.

Result: isolated buck startup and no-load regulation passed over the tested 8/12/24 V inputs. Load, ripple, transient and thermal validation remain pending.


## AutoEN normal-operation check — 2026-10-03

After fitting R5, normal 24 V operation was rechecked with FB1 and C11 still unpopulated. Measured EN ≈ 3.1 V and 3V3_BUCK ≈ 3.3 V. Result: AutoEN does not inhibit normal buck startup/regulation under this condition.


## 3V3_MCU rail-link check — 2026-10-03

After fitting C11 and FB1, powered at 24 V. Measured input current ≈ 1.8 mA, 3V3_BUCK ≈ 3.3 V, and TP2 / 3V3_MCU ≈ 3.3 V. Result: downstream 3.3 V rail distribution passed at no load.


## Gate 3 MCU + USB/UART bring-up — 2026-10-04

Gate 3 population was reported complete: U2/U3/U4/J2, Q2/Q3, R7-R13, C12-C18, C22 and SW1/SW2 fitted.

Unpowered checks passed before energizing: TP2-to-GND settled at about 36 kΩ; TP4/EN_ESP-to-GND and TP5/IO0-to-GND both settled at about 39.5 kΩ, with capacitor-related transient readings during settling and no stable near-zero short.

Initial 24 V powered check, current limit 50 mA: input current about 8 mA; TP2/3V3_MCU = 3.3 V; TP4/EN_ESP = 3.3 V; TP5/IO0 = 3.3 V. Thermal-camera inspection showed only mild warming around the buck, CP2102 and D2, with no concerning hotspot reported.

Manual reset/boot checks passed: SW1 pulled EN_ESP from 3.3 V to about 2.5 mV while pressed and it returned to 3.3 V when released; SW2 did the same for IO0.

USB enumeration is **not yet passing**. Windows detects a newly attached USB device but reports **Code 43 — Device Descriptor Request Failed** and exposes only `USB\\DEVICE_DESCRIPTOR_FAILURE`, not a CP2102 VID/PID or COM port. The same result persisted after changing both USB cable and host USB port.

Checks completed while isolating the USB fault:

- USB VBUS at R7/J2 side about 4.7 V; R7/R8 divider node about 3.1 V; both sides of R9 about 3.3 V.
- U3 channel continuity: pin 1↔6 and pin 3↔4 continuous; D+ and D− channels not shorted to one another.
- U3→U2 continuity: U3 pin 6→U2 pin 4 and U3 pin 4→U2 pin 5 continuous; crossed checks open.
- J2 duplicated USB-C data pads to U3 were reported continuous for A6/B6→U3 pin 1 and A7/B7→U3 pin 3, with no D+/D− short.
- U3 orientation/supply check passed: pin 5 about 4.7 V and pin 2 about 0 V.
- U2 direct pin checks reported about 3.3 V on VDD, REGIN, VBUS-sense and /RST pins. U2 visual orientation appears consistent with the PCB pin-1 marking; no obvious perimeter solder bridge is visible in the supplied macro image, but hidden QFN joints/exposed-pad quality are not proven by visual inspection.


- Oscilloscope USB check: D+ was observed rising to the Full-Speed idle high level after connection. At 50 ms/div the host was observed forcing D+ low for a reset interval and then releasing it high again, confirming host-side bus reset activity. A subsequent 2 µs/div single-shot trigger on D− (rising, ~1 V) did **not** trigger, so no D− transition was captured on the CP2102 side during the attempted post-reset enumeration. Further scope isolation is required on the connector side of U3 before attributing the fault to U2.

Current Gate 3 state: MCU power/reset/boot behavior passes; USB descriptor enumeration fault remains open. Do not mark USB/UART or diagnostic-firmware flashing complete until this is resolved.

Further USB isolation (2026-10-04): repeating the 2 µs/div single-shot test with a single 10× probe on **U3 pin 3 (D−, connector side)** also produced no trigger. Therefore no D− rising transition has yet been observed either before or after U3. Next diagnostic step is an unpowered resistance/diode check of D− to GND (and comparison with D+) to look for a hard clamp/short before any U2 rework.

Physical inspection follow-up (2026-10-04): U3 pin 3 (D− connector-side channel) was found to move mechanically when touched with a probe. Treat this as a defective/insufficient solder joint and repair U3 before continuing USB electrical diagnosis. Previous static continuity results on this path are no longer sufficient evidence of a reliable joint.

U3 rework result (2026-10-04): all six U3 pins were re-soldered after pin 3 was found mechanically loose. Windows behavior did not change: the board still enumerates only as an unknown USB device with device-descriptor failure. Therefore the loose U3 joint was a real assembly defect but was not sufficient to explain the remaining USB failure. Repeat high-speed D+/D− observation after this rework before changing U2.

Post-U3-rework scope result (2026-10-04): with a single 10× probe on U3 pin 3 (D−, connector side), D− now produces a rising-edge trigger after USB connection. The captured screen was at 100 µs/div, so the apparent ~1 V envelope/decay is not suitable for judging the actual 12 Mbps USB line amplitude or packet shape. This is a real behavioral change from the pre-rework no-trigger result and confirms that the repaired U3 joint affected the D− path, although Windows enumeration still fails. Next step is a faster timebase capture of D−/D+ packet activity.

Post-U3-rework oscilloscope result (2026-10-04): D− on U3 pin 3 (connector side) now shows clear high-speed USB activity. A 2.5 µs/div capture resolves a burst of transitions, whereas the pre-rework D− trigger produced no event. This confirms that the previously loose U3 pin-3 joint affected the USB path at high frequency. Windows descriptor enumeration was still failing immediately after the rework, so the remaining path/device behavior must still be isolated. Next comparison point: U3 pin 4 (CP2102 side) under the same probe/trigger conditions.

Post-rework propagation check (2026-10-04): U3 pin 4 (D−, CP2102 side) shows a waveform reported as practically identical to U3 pin 3 under the same oscilloscope settings. This indicates the observed D− activity is propagating through U3; U3 is therefore no longer the leading suspect for the remaining descriptor failure.

Gate 3 completeness audit (2026-10-04): rechecked the populated USB/CP2102 support against the current MCU schematic/netlist and Silicon Labs CP2102 regulator-bypassed self-powered configuration. No additional production component is missing from Gate 3: VDD has C12 4.7 µF + C13 100 nF; REGIN is tied to 3V3_MCU with C22 1 µF + C14 100 nF; R9 pulls /RST high; R7/R8 provide VBUS sensing; R10/R11 are the USB-C Rd resistors; J2/U3 provide connector/ESD path. CP2102 integrates the Full-Speed transceiver, pull-up/matching and calibrated oscillator, so no external USB termination resistor or crystal is required. A 100 ns/div D− capture after U3 rework shows full-swing high-speed transitions (~3.76 Vpp displayed), confirming bit-level activity but not by itself proving valid descriptor transactions or CP2102 responses.

CP2102 ground check (2026-10-04): with the board unpowered, U2 pin 3 to TP3/GND measured about 0.3 Ω. The CP2102 peripheral GND connection is therefore electrically present; the hidden exposed-pad solder joint remains visually unverifiable but is no longer the leading explanation for the descriptor failure.

Android host cross-check (2026-10-04): with the PCB powered from 24 V and connected to an Android phone as a prospective second USB host, the USB device-info app reported **no device detected**. This is not yet treated as a definitive second-host failure until USB VBUS at J2/R7 is measured while connected to the phone, because lack of OTG/host negotiation would produce the same result.

Android host VBUS check (2026-10-04): with the phone connected, the board-side USB_VBUS node at the J2/U3/R7 side measured about **3.3 V**, not the ~5 V expected from an active USB host. Therefore the Android "no device detected" result is not a valid independent-host enumeration failure; phone OTG/host mode was not established. Do not use this test as evidence against the PCB until a ~5 V host VBUS is confirmed.

USB VBUS backfeed check (2026-10-04): with USB disconnected and the PCB powered only from 24 V, the J2/U3/R7 USB_VBUS node measured about **1 mV**. The board is therefore not backfeeding USB VBUS from 3V3_MCU. The earlier ~3.3 V seen with the Android phone came from the phone/connection state and did not represent a valid ~5 V host VBUS, so that Android enumeration test remains invalid/inconclusive.

Android second-host confirmation (2026-10-04): while the powered board was connected to the Android phone, USB_VBUS measured 5 V at the J2/R7 node, confirming the phone was acting as an active USB host. The Android USB device-info app still reported no device detected. Together with the unchanged Windows Code 43 / device-descriptor failure, this strongly excludes a Windows-specific problem and shifts the leading fault hypothesis to U2 (CP2102) itself or its QFN soldering/hidden exposed-pad joint.

Clarification on U2 GND/EP (2026-10-04): U2 pin 3 and exposed pad 29 are on the same GND net; the PCB exposed-pad copper is tied into GND. Therefore the measured ~0.3 Ω from pin 3 to TP3 confirms the board-side GND network is intact. The only remaining EP uncertainty would be the hidden solder bond between the package exposed pad and the PCB pad itself, not whether the PCB pad has a GND connection. Given the working peripheral GND, this hidden EP joint is not the leading cause of the USB descriptor failure; prioritize perimeter-pin rework/inspection, especially pins 4–9.

USB enumeration resolved (2026-10-04): after reworking the perimeter pins of U2 (CP2102), Windows now enumerates the device successfully as **Silicon Labs CP210x USB to UART Bridge (COM5)**. The previous Code 43 / device-descriptor failure is resolved. Since reworking U3 alone had not fixed enumeration, while U2 perimeter rework did, the fault was most likely a marginal U2 solder joint. The exact pin was not isolated. Gate 3 USB enumeration now passes; proceed to serial/ESP32 communication validation.

Gate 3 UART boot-log check (2026-10-04): with COM5 open at 115200 baud, pressing SW1 RESET produced a valid ESP32 ROM boot log through U4 TXD0 -> U2 RXD -> USB, including `rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)` and normal flash-load/entry lines. This confirms the ESP32 is running, RESET works, and the ESP32-to-PC UART path is functional. Remaining Gate 3 checks are the reverse UART path and automatic/manual bootloader/programming behavior.

Gate 3 automatic upload attempt (2026-10-04): Arduino IDE 2.3.10 / ESP32 Dev Module / COM5 / esptool v5.3.0 failed during `Connecting...` with `Invalid head of packet (0x65): Possible serial noise or corruption.` The ESP32-to-PC UART path had already passed via readable ROM boot log. This failure does not yet prove a UART RX fault; first isolate the DTR/RTS auto-program circuit by forcing download mode manually with BOOT/RESET and retrying the upload.

Manual download-mode upload follow-up (2026-10-04): forcing BOOT low and resetting manually allows esptool v5.3.0 to connect successfully and identify the ESP32-D0WD-V3 revision 3.1, 40 MHz crystal and MAC. Therefore both UART directions and ROM download mode are functional. Failure now occurs specifically at `Uploading stub flasher...` with `Invalid head of packet (0x65)`. Next checks: confirm the 24 V bench supply is not still limited to 50 mA / entering current limit during stub start, verify 3V3_MCU stability, and if power is clean test an esptool no-stub/alternate-version path before changing hardware.

Gate 3 flash success after current-limit correction (2026-10-04): increasing the 24 V bench-supply current limit from 50 mA to 100 mA allowed the manually-entered ROM downloader session to complete normally. esptool v5.3.0 uploaded and ran the stub flasher, wrote bootloader/partition/app images, verified all hashes, and finished with `Hard resetting via RTS pin...`. The earlier `Invalid head of packet (0x65)` at stub startup was therefore caused by the 50 mA bench current limit / resulting supply droop, not by a bidirectional UART fault or esptool itself. Automatic entry into download mode still requires a separate no-button upload test at the corrected current limit.

Automatic-download retest at 100 mA (2026-10-04): with the bench current limit corrected to 100 mA, a no-button Arduino/esptool upload still fails at `Connecting...` with `Failed to connect to ESP32: No serial data received.` Manual BOOT+RESET at the same current limit programs successfully. Therefore the remaining Gate 3 failure is isolated to automatic download-mode entry (DTR/RTS -> Q2/Q3 -> IO0/EN), not the UART path, flash path, or supply-current limit. Next diagnostic is simultaneous oscilloscope capture of TP4/EN_ESP and TP5/IO0 during an automatic upload attempt.

Auto-program root cause identified (2026-10-04): simultaneous scope capture at TP4/EN_ESP (CH1/yellow) and TP5/IO0 (CH2/purple) during no-button esptool connection shows the two low pulses occurring sequentially rather than with IO0 held low across EN release. Source/netlist review finds the Q2/Q3 cross-coupled auto-program network reversed relative to Espressif's ESP32 reference truth table: current board has U2 /DTR -> Q2 base / Q3 emitter and U2 /RTS -> Q2 emitter / Q3 base, with Q2 collector -> IO0 and Q3 collector -> EN. This yields DTR=1,RTS=0 => IO0 low (instead of EN low), and DTR=0,RTS=1 => EN low (instead of IO0 low). Manual BOOT/RESET remains functional. Do not change released hardware silently; agree a first-article bodge and later schematic/PCB correction before editing design sources.


Auto-program bodge validation (2026-10-04): the first-article board was modified by lifting/cross-connecting the Q2/Q3 collector outputs so that Q2 collector now drives EN and Q3 collector now drives IO0. With the 24 V bench current limit at 100 mA, Arduino IDE/esptool then connected automatically with no BOOT/RESET button presses, identified the ESP32, started the stub flasher, verified all flash images and completed with `Hard resetting via RTS pin...`. This confirms the released PCB's DTR/RTS auto-program truth table was reversed and that swapping the two collector destinations is the correct first-article bodge. Manual BOOT/RESET remains available as a fallback. Runtime diagnostic-sketch serial output is the remaining Gate 3 functional check.

Gate 3 CLOSED (2026-10-04): after the validated Q2/Q3 collector-cross bodge, automatic esptool programming succeeds without button intervention. The flashed diagnostic sketch runs after reset and outputs its expected startup text followed by periodic `alive` messages at 115200 baud. USB enumeration, bidirectional UART, manual RESET/BOOT, ROM downloader, flash write/verify, automatic download entry and automatic reset are therefore all verified on the first article. Proceed to Gate 4A low-power I2C population/testing; keep heater/fan power-output groups unpopulated/disabled until their later gates.

Gate 4A population started (2026-10-04): J3 plus R14/R15 10 kΩ I2C pull-ups and R16/R17 33 Ω series resistors have been installed. J4 OLED and the external SHT45 remain disconnected. Released-netlist mapping at J3 is pin 1 = GND, pin 2 = SDA downstream of R17, pin 3 = SCL downstream of R16, pin 4 = 3V3_MCU. Next hold point is unloaded-bus electrical verification before attaching either I2C module.

Gate 4A unloaded-bus check PASSED (2026-10-04): with J4 OLED and the external SHT45 still disconnected, J3 continuity/short checks were satisfactory and powered idle levels at J3 were correct (3V3_MCU, SDA and SCL all high at approximately the 3.3 V rail). Proceed only after verifying the actual SHT45 board/harness pinout and connector orientation.

Gate 4A harness hold point (2026-10-04): the SHT45 remains disconnected after the unloaded-bus PASS. Its loose leads are red=3V3, yellow=SCL, green=SDA and black=GND. J3 is the fitted JST-PH 4-way board connector; mate the harness as J3-1 black/GND, J3-2 green/SDA, J3-3 yellow/SCL, J3-4 red/3V3. The currently available XH housings are not compatible with J3. A PH 2.00 mm assortment has been identified as a candidate source of the mating housing/pre-crimped leads, but purchase/receipt is not yet confirmed. Do not connect or energize the SHT45 until the finished harness orientation and continuity have been checked.

## Gate 4B first-article result — 2026-10-04

**PASS.**

- SW3, SW4, SW5 and SW6: approximately 3.3 V at rest and approximately 0 V when pressed.
- SW6 / BTN_DOWN initially remained near 0.91 V at rest; inspection/rework found a poor R24 solder joint. After rework the input returned to the expected ~3.3 V idle state.
- D3/R18: GPIO26 blink test passed; LED drive path is functional.
- D3 is not externally visible in the intended enclosure. Per D021 it is reserved for internal diagnostic/service patterns in future firmware.

Gate 4A remains partially open only because the external SHT45 JST-PH harness is not yet available. Gate 4C may proceed independently.

## Gate 4C first-article result — 2026-10-05

**PASS.**

- BZ1, Q6, D6, R27 and R37 populated.
- GPIO33 tone test at ~2.7 kHz produced the expected audible output.
- Variable-frequency three-note test passed, confirming firmware control of pitch and timing.
- Full-duty/near-50% drive was louder than desired; reduced duty cycle was tested successfully for a quieter result. Final firmware may expose discrete user volume levels after acoustic tuning.

Next staged group: Gate 4D NTC input.

## Gate 4D analog-front-end result — 2026-10-05

**PASS for R28/C19/ADC path; J7 and real NTC remain pending.**

- R28 = 47 kΩ and C19 = 100 nF fitted.
- J7 intentionally left unpopulated so the dryer connector can be reused later.
- J7 pad 2 / NTC node with no external resistor: ~3.3 V.
- With 100 kΩ from J7 pad 2 to J7 pad 1/GND: 2.253 V by DMM; ideal divider value ≈2.245 V.
- ESP32 GPIO34 ADC reading: RAW ~2643-2647; `analogReadMilliVolts()` ~2300-2302 mV, stable over repeated samples.
- ADC offset versus DMM (~47 mV, ~2.1%) is acceptable for first-article functional validation; final NTC temperature conversion requires calibration/characterization and fault thresholds.

Do not energize the heater until the real NTC path, calibration, fault handling and independent thermal cutoff requirements are closed.


## Gate 4A SHT45 live result — 2026-10-07

**PASS for the external SHT45/J3 path; J4 OLED remains pending.**

- JST-PH 4-way mating harness assembled to J3-1=GND/black, J3-2=SDA/green, J3-3=SCL/yellow, J3-4=3V3/red.
- With the sensor connected, J3 pin 4 remained at ~3.3 V.
- I2C scan detected the SHT45 at `0x44`.
- Direct SHT4x high-precision measurement command `0xFD` returned valid CRC-checked data.
- Repeated readings were stable at about 22.28-22.30 °C and 65.44-65.58 %RH.
- SHT45 power, harness and I2C communication are therefore functionally validated on the first article.

Next Gate 4A step: connect and test the actual 1.54-inch SSD1309 OLED at J4 on the validated I2C bus.


## Gate 5A fan-driver result — 2026-10-08

**PASS for the driver path without the actual fan load; real-fan validation remains pending.**

- Original Q5 gave about 1 ohm from J6 pin 1 to GND; removing Q5 raised that node to about 68 kohm.
- The removed device was not recognized as a MOSFET by the GM328 and showed a low-resistance path of about 1.69 ohm.
- The replacement device was recognized by the GM328 as an enhancement-mode N-MOSFET before installation.
- After replacement, unpowered J6 pin 1 to GND measured about 130 kohm and rising.
- Powered safe-OFF: Q5 pad 1 / gate = 0 V.
- With 100 kohm temporarily from J6 pin 2 (+24 V) to J6 pin 1, OFF-state J6 pin 1 = 23.96 V.
- Temporary GPIO16/FAN_PWM HIGH test pulled the switched node low as expected.

Next step: verify the actual fan voltage/current/polarity and test static OFF/ON under the real fan load before PWM-speed characterization.


### Real fan load follow-up — 2026-10-08

The actual 24 V fan was connected to J6 and started normally using the static-ON test firmware.

- Bench input current at startup: ~102 mA.
- Bench input current after settling: ~89 mA.
- J6 pin 1 to GND with Q5 fully ON and the fan running: 7.7 mV.
- No current-limit behavior or failed start was reported.

**Gate 5A real-load static ON/OFF path: PASS.** Remaining work is PWM-speed characterization and final firmware behavior.
