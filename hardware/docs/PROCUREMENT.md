# Procurement and TME sourcing

Status: **2026-09-01** — electrical checkpoint on branch `redesign/buck-sourcing`.

## Source-of-truth rules

For the active redesign branch, the current KiCad **schematic and freshly exported netlist** are
the authoritative source for implemented electrical connectivity.

At this checkpoint:

- `hardware/Power.kicad_sch` contains the integrated L7987L buck redesign;
- `hardware/Filament_Dryer_Monitor.net` was regenerated from that schematic on 2026-09-01;
- `hardware/Filament_Dryer_Monitor.kicad_pcb` is **not yet synchronized** and still contains
  the legacy AP66200 power stage and old copper/net names;
- `hardware/production/` outputs are therefore stale for the redesigned power stage and must
  not be used for a manufacturing order until the PCB has been updated and outputs regenerated.

The working purchasing BOM remains the Google Sheet:

- `Filament Dryer Monitor — BOM finale Mouser`
- tab: `BOM TME`

The Google Sheet remains the working source for supplier codes, stock, prices and purchasing
choices, but it is **not currently synchronized with the new L7987L block**.

### Current procurement hold

The present decision is to **defer final MPN selection/cleanup and defer the Google Sheet
update** until the electrical schematic has been frozen and the PCB update is ready.

Consequences:

- existing MPN/manufacturer fields on newly inserted buck components are provisional;
- blank MPN/footprint fields in the new block are expected at this stage;
- do not infer a final ordering choice from an MPN already present in KiCad;
- do not update `BOM TME` yet;
- do not use old buck reference designators in the sheet as if they still identified the same
  components.

The last point matters because the newly integrated buck block was selectively re-annotated.
Several references freed by the deleted AP66200 circuit are now reused by different L7987L
components. For example, `U1` is now the TLV1701 comparator, `U5` is the L7987L, and `L1` is
now the 15 µH L7987L inductor. Reference-only matching against an older BOM is therefore not
safe until the deliberate reconciliation step.

---

## Sourcing strategy

TME remains the preferred supplier for the prototype BOM. The goal is to use parts with good,
repeatable availability rather than preserving a historical JLCPCB/LCSC or Mouser part when a
fully compatible and better-stocked alternative is available.

Using another supplier remains acceptable when:

- an exact mechanical part is required;
- TME does not stock a sensible equivalent;
- a substitution would force unnecessary schematic/PCB redesign.

Stock, price and delivery dates are transient and should not be copied into design-state
documentation as permanent facts.

---

## L7987L buck redesign — current electrical state

The AP66200 replacement is no longer merely a candidate: it is **implemented in the current
Power schematic and exported netlist** on `redesign/buck-sourcing`.

Detailed calculations, simulation history and the fault-recovery rationale are in
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

### Important implementation changes from the earlier redesign draft

- L7987L `VIN1`, `VIN2` and `VCC` are now tied **directly** to `24V_PROT`.
- The earlier VIN-to-VCC 0 Ω jumper has been removed.
- VCC retains its local 1 µF bypass (`C3`).
- `PGOOD` and `SYNCH` are intentionally left NC.
- AutoEN is implemented with TLV1701 + Q7 and the complete threshold/pull-up/pull-down/EN-RC
  network.
- The temporary `Buck redesign` hierarchical sheet has been removed from the active hierarchy;
  the implementation lives directly in `Power.kicad_sch`.

---

## Known metadata/footprint work intentionally deferred

Do not resolve these as part of the present documentation checkpoint; they belong to the
forthcoming MPN/footprint pass.

Current known items include:

- U1/TLV1701 still carries a stale LM397 datasheet URL in its KiCad metadata;
- C10's current MPN text has a trailing comma;
- D7's manufacturer field has a leading space;
- several new buck passives and the catch diode/inductor still need deliberate final footprint
  assignment/review;
- existing MPN fields for C1/C3/C10/C21/L1/Q7/D7 are not to be treated as final purchasing
  approval merely because they are populated.

The Google Sheet must remain untouched until this pass is complete.

---

## Existing non-buck procurement items

The previous TME conversion identified several non-buck substitutions that may still require
mechanical/electrical review when the final sourcing pass resumes. These are retained here as
pending procurement context, not as automatically approved KiCad changes:

| Current project item | Previous TME-oriented candidate / issue | Required action before applying |
|---|---|---|
| C5, 100 µF / 50 V bulk | Panasonic `EEEFK1H101P` candidate | Verify land pattern/body clearance and final sourcing choice |
| J2 USB-C | GCT `USB4216-03-A` candidate | Verify footprint, pin mapping and 3D/mechanical fit |
| Q1 reverse-polarity P-MOS | onsemi `NTD20P06LT4G` candidate | Verify package/pinout and electrical suitability |
| Q5 fan MOSFET | Infineon `IRLML2060TRPBF` candidate | Verify SOT-23 pinout and final part choice |
| U2 USB-UART | Silicon Labs `CP2102-GM` candidate | Re-check exact package/pin compatibility before any substitution |
| U3 USB ESD protector | Akyga Semi `AKS1201` candidate | Verify pinout, ESD characteristics, capacitance and footprint |
| J7 JST GH NTC connector | Exact `BM02B-GHS-TBT` preferred | Preserve top-entry mechanics; use another supplier rather than change geometry without reason |

The old procurement entry for the AP66200-era `L1 = 8.2 µH` is **superseded** by the L7987L
redesign. Current `L1` is the new 15 µH buck inductor and must be sourced as part of the new
buck pass.

---

## PCB and production state

The schematic/netlist redesign is ahead of the board.

The current PCB still contains:

- the AP66200 footprint;
- legacy `/Power/VCC_AP66200` routing/net data;
- the previous power-stage placement/routing.

Therefore the next hardware implementation phase is **not procurement**. It is:

1. finish small schematic naming/metadata cleanup that does not require purchasing choices;
2. assign/verify final footprints together with the later MPN pass;
3. update PCB from the current schematic, replacing the AP66200 stage;
4. place and route the L7987L power loop according to the ST layout guidance;
5. rerun ERC and DRC;
6. regenerate BOM, positions, fabrication outputs and production netlist;
7. only then reconcile those generated outputs against `BOM TME` and finalize the order.

Until those steps are complete, the old production exports are historical only.
