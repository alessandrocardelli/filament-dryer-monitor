# First-article staged population matrix — working draft

Status: **pre-assembly planning only; no physical tests performed**. Active branch `pcb/l7987l-layout`; released hardware source checkpoint `74a3000127371ac6c3b3b7197f9036dec5b58eef`. Derived from the stored 2026-09-17 KiCad netlist, current BOM, `docs/ASSEMBLY.md` and source sheets. This is **not yet a solder-by-reference instruction**: the released PCB has now been read successfully through the GitHub contents endpoint and key pad/net locations below are source-verified, but the exhaustive 94-reference population grouping and bench acceptance limits still require completion before assembly.

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

## Planned assembly gates — pending exhaustive reference grouping

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

`J1, F1, Q1, D1, D2, R3, C1, C2, C5, TP1, TP3`

Hold:

- Do not populate FB1 or R5.
- Do not connect external heater/fan/sensor/display loads.
- U5 buck group is added only at Gate 1B.

Rationale from netlist: J1/F1/Q1 form the raw-to-protected 24 V path; D1/R3 are the Q1 gate network; D2 and C1/C2/C5 are on the protected input/GND rail. TP1 measures `24V_PROT`; TP3 is GND.

### Gate 1B — complete isolated L7987L buck

Add:

`U5, L1, D7, C3, C4, C6, C8, C9, C10, C20, C21, R4, R29, R33, R34, R35, R36`

Also populate the EN default network needed with AutoEN disabled:

`C7, Q7, R30, R31, R32`

**Leave unpopulated:** `FB1, R5, U1, R1, R2, R6`.

Reasoning:

- C3 is the U5 VCC bypass on `24V_PROT`/GND.
- C4 = soft-start, R4 = FSW, R29 = ILIM.
- C6/D7/L1 form the bootstrap/switch/output path.
- C8/C9/C20 and R33-R36 implement compensation/feedback/output sensing.
- C10/C21 are directly on `3V3_BUCK`.
- With R5 absent, R30 holds Q7 base low; the documented D014 behavior keeps AutoEN from suppressing the initial buck start.
- FB1 absent isolates `3V3_BUCK` from `3V3_MCU`.

Initial measurement: `3V3_BUCK` at C10 pad 1 or C21 pad 1 relative to TP3/GND. Do not use TP2 yet: TP2 is downstream of FB1 on `3V3_MCU`.

### Gate 2 — AutoEN, then 3V3_MCU distribution

First add the complete AutoEN sensing path:

`U1, R1, R2, R6`

Then, **only after the isolated buck has passed**, add `R5` and repeat the buck-start check. This preserves D014.

After AutoEN behavior is confirmed sufficiently for normal rail bring-up, populate the MCU-rail bulk/local decoupling and rail link:

`C11, C12, C13, C14, C16, C17, C18, C22, FB1, TP2`

At this point TP2 is the intended `3V3_MCU` measurement node. C11 remains required; do not bridge this gate by omitting it because of procurement delay.

### Gate 3 — ESP32 + boot/reset + USB/UART programming

Populate:

`U4, U2, U3, J2, Q2, Q3, R7, R8, R9, R10, R11, R12, R13, C15, SW1, SW2`

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

`J3, J4, R14, R15, R16, R17`

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

- Gate 1A: 10
- Gate 1B: 21
- Gate 2: 13
- Gate 3: 16
- Gate 4A: 6
- Gate 4B: 10
- Gate 4C: 5
- Gate 4D: 3
- Gate 5A: 5
- Gate 5B: 5

Total: **94**.

Before using the list physically, perform one final cross-check against the delivered components and received PCB. Population order inside each gate should favor low-profile passives before large/hot-air parts where practical, but electrical completeness at the gate boundary is mandatory.
