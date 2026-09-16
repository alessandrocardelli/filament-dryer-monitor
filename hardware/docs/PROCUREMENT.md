# Procurement and footprint state

Status: **2026-09-16 — sourcing basis retained; PCB routed; release reconciliation still open** on branch **pcb/l7987l-layout**.

Hardware checkpoint before this documentation refresh: **9da46e7953f801073882fd1934802fa8ace1f1c2** (updated layout).

## Source of truth

The current KiCad schematic and PCB on the active branch are authoritative for implementation. A freshly exported netlist is authoritative for schematic connectivity only when it has been regenerated after the latest schematic edit.

The purchasing source is the Google Sheet **Filament Dryer Monitor — BOM finale Mouser**, tab **BOM TME**, together with manufacturer/MPN fields synchronized into KiCad.

If documentation disagrees with the actual KiCad files, the KiCad implementation wins. If the live purchasing sheet disagrees with current KiCad references, reconcile it before ordering.

## Current PCB/manufacturing state

The board is synchronized to the L7987L redesign and substantially routed:

- U5 = L7987L is present on B.Cu;
- AutoEN is present on B.Cu;
- legacy AP66200 and /Power/VCC_AP66200 are absent;
- both internal copper layers are solid GND planes;
- local external copper/zones exist for 24V_PROT, 3V3_BUCK, 3V3_MCU, HEATER_SW, LX and GND;
- USB routing is configured for a 90 Ω differential target;
- fresh DRC dated 2026-09-16 reports **0 errors and 0 unconnected pads**.

The PCB is still **not manufacturing-ready** because U5 exposed-pad thermal/paste implementation is open, accepted D012 is not currently implemented, stored netlist/ERC are stale after the latest MCU schematic edit, DRC warnings remain to review, and final production outputs have not been regenerated.

## Current DRC warning set

Fresh DRC has 14 active warnings plus 1 excluded warning.

Active:

- library-footprint mismatch: J4, U4, J6, J1, J5, J3;
- eight BZ1 silkscreen-over-copper warnings.

Excluded:

- ESP32 silkscreen clipped by board edge.

These warnings require deliberate release review even though there are no DRC errors.

## Closed sourcing basis

The component-selection basis remains:

- U5 = L7987L;
- U1 = TLV1701AIDBVR;
- Q7 = MMBT3904;
- L1 = SRN6045-150M, 15 µH;
- D7 = STPS2L60A;
- R29 = 47.5 kΩ ILIM;
- R33 = 16 kΩ, R34 = 16 kΩ, R35 = 1.13 kΩ 0.1%, R36 = 49.9 kΩ;
- C1 = Murata GRM32EC72A106KE05L, 10 µF / 100 V / X7S;
- C10 = Taiyo Yuden LMK325B7476KM-PR / current number MSASL32MSB7476KPNB25, 47 µF / 10 V / X7R;
- C3 = Murata GRJ21BC72A105KE11L, 1 µF / 100 V / X7S;
- C21 = Murata GCM188R71E105KA64J, 1 µF / 25 V / X7R;
- upstream C5 = 100 µF / 50 V bulk reservoir.

### D012 sourcing/implementation conflict

Decision D012 accepted **two physically local 1 µF / 100 V capacitors** for L7987L VIN/VCC, historically C3 + C22.

Current source state:

- Power.kicad_sch contains C3 as the only 1 µF / 100 V part;
- there is no current C_vin_byp1 part in the Power sheet;
- current C22 is in MCU.kicad_sch and is the CP2102 REGIN 1 µF / 25 V capacitor.

The electrical decision has not been withdrawn. Before release, restore the second 1 µF / 100 V VIN bypass under a non-conflicting reference or explicitly reopen D012.

## Live BOM TME reconciliation issue

The live BOM TME currently contains:

- **C15,C17,C21,C22** -> 1 µF / 25 V X7R 0603, Murata GCM188R71E105KA64J;
- **C3,C22** -> 1 µF / 100 V X7S 0805, Murata GRJ21BC72A105KE11L.

That is internally inconsistent with the current KiCad source because C22 cannot be both parts. Do not order/finalize the production BOM until D012 is resolved and the stale C22 entry is removed from the wrong row.

## Footprint audit

The main package/pin-number audit remains the basis of the design.

Important non-trivial footprint decisions:

| Ref | Final MPN | Footprint decision |
|---|---|---|
| C5 | Panasonic EEEFK1H101P | Custom Panasonic size-F footprint, Ø8 × 10.2 mm, manufacturer land-pattern basis |
| J2 | GCT USB4216-03-A | Custom USB-C footprint with fully SMT shell stakes |
| L1 | Bourns SRN6045-150M | Custom footprint based on Bourns recommended PCB layout |
| U2 | Silicon Labs CP2102-GM | Custom QFN28 footprint based on Silicon Labs CP2102/9 package guidance |
| SW1–SW6 | GCT SWT0110-020010SSA | Custom footprint based on GCT mechanical drawing |
| BZ1 | Loudity LD-BZEL-T67-0808 | Custom footprint with conservative terminal lands |
| U5 | ST L7987L | SamacSys_Parts:SOP65P640X120-17N; HTSSOP-16 exposed pad |

### CP2102-GM footprint state

Current custom U2 footprint:

- pitch 0.50 mm;
- perimeter lands 0.95 × 0.28 mm;
- exposed pad 3.25 × 3.25 mm;
- footprint solder-mask expansion +0.06 mm;
- exposed-pad paste split into 3 × 3 apertures of 0.9 × 0.9 mm.

Current REGIN bypass:

- C22 = 1 µF / 25 V / X7R / 0603;
- Murata GCM188R71E105KA64J;
- connected between 3V3_MCU and GND.

### Strict symbol/pin-numbering audit

The earlier audit corrected and retained:

- D1 = BZX84C15-7-F mapping;
- Q5 = IRLML2060TRPBF mapping;
- U2 = classic CP2102-GM symbol and custom QFN28 footprint.

Current USB board mapping is:

- U2 pin 4 = D+;
- U2 pin 5 = D−;
- U3 1↔6 = D+ channel;
- U3 3↔4 = D− channel;
- J2 A6/B6 = D+;
- J2 A7/B7 = D−.

The stored netlist predates the last MCU correction and still shows the old reversed J2 labels. Regenerate it before final pinout sign-off.

## Physical/manufacturing items still open

- **U5:** current EP is 3.2 × 3.2 mm with no vias inside the exposed pad; final thermal-via and stencil/paste-window strategy remains open.
- **J5:** real HALJIA XH-compatible connector fit/polarization still needs physical verification.
- **U4:** final ESP32 antenna keepout/board-edge review remains part of release.
- **J4 OLED:** custom/mechanical verification remains relevant if fitted.
- **BZ1:** first-board solderability/fit remains worth checking because manufacturer land-pattern data are limited.
- current DRC footprint-library mismatch warnings must be deliberately reviewed rather than ignored generically.

## Current stackup / plane decision

The 4-layer stackup remains:

- F.Cu 35 µm;
- 0.10 mm FR4;
- In1.Cu 35 µm **solid GND**;
- 1.24 mm FR4 core;
- In2.Cu 35 µm **solid GND**;
- 0.10 mm FR4;
- B.Cu 35 µm.

No internal 3V3/24 V power planes are used. Power distribution remains on external copper.

## Netclasses / USB

| Class | Clearance | Track | DP width / gap | Via dia/drill |
|---|---:|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.20 / 0.25 mm default | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | — | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | — | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | — | 0.80/0.40 mm |
| USB | 0.20 mm | **0.20 mm** | **0.20 / 0.25 mm** | 0.60/0.30 mm |

USB tuning profile USB_90R targets 90 Ω differential on B.Cu referenced to In2.Cu.

## Production release sequence

1. resolve D012 and the C22 reference conflict;
2. finalize U5 EP thermal-via/paste implementation;
3. regenerate netlist and ERC from the latest schematic;
4. review/close DRC warnings and run final DRC after remaining changes;
5. perform final electrical/layout/mechanical/antenna review;
6. reconcile BOM TME against current KiCad references and MPNs;
7. regenerate BOM, CPL/position data, Gerbers/drills and production netlist from the same final revision;
8. perform final fabrication/assembly review;
9. release fabrication only after all above items are closed.

Existing generated production outputs remain historical until that sequence is complete.
