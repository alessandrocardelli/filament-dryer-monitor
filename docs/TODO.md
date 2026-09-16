# TODO

Current phase: **PCB release review / manufacturing preparation** on `pcb/l7987l-layout`.

Hardware checkpoint before this documentation refresh: **`9da46e7953f801073882fd1934802fa8ace1f1c2`**.

Read `docs/PROJECT_STATE.md` and `docs/DECISIONS.md` before acting. The KiCad source files override this list if they disagree.

## Completed

- [x] L7987L + AutoEN schematic integrated and engineering design review completed.
- [x] Legacy AP66200 removed from current schematic/PCB.
- [x] 4-layer stackup fixed with **In1 = solid GND** and **In2 = solid GND**.
- [x] PGND/SGND implementation fixed as current-path regions on the same GND net/planes.
- [x] Buck/AutoEN components placed on B.Cu.
- [x] Power/default netclasses configured.
- [x] PCB routing connectivity completed: current DRC reports **0 unconnected pads**.
- [x] USB data nets renamed to proper `+`/`-` differential-pair names.
- [x] USB class configured for **0.20 mm width / 0.25 mm gap**.
- [x] `USB_90R` tuning profile configured for B.Cu referenced to In2.Cu, target 90 Ω.
- [x] U2 -> U3 USB pair routed on B.Cu with no vias.
- [x] USB-C duplicated D+ pad crossover implemented with two short signal vias and nearby GND stitching; D− remains on B.Cu.
- [x] J2 physical USB mapping corrected: A6/B6 D+, A7/B7 D−; J2 GND/shield pads are GND.
- [x] CP2102-GM footprint updated to 0.95 × 0.28 mm perimeter pads, +0.06 mm mask expansion, 3.25 mm EP and 3×3 0.9 mm paste apertures.
- [x] CP2102 REGIN 1 µF / 25 V local bypass added as current ref C22.
- [x] L7987L VIN/VCC bypass interpretation reconciled: C1 = 10 µF / 100 V already satisfies the VIN ≥1 µF ceramic requirement; C3 = 1 µF / 100 V is the VCC bypass; redundant buck C22 was removed in `24a9170a` (D018 supersedes D012).
- [x] Live BOM TME rechecked: C22 is only the CP2102 1 µF / 25 V part; C3 is the sole 1 µF / 100 V entry.
- [x] Fresh DRC run 2026-09-16: **0 errors, 0 unconnected pads**.
- [x] External power-distribution policy recorded as D017: compact local `3V3_BUCK` copper, `3V3_MCU` primarily 0.50 mm traces after FB1, wide/local `24V_PROT` copper where current requires it, and no internal power planes.

## Release blockers — do these first

- [ ] **Finalize U5 exposed pad.** Current pad 17 is 3.2 × 3.2 mm GND with no thermal vias inside the EP and a full-size paste pad. Define via matrix, drill/diameter/process and stencil/paste windowing against ST + JLCPCB requirements.
- [ ] **Regenerate netlist after latest MCU schematic edit.** The stored netlist still has J2 D+/D− labels reversed relative to the corrected PCB.
- [ ] **Regenerate ERC after latest MCU schematic edit.** Stored report has 1 reviewed GND modeling error and 0 warnings, but predates the latest MCU edit.

## DRC warning closure

Current fresh report has 14 active warnings plus 1 excluded warning.

- [ ] Review/fix footprint-library mismatch warnings for J4, U4, J6, J1, J5 and J3.
- [ ] Review/fix the eight BZ1 silkscreen-over-copper warnings.
- [ ] Reconfirm the existing excluded ESP32 silkscreen/board-edge warning is still intentional at release.
- [ ] Re-run DRC after every release-blocking hardware change.

## Buck final review

Electrical connectivity is complete, but the layout quality gate remains open.

- [ ] Review C1/C3 -> VIN/VCC input-loop geometry against ST guidance.
- [ ] Review C6 BOOT-LX and LX/D7/L1 switch-loop geometry; keep switch-node copper no larger than needed.
- [ ] Review L1 -> C10 -> `3V3_BUCK` -> FB1 path and output-ground return.
- [ ] Review FB/COMP routing and return isolation from switching/high-current current paths.
- [ ] Review AutoEN routing and local returns.
- [ ] Review local GND via/current-return placement for C1−, D7 anode, C10−, U5 pin16/EP, C3− and C21−.
- [ ] Preserve D017 during final cleanup: `3V3_BUCK` remains compact local external copper; `3V3_MCU` may stay trace-distributed at 0.50 mm with only optional local pours; do not create internal power planes.

## USB final review

- [x] 90 Ω target geometry configured: B.Cu / In2.Cu, 0.20 mm width, 0.25 mm gap.
- [x] U2 -> U3 pair kept on B.Cu without vias.
- [x] Type-C A6/B6 and A7/B7 duplicated pads connected correctly.
- [x] Short D+ crossover uses two vias; nearby GND stitching via added.
- [ ] After netlist regeneration, verify schematic ↔ PCB USB mapping one final time.
- [ ] Confirm no later power-zone edit crowds the USB pair enough to invalidate the intended geometry/reference environment.

## Full-board review

- [ ] Verify both internal GND planes remain continuous with no accidental islands/splits.
- [ ] Review ESP32 antenna keepout on every copper layer and at the board edge.
- [ ] Review heater/fan high-current paths, connector current capacity and switched-node copper area.
- [ ] Review 24 V input/protection path and local copper.
- [ ] Review component-to-edge, courtyard and enclosure/mechanical constraints.
- [ ] Review all exposed-pad/stencil details, especially U5 and U2.
- [ ] Decide whether additional buck DFT access is still worth adding before fabrication; existing TP1/TP2/TP3 provide 24V_PROT/3V3_MCU/GND access.

## Production release

- [ ] After all hardware changes, run a final fresh DRC and ERC.
- [ ] Ensure current-branch ERC is actually enforced manually or update the workflow trigger; current workflow push trigger covers `redesign/buck-sourcing`, not `pcb/l7987l-layout`.
- [ ] Regenerate production BOM, CPL/position files, Gerbers, drills and netlist from the same final commit.
- [ ] Reconcile generated production data with `BOM TME`.
- [ ] Perform final fabrication/assembly review before ordering.

## First-board bring-up

- [ ] Follow `docs/ASSEMBLY.md`: assemble without R5, confirm 3.3 V, then fit R5 and confirm 3.3 V again before fault testing.
- [ ] Verify 3.3 V regulation and startup.
- [ ] Measure load-transient behavior.
- [ ] Probe LX waveform/ringing with appropriate short-ground technique.
- [ ] Measure U5, D7, L1 and capacitor temperatures under representative load.
- [ ] Verify current-limit/foldback behavior.
- [ ] Verify AutoEN COMP threshold, shutdown/retry timing and clean restart after fault removal.
- [ ] Establish whether the simulated foldback-lock condition occurs with the real load.
- [ ] Complete NTC/firmware safety calibration before normal heater operation.
