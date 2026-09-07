# Procurement and footprint state

Status: **2026-09-07 — sourcing/footprint/schematic review closed; PCB synchronized and in layout/routing phase** on branch **`pcb/l7987l-layout`**.

Hardware/layout checkpoint before this documentation refresh: `6e74f76d8d6877fab63283c1aa520bdbfc149921` (`Layout update`). Documentation-only commits may advance branch HEAD without changing the hardware baseline.

## Source of truth

The current KiCad schematic and PCB on the active branch are authoritative for implementation. A freshly exported netlist is authoritative for schematic connectivity when one is needed for cross-checking.

The purchasing source is the Google Sheet **`Filament Dryer Monitor — BOM finale Mouser`**, tab **`BOM TME`**, together with the manufacturer/MPN fields synchronized into KiCad.

If documentation disagrees with the actual KiCad files, the KiCad implementation wins.

## Current PCB/manufacturing state

The previous warning that the PCB still contained the AP66200 is obsolete.

The current board has been synchronized to the L7987L redesign:

- U5 = L7987L is present on B.Cu;
- the AutoEN block is present on B.Cu;
- the old AP66200 is absent;
- old `/Power/VCC_AP66200` is absent;
- both internal copper layers are currently full GND planes;
- local buck copper/zones and critical routing are still being completed.

The PCB is **not yet manufacturing-ready**. The historical `hardware/DRC.rpt` is dated 2026-08-06 and does not validate the current board. A fresh DRC and regenerated production outputs are required before release.

## Closed sourcing state

The final purchasing pass is closed for the current schematic. Manufacturer and MPN fields were synchronized into KiCad; obsolete LCSC metadata is not the purchasing source for the redesign.

Important buck selections:

- `U5 = L7987L`;
- `U1 = TLV1701AIDBVR`;
- `Q7 = MMBT3904`;
- `L1 = SRN6045-150M`, 15 µH;
- `D7 = STPS2L60A`;
- `R29 = 47.5 kΩ` ILIM;
- `R33 = 16 kΩ`, `R34 = 16 kΩ`, `R35 = 1.13 kΩ 0.1%`, `R36 = 49.9 kΩ`;
- `C1 = Murata GRM32EC72A106KE05L`, 10 µF / 100 V / X7S;
- `C10 = Taiyo Yuden LMK325B7476KM-PR`, current number `MSASL32MSB7476KPNB25`, 47 µF / 10 V / X7R;
- `C3 = 1 µF / 100 V` local L7987L bypass;
- `C21 = 1 µF / 25 V` VBIAS bypass;
- upstream `C5 = 100 µF / 50 V` bulk reservoir retained.

Sourcing/component selection should only be reopened if a later layout/manufacturing/physical-validation issue forces a part change.

## 2026-09-04 schematic review sign-off

The L7987L + AutoEN schematic passed the Hardware Design Manual-based engineering review.

Closed gates:

1. **ERC** — KiCad 10 ERC is CI-enforced. Current reviewed state is one `power_pin_not_driven` error on external GND plus eleven reviewed warnings. The exact error is a KiCad modeling condition and is explicitly waived by the CI gate.
2. **ILIM / L1** — R29 = 47.5 kΩ gives approximately 1.705 A nominal current limit. A conservative engineering envelope of approximately 1.42–2.15 A was used; the upper estimate remains below the SRN6045-150M ~2.3 A Isat. The estimate is not an ST guarantee at 47.5 kΩ.
3. **Capacitance / stability** — actual sourced C1/C10 were reviewed including class-II MLCC bias sensitivity. No unsupported guaranteed `Ceff,min` was claimed. Component selection remains closed; real load-transient validation is a bring-up task.
4. **AutoEN corners** — reviewed COMP trip range approximately 1.56–2.07 V across the engineering corners; EN-high margin remains comfortably above the L7987L enable threshold.

This is schematic sign-off, not manufacturing release.

## Footprint audit — closed 2026-09-02

All BOM components have a deliberate footprint assignment for the selected MPN/package.

Important non-trivial decisions:

| Ref | Final MPN | Footprint decision |
|---|---|---|
| C5 | Panasonic `EEEFK1H101P` | Custom `FilamentDryer:CP_Panasonic_F_8x10.2`; Panasonic FK size F, Ø8 × 10.2 mm, manufacturer land-pattern basis. |
| J2 | GCT `USB4216-03-A` | Custom `FilamentDryer:USB_C_GCT_USB4216-03-A`; fully-SMT shell stakes, not mechanically interchangeable with the previous HRO THT-shell footprint. |
| L1 | Bourns `SRN6045-150M` | Custom `FilamentDryer:L_Bourns_SRN6045`, based on the Bourns recommended PCB layout. |
| U2 | Silicon Labs `CP2102-GM` | Custom `FilamentDryer:CP2102_GM_QFN28_5x5_P0.5_EP3.25`, based on Silicon Labs classic CP2102/9 land-pattern guidance. |
| SW1–SW6 | GCT `SWT0110-020010SSA` | Custom `FilamentDryer:SW_GCT_SWT0110`, based on the GCT mechanical drawing. |
| BZ1 | Loudity `LD-BZEL-T67-0808` | Custom `FilamentDryer:BUZ_Loudity_SMT67_8.5x8.5`; conservative land dimensions because the manufacturer drawing defines terminal zones/body but no numeric recommended PCB land size. |
| U5 | ST `L7987L` | `SamacSys_Parts:SOP65P640X120-17N`; HTSSOP-16 exposed-pad package geometry checked against ST. |

Standard passives and the remaining established SMD packages were checked in the same audit.

### Strict symbol / pin-numbering audit — closed 2026-09-02

The pass checked selected MPN pin numbering against symbol and footprint numbering, not just package size. It found and corrected three real symbol-level issues:

- `D1 = BZX84C15-7-F`: project symbol maps pin 1 = A, pin 2 = NC, pin 3 = K, retaining the standard SOT-23 footprint.
- `Q5 = IRLML2060TRPBF`: final device mapping is 1=G, 2=S, 3=D; the symbol was aligned with that mapping.
- `U2 = CP2102-GM`: dedicated classic CP2102-GM symbol; pins 10 and 13–22 NC, pin 2 `~RI` input, custom QFN28 footprint retained.

The remaining semiconductor mappings were checked without finding another pin-number mismatch.

### Physical/manufacturing review items that remain

These are not unresolved electrical pinout errors:

- `J5`: real HALJIA XH-compatible connector; fit/polarization should be checked against the actual connector despite the 2.50 mm JST-XH footprint.
- `U5`: exposed-pad copper/package geometry is selected, but **thermal-via and stencil/paste aperture strategy is still open in PCB layout**.
- `U4`: final ESP32 antenna keepout and board-edge/copper review remains part of the PCB pass.
- optional OLED footprint: custom and excluded from BOM; mechanical verification only if installed.
- BZ1: first-board solderability/fit should be inspected because the manufacturer does not publish a numeric recommended land pattern.

## Current stackup / plane decision relevant to manufacturing

The current 4-layer stackup is:

- F.Cu 35 µm;
- 0.10 mm FR4;
- In1.Cu 35 µm **solid GND**;
- 1.24 mm FR4 core;
- In2.Cu 35 µm **solid GND**;
- 0.10 mm FR4;
- B.Cu 35 µm.

This is deliberate. No 3V3/24 V internal power zones are planned. Power distribution uses external copper. Do not revert In2 to the earlier mixed power-plane concept without explicitly reopening the recorded design decision in `docs/DECISIONS.md`.

## Current netclasses

Accepted power/default classes:

| Class | Clearance | Track | Via dia/drill |
|---|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | 0.80/0.40 mm |
| USB | 0.20 mm | 0.25 mm | 0.60/0.30 mm |

USB differential-pair width/gap is **not closed** yet and must be verified for the actual stackup before production routing is frozen.

## DFT / debug before routing freeze

Existing access includes `24V_PROT`, `3V3_MCU` and GND.

Still decide whether to add convenient access for:

- `3V3_BUCK` before FB1;
- L7987L COMP;
- L7987L EN / AutoEN control node;
- local buck GND.

These are DFT recommendations, not schematic sign-off blockers. Avoid adding a large LX test pad merely for convenience.

## Current manufacturing-preparation sequence

Sourcing, footprint selection, schematic sign-off and PCB synchronization are no longer blockers. Proceed in this order:

1. finalize U5 exposed-pad thermal/GND via and paste strategy;
2. finish L7987L local PGND/SGND current-return geometry using the same continuous internal GND planes;
3. finish critical buck input, BOOT/LX/diode/inductor, output, feedback/compensation and AutoEN routing against ST/TI guidance;
4. close optional DFT access before routing freeze;
5. calculate/verify USB differential-pair geometry and review continuous reference-plane support;
6. review full-board thermal, GND, antenna, high-current and mechanical constraints;
7. run a **fresh DRC** and resolve/review all current violations;
8. regenerate BOM, CPL/position data, Gerbers/drill files and production netlist from the same final revision;
9. reconcile generated production data against the final `BOM TME` purchasing sheet;
10. release fabrication only after the final review;
11. during first-board bring-up, validate 3.3 V regulation/transients, component temperatures, switching-node stress, current-limit/fault behavior and AutoEN shutdown/recovery timing.

Until steps 1–9 are complete, existing production outputs are **historical and not manufacturing-ready**.