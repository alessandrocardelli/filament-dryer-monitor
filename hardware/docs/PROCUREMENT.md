# Procurement and TME sourcing

Status: **2026-09-01** — electrical schematic checkpoint on branch `redesign/buck-sourcing`.

## Source-of-truth rules

For the active redesign branch, the current KiCad **schematic and freshly exported netlist** are
the authoritative source for implemented electrical connectivity.

At this checkpoint:

- `hardware/Power.kicad_sch` contains the integrated L7987L buck redesign;
- `hardware/Filament_Dryer_Monitor.net` was regenerated from the current schematic on 2026-09-01;
- the buzzer NPN driver now includes `R37 = 100 kΩ` as a base-emitter pull-down on Q6;
- the pre-FB1 buck output has the explicit net name `/Power/3V3_BUCK`;
- U1/TLV1701 now points to the correct TLV1701 datasheet;
- `hardware/Filament_Dryer_Monitor.kicad_pcb` is **not yet synchronized** and still contains
  the legacy AP66200 power stage and old copper/net names;
- `hardware/production/` outputs are stale for the redesigned power stage and must not be used
  for a manufacturing order until the PCB has been updated and outputs regenerated.

The current electrical review found no blocking schematic omission. The topology can therefore
be treated as **electrically frozen for the sourcing/footprint pass**, subject to a fresh ERC and
the normal PCB/layout review later.

The working purchasing BOM remains the Google Sheet:

- `Filament Dryer Monitor — BOM finale Mouser`
- tab: `BOM TME`

The sheet is still based on the previous procurement state and is **not synchronized with the
new L7987L block**.

Do not reconcile old and new parts by reference designator alone. The L7987L integration reused
some references freed by the removed AP66200 block: for example, `U1` is now TLV1701, `U5` is
L7987L and `L1` is the new 15 µH buck inductor.

---

## Current sourcing decision

The next hardware task is now **TME sourcing and final MPN/footprint selection before PCB
synchronization**.

Reason: several parts in the redesigned block still have provisional MPNs or unverified
footprints, and it is preferable to choose actual purchasable parts before placing/routing the
new power stage.

For each component during the sourcing pass:

1. confirm electrical requirements from the current schematic/design record;
2. search TME for the exact part or a fully compatible alternative;
3. prefer parts with good, repeatable availability;
4. verify package, pinout, body dimensions and footprint compatibility before approval;
5. record the final manufacturer MPN and TME code in the working BOM;
6. update KiCad symbol fields/footprints together in a deliberate cleanup pass.

Using another supplier remains acceptable when:

- an exact mechanical part is required;
- TME does not stock a sensible equivalent;
- a substitution would force unnecessary schematic/PCB redesign.

Stock, price and delivery dates are transient and should not be copied into permanent design
state as fixed facts.

---

## L7987L buck redesign — current electrical state

The AP66200 replacement is implemented in `hardware/Power.kicad_sch` and in the current
exported netlist on `redesign/buck-sourcing`.

Detailed calculations, simulation history and the AutoEN fault-recovery rationale are in
`docs/BUCK_L7987L_DESIGN.md`.

### Current reference/value map

These are current **electrical references and values**, not final procurement approvals:

| Ref | Value / device | Electrical role |
|---|---|---|
| U5 | L7987L | 24 V -> 3.3 V asynchronous buck |
| U1 | TLV1701 | AutoEN comparator |
| Q7 | MMBT3904 | Pulls L7987L EN low during AutoEN fault cycle |
| L1 | 15 µH | Buck inductor |
| D7 | STPS2L60A | Catch Schottky diode |
| C1 | 10 µF | Local `24V_PROT` input ceramic |
| C2 | 100 nF | TLV1701 supply bypass |
| C3 | 1 µF | L7987L VCC bypass |
| C4 | 33 nF | Soft-start capacitor |
| C6 | 100 nF | Bootstrap capacitor |
| C7 | 330 nF | EN retry/timing capacitor |
| C8 | 39 pF | Type-III compensation |
| C9 | 18 nF | Type-III compensation |
| C10 | 47 µF | Main buck output capacitor |
| C20 | 560 pF | Type-III compensation |
| C21 | 1 µF | VBIAS/output bypass |
| R1 | 270 kΩ | AutoEN fault-threshold divider, top |
| R2 | 22 kΩ | AutoEN fault-threshold divider, bottom |
| R4 | 47 kΩ | Switching-frequency programming |
| R5 | 100 kΩ | TLV1701 open-collector output pull-up |
| R6 | 100 kΩ | TLV1701 output to Q7 base |
| R29 | 47.5 kΩ | Current-limit programming |
| R30 | 47 kΩ | Q7 base-emitter pull-down |
| R31 | 100 kΩ | EN pull-up |
| R32 | 15 kΩ | EN pull-down |
| R33 | 16 kΩ | Type-III compensation |
| R34 | 15.2 kΩ | Feedback divider, lower |
| R35 | 1.13 kΩ | Type-III compensation |
| R36 | 47.5 kΩ | Feedback divider, upper |

The pre-existing `C5 = 100 µF / 50 V` bulk capacitor remains upstream on `24V_PROT`, and the
existing `FB1` / `C11` output filtering toward `3V3_MCU` remains part of the project.

### Important implemented details

- L7987L `VIN1`, `VIN2` and `VCC` are tied directly to `24V_PROT`.
- The earlier VIN-to-VCC 0 Ω jumper has been removed.
- VCC retains its local 1 µF bypass (`C3`).
- `PGOOD` and `SYNCH` are intentionally left NC.
- AutoEN is implemented with TLV1701 + Q7 and the complete threshold/pull-up/pull-down/EN-RC
  network.
- The temporary `Buck redesign` hierarchical sheet has been removed from the active hierarchy;
  the implementation lives directly in `Power.kicad_sch`.
- Q6 buzzer driver now has `R37 = 100 kΩ` base-emitter pull-down so the transistor remains off
  while the ESP32 output is high-impedance during reset/boot.

---

## Metadata and footprint cleanup for the sourcing pass

The next TME/MPN pass should deliberately resolve symbol fields and footprints rather than
trusting provisional data already present in KiCad.

Known items include:

- `R37` is electrically correct but its copied `Function` metadata should be renamed from the
  fan pull-down label to a buzzer-specific name such as `R_pd_buz1` in Symbol Fields Editor;
- C10's current MPN text has a trailing comma;
- D7's manufacturer field has a leading space;
- several new buck passives and the catch diode/inductor still need deliberate final footprint
  assignment/review;
- existing MPN fields for C1/C3/C10/C21/L1/Q7/D7 are provisional until explicitly approved in
  the sourcing pass.

These metadata corrections can be performed together in KiCad Symbol Fields Editor once the
TME choices are known; they do not justify separate electrical commits.

---

## Existing non-buck procurement items

The previous TME conversion identified several non-buck substitutions that still require
mechanical/electrical review during the final sourcing pass. They are retained as pending
procurement context, not as automatically approved KiCad changes:

| Current project item | Previous TME-oriented candidate / issue | Required action before applying |
|---|---|---|
| C5, 100 µF / 50 V bulk | Panasonic `EEEFK1H101P` candidate | Verify land pattern/body clearance and final sourcing choice |
| J2 USB-C | GCT `USB4216-03-A` candidate | Verify footprint, pin mapping and 3D/mechanical fit |
| Q1 reverse-polarity P-MOS | onsemi `NTD20P06LT4G` candidate | Verify package/pinout and electrical suitability |
| Q5 fan MOSFET | Infineon `IRLML2060TRPBF` candidate | Verify SOT-23 pinout and final part choice |
| U2 USB-UART | Silicon Labs `CP2102-GM` candidate | Re-check exact package/pin compatibility before any substitution |
| U3 USB ESD protector | Akyga Semi `AKS1201` candidate | Verify pinout, ESD characteristics, capacitance and footprint |
| J7 JST GH NTC connector | Exact `BM02B-GHS-TBT` preferred | Preserve top-entry mechanics; use another supplier rather than change geometry without reason |

The old procurement entry for the AP66200-era `L1 = 8.2 µH` is superseded. Current `L1` is the
new 15 µH L7987L inductor and must be sourced as part of the new buck pass.

---

## PCB, ERC and production state

The schematic/netlist redesign is ahead of the board.

The current PCB still contains:

- the AP66200 footprint;
- legacy `/Power/VCC_AP66200` routing/net data;
- the previous power-stage placement/routing.

The existing `hardware/ERC.rpt` predates the L7987L integration and is not evidence that the
current schematic is ERC-clean. A fresh ERC must be generated from the current schematic.

After the TME sourcing/MPN/footprint pass, proceed in this order:

1. update KiCad Symbol Fields Editor with approved MPN/manufacturer/TME metadata and cleanup;
2. assign/verify the approved footprints;
3. run a fresh ERC on the synchronized schematic revision;
4. update PCB from schematic, removing the legacy AP66200 stage;
5. place and route the L7987L power loop according to ST layout guidance;
6. perform PCB-specific review of switching loops, grounding, thermal paths, USB routing,
   ESP32 antenna keepout and heater/fan high-current paths;
7. run DRC;
8. regenerate BOM, position files, fabrication outputs and production netlist;
9. reconcile generated production data against the final `BOM TME` sheet before ordering.

Until those steps are complete, the old PCB and production exports are historical only and are
not manufacturing-ready.
