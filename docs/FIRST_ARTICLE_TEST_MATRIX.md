# First-article staged population matrix — working draft

Status: **pre-assembly planning only; no physical tests performed**. Active branch `pcb/l7987l-layout`; released hardware source checkpoint `74a3000127371ac6c3b3b7197f9036dec5b58eef`. Derived from the stored 2026-09-17 KiCad netlist, current BOM, `docs/ASSEMBLY.md` and source sheets. This is **not yet a solder-by-reference instruction**: physical PCB pad/test-point accessibility and full pin-by-pin isolation remain to be verified against a usable PCB view/pcbnew export. The GitHub file reader returned empty text for the large `.kicad_pcb` in this session; do not infer physical locations from this draft.

## Verified net relationships relevant to the staging

- `/Power/3V3_BUCK`: U5, L1, C10, C20, C21, R36 and **FB1**.
- `3V3_MCU`: FB1, **C11**, TP2, U4 (ESP32), U2 (CP2102-GM), J3, J4 and multiple local bypass/passive components. FB1 is the series connection between the buck output and MCU rail: downstream loads are not automatically isolated when FB1 is fitted.
- `24V_PROT`: Q1, U5, U1, TP1 and the 24 V input protection/load connections; TP3 is on GND.
- `/Power/EN`: U5, Q7, C7, R31 and R32. R5 connects `24V_PROT` to the U1 output/R6 node. **R5 stays DNP until the buck is verified** (D014); R5 alone is not a complete isolation switch for all downstream circuitry.
- U5 support nets include `Net-(D7-K)` (D7, L1, U5, C6), `Net-(U5-BOOT)` (C6, U5), `Net-(U5-FB)` (C8, C9, R34, R35, R36, U5), `Net-(U5-FSW)` (R4, U5), `Net-(U5-ILIM)` (R29, U5) and `Net-(U5-SS)` (C4, U5). Do not power a buck with any required support circuit absent.
- MCU UART: U4/U2/TP6/TP7; I2C SDA: U4/R14/R17; SCL: U4/R15/R16. NTC: U4/R28/C19/J7. Fan control: U4/R25; heater control: U4/R20; heater switched node: Q4/D4/J5.

## Planned assembly gates — pending full reference and PCB review

| Gate | Intended population | Verify / hold point |
|---|---|---|
| 0 — inventory and unpowered inspection | No soldering; reconcile actual parts, C11, bare board, off-board TCO, connectors and modules. | Inspect for shorts and manufacturing defects; document instruments and current limit before power-up. |
| 1 — protected input + complete buck | Input connector/protection and **all necessary** U5, L1, D7, C1/C3, feedback/compensation/soft-start/limit and output components; leave **R5 absent**. Exact reference list and whether FB1 is installed here require pin-level review. | Measure input at TP1 relative to TP3; measure buck output directly at a PCB-accessible node; verify current-limited startup. No downstream module or external load attached. |
| 2 — 3V3 distribution + AutoEN | Complete necessary MCU-rail decoupling (including **C11**) and the AutoEN circuitry; fit R5 **after** verified buck output. Determine FB1 installation point and downstream rail loads before energizing. | Check TP2 relative to TP3, confirm 3.3 V before/after R5. Plan separate controlled AutoEN recovery/fault test; no arbitrary fault injection. |
| 3 — MCU + USB/UART | U4, U2 and their complete required local bypass, reset/boot, USB-C/ESD and serial-support parts, based on full schematic population map. | Power-up default outputs OFF; USB enumeration, serial, boot and flash diagnostic firmware. |
| 4 — low-power peripherals | I2C connector/pull-ups and SHT45/OLED, buttons, status LED, buzzer and NTC front end, as complete testable subgroups. | Diagnostic serial tests for each fitted interface. Heater and fan disabled. |
| 5 — power outputs | Fan and heater MOSFET/driver/support parts, then connect off-board loads **separately**. | Verify OFF defaults first; test fan. Heater load remains prohibited until valid NTC conversion/fault gates and independently installed/tested series thermal cutoff are verified. |

## Firmware deliverable before components arrive

Prepare/compile a **separate diagnostic build**, after reading current firmware source and actual MCU sheet. At startup set both power outputs to safe OFF, report boot/serial and sensor/NTC/button status, and expose explicit individual low-power tests. Do not expose an unrestricted heater ON command or claim that firmware alone can substitute for the independent thermal cutoff. The normal application firmware is a later milestone.

## Unresolved before using this document at the bench

1. Read the **actual released PCB** with KiCad/pcbnew or a trustworthy PCB export; map accessible test points and solder pads, polarity and clearance, especially a direct `3V3_BUCK` measurement and FB1.
2. Generate an exhaustive 94-reference population table against the released BOM and current schematic/netlist, including every required local decoupler and all dependencies; distinguish DNP-at-gate from permanently unpopulated.
3. Establish a bench supply current limit, measured voltage/ripple/current/thermal acceptance ranges and abort criteria from device data and completed load inventory. Do not invent these values.
4. Inspect current firmware tree and compile diagnostic build. None of these tasks is marked completed merely by creating this draft.

See `docs/ASSEMBLY.md` and `docs/TODO.md` for the accepted handoff and open gates.
