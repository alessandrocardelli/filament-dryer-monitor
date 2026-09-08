# TODO

Current phase: **L7987L PCB layout/routing** on `pcb/l7987l-layout`.

Read `docs/PROJECT_STATE.md` and `docs/DECISIONS.md` before acting on this list. Inspect the actual current KiCad board before assuming coordinates or placement from older notes.

Latest PCB-only checkpoint recorded for this handoff: **`e8741347ef5570948a1115719ae65fbc0de02ce6`**. Documentation commits may advance branch HEAD without changing the board.

## Completed before this checkpoint

- [x] L7987L + AutoEN schematic integrated in `hardware/Power.kicad_sch`.
- [x] Schematic engineering review signed off 2026-09-04.
- [x] ERC regression gate added to CI with the exact reviewed waiver/warning set.
- [x] TME sourcing / MPN synchronization / footprint audit closed for the current schematic.
- [x] PCB synchronized from the L7987L schematic; legacy AP66200 stage removed.
- [x] Buck and AutoEN components moved to B.Cu and initial placement reviewed.
- [x] Stackup decision closed: **In1 = solid GND, In2 = solid GND**.
- [x] In1 and In2 contain full-board GND zones; there are no internal 3V3/24 V power planes.
- [x] PGND/SGND implementation decision closed: same `GND` net and same continuous inner planes; distinction is local current-return geometry/via placement, not split planes/nets.
- [x] Power/default netclass values reviewed and accepted.
- [x] Current physical placement convention recorded: U5 VIN/VCC side is the **left side** in the board view; C3 closest to VIN/VCC, C1 same side but more external, C21 kept close to pin 1 VBIAS.

## Immediate next action — start here in the next chat

- [ ] **Finalize U5 exposed-pad implementation.** Determine and verify GND/thermal via count, finished/drill diameter, spacing and paste-aperture strategy against ST package guidance and the intended JLCPCB process. The previous chat ended while preparing to place these vias. Do not assume a final via pattern exists unless the current PCB at latest HEAD shows it.
- [ ] After the EP strategy is fixed, place/review the remaining local GND vias. High-current group: C1−, D7 anode, C10−. Quiet group: U5 pin16/EP, C3−, C21− and sensitive control returns. All connect to the same solid inner GND planes.
- [ ] Do **not** create a required B.Cu GND corridor from C1− across U5 to D7/C10−. Short local vias into the common planes are allowed; keep the pulsed return geometry away from the quiet local return region.

## Critical buck routing

- [ ] **Finish critical input loop.** Route/zone C1, C3 and U5 VIN/VCC according to ST guidance, with C3 getting the shortest local bypass connection.
- [ ] **Finish BOOT/LX/catch-diode/inductor geometry.** Keep C6 BOOT-LX short and switch-node copper compact; verify D7/L1 connections against the ST reference layout.
- [ ] **Finish output path.** Complete L1 -> C10 -> `3V3_BUCK` -> FB1 path and local output-ground return.
- [ ] **Finish feedback/compensation routing.** Keep FB and COMP short, quiet and away from LX/high-current copper; verify final R33/C9/C8/R35/C20/R36/R34 routing against U5 pins.
- [ ] **Finish AutoEN routing.** Preserve compact U1/C2/Q7/R1/R2/R5/R6/R30/R31/R32/C7 geometry and keep it out of the switching-current region.

## DFT / debug before routing freeze

- [ ] Decide whether to add convenient test access for `3V3_BUCK`.
- [ ] Decide whether to add COMP test access.
- [ ] Decide whether to add EN / AutoEN-node test access.
- [ ] Ensure convenient local GND probe access near the buck.
- [ ] Avoid a large dedicated LX test pad; if LX must be probed during bring-up, prefer existing D7/L1/U5 switch-node access with a very short probe ground.

## USB

- [ ] Calculate/verify USB differential-pair width and gap for the actual stackup: B.Cu, 0.10 mm FR4 to In2 GND, Er currently entered as 4.5.
- [ ] Put the verified DP width/gap in the **USB** netclass. In the last reviewed Board Setup, USB DP width/gap fields were blank; Default contained 0.20 mm / 0.25 mm but those values are not approved for USB.
- [ ] Review the full D+/D− route for continuous GND reference and avoid reference-plane discontinuities.
- [ ] Review pair spacing, skew, via use and connector/ESD transitions.

## Full-board PCB review after buck routing

- [ ] Verify both internal GND planes remain continuous and no accidental split/isolated area has been introduced.
- [ ] Review GND stitching where useful, especially around return-path transitions; do not add arbitrary periodic stitching without purpose.
- [ ] Review ESP32 antenna keepout on every copper layer and board-edge placement.
- [ ] Review heater and fan high-current paths and connector/current capacity.
- [ ] Review 24 V protection/input-current paths.
- [ ] Review all component-to-edge/courtyard/mechanical clearances.
- [ ] Review U5, U2 and other exposed-pad/stencil details before paste release.

## Verification and production

- [ ] Run a **fresh KiCad DRC** on the current routed board. `hardware/DRC.rpt` dated 2026-08-06 is historical and is not valid for this PCB checkpoint.
- [ ] Resolve/review every new DRC error and warning; regenerate `hardware/DRC.rpt` from the final board.
- [ ] Confirm ERC CI still passes if no schematic changes were made. If schematic changes occur, run a fresh ERC and reopen any affected review gate.
- [ ] Regenerate BOM, CPL/position files, Gerbers/drill files and production netlist from the same final revision.
- [ ] Reconcile generated production data against the final purchasing BOM (`Filament Dryer Monitor — BOM finale Mouser`, tab `BOM TME`).
- [ ] Perform final fabrication/assembly review before ordering.

## First-board bring-up

- [ ] Verify 3.3 V regulation and startup.
- [ ] Measure load-transient behavior.
- [ ] Observe LX switching waveform/ringing and diode stress with appropriate probing.
- [ ] Measure U5, D7, L1 and relevant capacitor temperatures under representative load.
- [ ] Verify fault current/current-limit behavior.
- [ ] Verify AutoEN COMP threshold, shutdown/retry timing and clean restart after fault removal.
- [ ] Validate the remaining NTC/firmware safety calibration work before enabling the heater in normal operation.