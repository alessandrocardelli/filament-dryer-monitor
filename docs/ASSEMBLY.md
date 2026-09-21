# Assembly and bring-up order

Hand assembly with hot air and a fine-tip iron. This document covers only the steps where
assembly order carries engineering meaning. Ordinary reflow/soldering sequence is not
prescribed here.

## Before assembly — current fabrication and purchasing checkpoint

The current prototype fabrication-source revision is `74a3000` (2026-09-17). Its stored ERC reports 0 errors / 0 warnings; DRC reports 0 active errors / 0 unconnected pads, plus one explicitly excluded U4 silkscreen/board-edge warning. The bare-board Gerbers/drills have been uploaded to JLCPCB and its CAM production file was reviewed, but manufacturing/shipping and physical assembly have not been independently confirmed by this repository. **Do not assemble from an earlier AP66200/CP2102N-era production BOM.** The suffixed `hardware/production/Filament_Dryer_Monitor_bom.csv`, current netlist and live `BOM TME` represent the released design.

As of 2026-09-21 the TME component order has been placed but receipt is not confirmed. C11 (Samsung `CL21A226MAYNNNE`, 22 µF / 25 V X5R / 0805, on `3V3_MCU`) is **unresolved**: TME is checking for its unlocated stock unit. Do not claim C11 has been supplied, omit it from the completed build, or substitute an alternate until the actual part and electrical suitability have been confirmed. An alternative 16 V TDK part was only discussed with TME, **not ordered or approved for the build**. First-article bring-up should wait until the required rail decoupling and the safety-related off-board items are accounted for.

Before population, physically inventory the items marked `In casa` in the sheet (J5 plus 13 resistor rows), compare supplier deliveries with the order confirmation, and identify any missing off-board display/sensor/cutoff/cabling parts. One ordered parts set is sufficient for **one assembled PCB**, even if more bare PCBs are fabricated.

The U5 exposed-pad design remains closed for the planned manual assembly method (see D019). The **R5 assembly sequence below is mandatory** for the first power-up.

## U5 exposed pad — hand-assembly implementation

U5 pad 17 is the 3.2 × 3.2 mm GND exposed pad on B.Cu. The board intentionally does **not** use via-in-pad. Instead, six GND vias (0.60 mm diameter / 0.30 mm drill) sit immediately outside the exposed pad in two rows of three, one above and one below U5. They provide the short thermal/GND path into the internal GND planes while avoiding solder wicking through open vias under the pad.

This arrangement was implemented in commit `c9dc4129` and remains present in the current PCB. For the planned prototype assembly, U5 is fitted by lightly pre-tinning the exposed pad, applying flux, placing the device and heating with hot air. Because no stencil is used for this operation, the full B.Paste definition on pad 17 is not a fabrication blocker for the prototype.

If the assembly process later changes to stencil/reflow or external PCBA, reopen the paste-aperture/via-treatment decision for that process. First-board thermal validation of U5 remains required, but it is a validation item, not unfinished PCB layout.

## R5 — do not fit at first assembly

**Rule: keep R5 unpopulated until the buck has independently produced a verified 3.3 V rail.** For the first article, populate and test functional groups progressively as specified below; do not interpret the historical complete-board wording as a requirement to install every other component before the first power-up.

Reason, in short: U1 (TLV1701) has an open-collector output. If U1 is missing, damaged, or
has one unsoldered pin, R5 pulls its output node up, Q7 conducts, and EN is held low. The
buck never starts and the board shows no 3.3 V — which looks exactly like a dead regulator.

U1 is a hand-soldered SOT-23-5. A single cold joint is a realistic first-power-up outcome,
and without this precaution it sends debugging effort to U5, L1 and the feedback network
while the regulator is actually healthy.

With R5 absent, R30 holds the Q7 base at ground, Q7 stays off, EN is free, and the buck runs
normally without AutoEN protection.

Background: `docs/BUCK_L7987L_DESIGN.md` section 11.3, decision D014.

### Bring-up sequence

1. Populate the complete, electrically necessary input-protection and buck group **except R5**; verify against the actual schematic/netlist that all required bypass, feedback, compensation and output components are fitted. Do not energize an incomplete regulator.
2. Power up. Verify the buck produces 3.3 V.
   - If 3.3 V is absent, inspect the populated supply group and any already-connected load or short; do not assume the regulator IC itself is defective.
     AutoEN is not involved and cannot be the cause.
3. Fit R5. Power up again and verify 3.3 V is still present.
   - If the rail disappears at this step, investigate the AutoEN path (including U1, Q7 and solder joints) and verify the buck independently before replacing parts.
4. Only then proceed to fault-injection testing of AutoEN.

Step 3 is the point of the whole procedure: it separates "the buck does not work" from
"AutoEN is shutting down a working buck".

## Off-board items and first-article inventory

The thermal cutoff (~100–110 °C) is **off-board, in series with HEATER+**, and must not be omitted when wiring the assembly into the enclosure. Its physical procurement/installation and open-circuit behavior must be checked before the heater is enabled.

The **94 PCB-mounted references** in the live `BOM TME` do not include the separate **1.54-inch SSD1309 OLED for J4**, **SHT45 humidity/temperature sensor for J3**, the thermal cutoff, mating JST housings/contacts/cables, heater/fan/NTC wiring or enclosure/mounting parts. Verify exact in-hand availability and pinout/fit rather than assuming that a complete PCB BOM covers the assembled dryer.

For the first board, verify the actual mating J5 connector's polarization and fit, J4 OLED form factor, J3 SHT45 harness, and electrical isolation/current paths before energizing the heater. Do not enable normal heater operation until firmware NTC fault handling and independent thermal cutoff protections have been tested.

## First-article handoff — staged assembly and diagnostic firmware (agreed 2026-09-21)

**Resume here when the bare boards and required components arrive.** The first article will be assembled and checked in functional groups rather than populating the entire PCB before the first electrical test. This changes the *assembly/test plan only*, not the released PCB, schematic, BOM or accepted circuit decisions. The R5 rule above remains mandatory.

Before soldering, inspect the released PCB, schematic, netlist, current firmware and actual delivered parts. Produce a reference-by-reference population matrix for each group, identify accessible measurement points, and check whether leaving later groups unpopulated actually isolates their loads (especially the FB1 path from `3V3_BUCK` to `3V3_MCU`). Do not guess which parts can be omitted from a live regulator or apply power to an incomplete feedback/compensation/bypass network. Define bench supply current limits and measured acceptance criteria before energizing the first article; the table below is a planning outline, not a validated step-by-step test instruction.

| Group | Planned population / verification gate |
|---|---|
| 1 — input and buck | Populate the complete input protection and L7987L power stage, including every electrically necessary support component, **without R5**. With external loads disconnected, inspect for shorts and check regulated 3.3 V and supply behavior using a current-limited bench supply. Verify actual isolation of unpopulated downstream groups before power-up. |
| 2 — rail distribution and AutoEN | Populate the remaining required rail-distribution/AutoEN parts; fit R5 **only after** the initial 3.3 V check, then verify 3.3 V again. Fault-injection and recovery tests require a separately reviewed procedure; do not assume an absent 3.3 V uniquely identifies U1. |
| 3 — ESP32, boot and USB-UART | Populate the complete MCU/boot/programming group. Check supply and USB enumeration, then flash and run the pre-prepared diagnostic firmware over serial before adding later peripherals. |
| 4 — low-power interfaces | Populate and check the sensor/display connectors, buttons, LED, buzzer and NTC interface in suitable groups. Exercise each with diagnostic firmware while fan and heater remain disabled. Confirm the actual off-board modules, wiring and connector pinouts before connection. |
| 5 — output stages and external loads | Populate fan/heater driver groups; check their power-up OFF states and outputs without energizing the heater. Test the fan separately. Heater load testing requires validated NTC calibration/fault handling and the independent off-board thermal cutoff installed and checked; never bypass these gates through a diagnostic command. |

**Firmware timing:** prepare and compile a minimal diagnostic firmware *before assembly*, based on the current GPIO map and actual source. It must default fan and heater OFF, expose boot/serial diagnostics and independently test inputs and low-power outputs. Flash it at group 3 and use it at each subsequent gate. Heater diagnostics must inhibit energization on missing, invalid or faulty NTC readings and must not bypass the independent thermal cutoff. The complete application (OLED UI, drying-cycle control, logging and web UI) follows successful first-article hardware/safety validation; it is not a prerequisite for initial bring-up.

**Not yet performed:** the population matrix, physical test-point mapping, diagnostic firmware, numerical acceptance limits and first-board tests. No receipt, assembly, power-up or safety validation is claimed by this handoff. C11 and the off-board safety items remain procurement/inventory gates as documented above.
