# Procurement and footprint state

Status: **2026-09-21 — bare-PCB fabrication package submitted; one-board TME order placed, C11 supply issue open; manual assembly pending** on branch **pcb/l7987l-layout**.

Fabrication-source checkpoint (before documentation-only commits): **74a3000127371ac6c3b3b7197f9036dec5b58eef** (2026-09-17, `Production files`).

## Source of truth

The current KiCad schematic and PCB on the active branch are authoritative for implementation. A freshly exported netlist is authoritative for schematic connectivity only when it has been regenerated after the latest schematic edit.

The purchasing source is the Google Sheet **Filament Dryer Monitor — BOM finale Mouser**, tab **BOM TME**, together with manufacturer/MPN fields synchronized into KiCad.

If documentation disagrees with the actual KiCad files, the KiCad implementation wins. If the live purchasing sheet disagrees with current KiCad references, reconcile it before ordering.

## Current PCB/manufacturing state

The 2026-09-17 KiCad release contains L7987L U5, AutoEN, IRLML2060TRPBF Q5, Nexperia PMEG6030EP.115 D7 and both continuous inner GND planes; legacy AP66200 is absent. Current `hardware/ERC.rpt`: **0 errors, 0 warnings** (2026-09-17 22:18). Current `hardware/DRC.rpt`: **0 active errors, 0 active warnings, 0 unconnected pads, 0 footprint errors**, with one **excluded** U4 B.Silkscreen/board-edge warning (2026-09-17 22:18). The 2026-09-17 netlist correctly maps USB-C A6/B6 to D+ and A7/B7 to D−.

The current PCB and drill/Gerber package was uploaded to JLCPCB as a **bare 4-layer PCB**, with **manual component assembly** planned (no JLC PCBA or stencil). JLCPCB's CAM/production ZIP was received and reviewed against the original uploaded Gerbers in the order conversation. GitHub does not independently establish whether the order has entered fabrication or shipped; inspect the actual manufacturer order status.

**Correct current release exports:** `hardware/production/Filament_Dryer_Monitor_bom.csv` and `Filament_Dryer_Monitor_positions.csv`. The similarly named **unsuffixed** `hardware/production/bom.csv`, `positions.csv` and `designators.csv` belong to an **obsolete AP66200 / CP2102N-era BOM** and are not an acceptable purchasing or assembly source for this revision. The current suffixed export retains an `LCSC Part #` column with heterogeneous legacy/supplier identifiers, **not necessarily valid LCSC order codes**. Use the live sheet's `Codice TME` field when procuring from TME.

## Current DRC warning set

The previously documented 2026-09-16 14-warning set was resolved in the saved 2026-09-17 DRC. Only the intentionally **excluded** U4 B.Silkscreen/board-edge `silk_edge_clearance` item remains in the report. This does not prove mechanical fit, RF behavior, or final solderability; check the first delivered PCB. A documentation-only commit does not require regenerating ERC/DRC; any new hardware revision does.

## Closed sourcing basis

The component-selection basis remains:

- U5 = L7987L;
- U1 = TLV1701AIDBVR;
- Q7 = MMBT3904;
- L1 = SRN6045-150M, 15 µH;
- D7 = Nexperia PMEG6030EP,115 / KiCad MPN PMEG6030EP.115, 60 V / 3 A Schottky, CFP5/SOD-128;
- R29 = 47.5 kΩ ILIM;
- R33 = 16 kΩ, R34 = 16 kΩ, R35 = 1.13 kΩ 0.1%, R36 = 49.9 kΩ;
- C1 = Murata GRM32EC72A106KE05L, 10 µF / 100 V / X7S;
- C10 = Taiyo Yuden LMK325B7476KM-PR / current number MSASL32MSB7476KPNB25, 47 µF / 10 V / X7R;
- C3 = Murata GRJ21BC72A105KE11L, 1 µF / 100 V / X7S;
- C21 = Murata GCM188R71E105KA64J, 1 µF / 25 V / X7R;
- upstream C5 = 100 µF / 50 V bulk reservoir.

### VIN/VCC bypass sourcing status — resolved

Historical D012 called for an additional dedicated 1 µF / 100 V VIN capacitor, but D018 supersedes that interpretation. ST's VIN requirement is 1 µF or higher; current C1 = 10 µF / 100 V X7S already satisfies it. C3 = 1 µF / 100 V X7S remains the dedicated VCC bypass.

The redundant buck C22 was deliberately removed in commit `24a9170a`. Current C22 is exclusively the CP2102 REGIN 1 µF / 25 V capacitor.

## Live BOM TME status

As read on **2026-09-21**, the live Google Sheet `Filament Dryer Monitor — BOM finale Mouser`, tab **`BOM TME`**, is set to **one PCB for manual assembly** and contains 94 unique PCB-mounted refs in 57 procurement/stock rows. Of these, **43 rows / 61 refs** are marked `TME`, and **14 rows / 33 refs** are marked `In casa` (J5 plus 13 resistor groups). The generated `TME_IMPORT` tab contains the 43 current TME code/quantity rows; it is an **export view, not proof of shipping or payment**. The full reference/MPN comparison against the released schematic netlist and current suffixed production BOM found no missing or duplicate PCB-mounted refs in the 2026-09-19 audit. Count actual packs ordered/shipped separately: TME minimum packs can exceed one-board quantities.

The order was **placed and paid 2026-09-20**. Supplier confirmation initially marked C11 `CL21A226MAYNNNE` as `Disponibile a magazzino` (page 1, position 3). On 2026-09-21 the supplier reported its sole warehouse unit could not be located and is investigating; a separate automatic notice listed `settimana 48/2026` for C11 alone. The **remaining order's release/ship date has not been explicitly confirmed**. Supplier discussions mentioned a possible refund/reorder route, but **no C11 cancellation, substitute order or complete-order cancellation is authorized**. Wait for their definite warehouse answer; do not arrange a second paid shipment without user approval.

**C11 identity / design role:** the released PCB/schematic uses Samsung `CL21A226MAYNNNE` (22 µF, 25 V, X5R, 0805), tied between `3V3_MCU` and GND. This is an actual assembly part, not DNP and not a missing PCB designator. TDK `C2012X5R1C226M125AC` (TME catalog code `C2012X5R1C226MAC`), 22 µF / 16 V / X5R / 0805, is **only a candidate**. Although 16 V nominal exceeds 3.3 V, review usable capacitance under 3.3 V DC bias and exact product identity before approving an assembly/BOM substitution. Do not silently overwrite current C11 MPN or sheet until delivery/substitution is resolved.

The previous C22 duplication is closed: C1 = 10 µF / 100 V VIN, C3 = 1 µF / 100 V VCC, and **C15,C17,C21,C22** share the separate 1 µF / 25 V 0603 sourcing row; C22 is the CP2102 REGIN bypass.

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
| U5 | ST L7987L | SamacSys_Parts:SOP65P640X120-17N; HTSSOP-16 exposed pad; six peripheral GND/thermal vias, no via-in-pad |

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

### U5 exposed-pad / assembly state

U5 pad 17 is 3.2 × 3.2 mm on B.Cu. The implemented thermal/GND path uses six through vias, each 0.60 mm diameter / 0.30 mm drill, immediately outside the pad in two rows of three. The vias are deliberately not placed inside the solderable EP, avoiding open via-in-pad solder-wicking risk.

The prototype assembly method is hand assembly: very light pre-tin on the power pad, flux and hot air. Therefore the full B.Paste shape in the KiCad footprint does not control the planned prototype solder volume and is not a release blocker. If the process changes to stencil/reflow or external PCBA, paste aperture and via treatment must be reopened for that process.

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

The stored netlist was regenerated on 2026-09-17 and correctly maps J2 A6/B6 to D+ and A7/B7 to D−. Functional USB verification remains a first-board task.

## Physical/manufacturing items still open

- **Supplier fulfillment:** confirm TME warehouse outcome for C11, shipping of other available items, final shipped quantities, and payment/refund implications before modifying purchasing status.
- **Stock marked `In casa`:** inspect J5 and all 13 resistor BOM rows for correct values, packages and usable quantities; an `In casa` flag is not an inventory count.
- **Off-board items excluded from the 94 PCB refs:** the SSD1309 OLED for J4, SHT45 sensor for J3, independent heater cutoff/TCO in the HEATER+ wire, mating JST housings/contacts, leads/cable assemblies and mounting hardware. Verify real inventory, physical fit, pinout and wiring; do not infer they are included in the TME order.
- **Incoming bare board:** confirm JLC fab/shipment, inspect 91 × 52 mm PCB outline, four-layer build and solder mask/drilling; check U4 antenna edge, U2 EP solderability, J4/J5/JST mechanical alignment and BZ1 footprint before full population.
- **Hand assembly:** R5 remains **unfitted for first power-up**; C11 remains required for the completed design. Follow `docs/ASSEMBLY.md` and validate the 3.3 V regulator before enabling AutoEN/heater.

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

The 2026-09-17 production-source revision and ERC/DRC reports supersede the historical *pre-fabrication* checklist. For **this submitted prototype**, first confirm the manufacturer's current order/shipping status; then audit delivered bare PCBs and TME parts, close the C11 supply gap and off-board inventory, hand-assemble with R5 omitted initially, and perform first-board electrical/thermal/USB/safety checks. An actual **future hardware change** must restart schematic/PCB synchronization, ERC/DRC, production-output generation and BOM reconciliation under `AGENTS.md`; do not silently update already submitted fabrication data.
