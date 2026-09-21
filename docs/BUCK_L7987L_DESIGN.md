# L7987L buck redesign — current design record

Status: **schematic design basis accepted; bare-PCB fabrication files submitted, first-board validation pending** (2026-09-21).

Repository: alessandrocardelli/filament-dryer-monitor  
Current hardware branch: **pcb/l7987l-layout**  
Fabrication-source checkpoint before documentation-only commits: **74a3000127371ac6c3b3b7197f9036dec5b58eef** (2026-09-17, `Production files`).  
Hardware Design Manual-based schematic review completed on **2026-09-04**.  
The saved 2026-09-17 PCB DRC reports 0 active errors, 0 active warnings, 0 unconnected pads and 1 deliberately excluded U4 silkscreen/board-edge warning. The 2026-09-17 ERC reports 0 errors / 0 warnings.

The actual KiCad schematic and PCB override this document if they disagree. The active electrical implementation is in hardware/Power.kicad_sch; the current board implementation is hardware/Filament_Dryer_Monitor.kicad_pcb.

> **Current prototype / fabrication state**
>
> The board contains the L7987L redesign, not legacy AP66200. U5 retains six 0.60/0.30 mm GND vias arranged 3+3 *outside* the exposed pad for hand assembly. The current netlist/ERC/DRC and suffixed production exports are dated 2026-09-17; bare-PCB Gerbers/drills were uploaded to JLCPCB and the CAM production file was reviewed. GitHub does not verify subsequent fabrication/shipping status. No JLC PCBA or stencil was ordered/planned. C11 procurement remains unresolved with TME; the electrical design has not been changed.

> **Design-review result**
>
> The L7987L + AutoEN electrical calculations accepted on 2026-09-04 remain the design basis: ILIM/L1 selection, capacitance/compensation sensitivity and AutoEN corners are not reopened by PCB routing alone. Physical ringing, thermal behavior, real load transients, fault current and AutoEN timing remain prototype validation items.

## 1. Design objective

Replace the previous AP66200 24 V -> 3.3 V buck with a sourceable high-voltage regulator while preserving the rest of the board architecture.

Selected regulator:

- STMicroelectronics **L7987L**;
- asynchronous buck;
- 4.5–61 V operating input range;
- 2 A capability;
- adjustable switching frequency;
- programmable peak current limit;
- external compensation.

Primary manufacturer source:

- STMicroelectronics, **L7987L — 61 V, 2 A asynchronous step-down switching regulator with adjustable current limitation**, DocID026362 Rev. 4;
- https://www.st.com/resource/en/datasheet/l7987l.pdf

AutoEN comparator:

- TI **TLV1701AIDBVR**;
- 2.2–36 V supply;
- rail-to-rail input common-mode;
- open-collector output;
- https://www.ti.com/lit/ds/symlink/tlv1701.pdf

ST layout reference:

- **STEVAL-ISA198V1**, L7987L demonstration/evaluation board;
- use the official ST layout and Gerbers as the primary topological/geometric reference, adapted to this board's four-layer stackup and actual footprints.

---

## 2. Electrical requirements

### Load topology

Heater and fan are not supplied by the 3.3 V buck.

- Heater: `24V_PROT`, low-side switched, approximately 1.6 A at 24 V.
- Fan: `24V_PROT`, low-side switched, approximately 0.2 A.
- Buck: low-voltage electronics only.
- Buck output -> `3V3_BUCK` -> FB1 -> `3V3_MCU`.

3.3 V load budget used during design:

- conservative normal load ~0.66 A;
- including SHT45 internal-heater case ~0.76 A;
- adopted buck design target **1.0 A**.

Input engineering range: **21.6–26.4 V** around 24 V nominal. This ±10% range is a project engineering assumption, not a verified tolerance specification of the original eSUN supply.

Original supply is 24 V / 48 W. The combined design-point load leaves limited source margin; a future 24 V / 3 A / 72 W minimum replacement remains a sensible target, but no replacement supply is selected in this record.

---

## 3. Implemented topology

### Input / VCC / bypass

24V_PROT directly supplies L7987L VIN1, VIN2 and VCC.

Current source implementation:

- C1 = 10 µF / 100 V X7S, main local input ceramic.
- C3 = 1 µF / 100 V X7S, local L7987L VCC/input bypass.
- C2 = 100 nF, local TLV1701 supply bypass.
- C5 = 100 µF / 50 V, upstream bulk reservoir retained.

**VIN/VCC local ceramic requirement — resolved:** ST requires a ceramic of 1 µF or higher across VIN-to-power-GND and another across VCC-to-IC-GND, close to the device. C1 = 10 µF / 100 V already satisfies the VIN-side requirement; C3 = 1 µF / 100 V is the VCC bypass. The additional 1 µF / 100 V buck C22 added on 2026-09-09 was redundant and was deliberately removed in commit `24a9170a`. D018 supersedes historical D012.

### Output / VBIAS

- C10 = 47 µF main pre-bead output capacitor.
- C21 = 1 µF / 25 V VBIAS/output bypass.
- pre-bead node = 3V3_BUCK.
- FB1 feeds 3V3_MCU.

### Unused pins

- U5 pin 12 PGOOD: intentionally NC.
- U5 pin 7 SYNCH: intentionally NC.

## 4. Current reference/value map

### Active devices / magnetics

| Ref | Current device | Role |
|---|---|---|
| U5 | L7987L | Buck regulator |
| U1 | TLV1701AIDBVR | AutoEN comparator |
| Q7 | MMBT3904 | AutoEN EN pull-down |
| L1 | SRN6045-150M, 15 µH | Buck inductor |
| D7 | PMEG6030EP,115 | Catch Schottky; Nexperia 60 V / 3 A CFP5/SOD-128 |

### Buck and AutoEN capacitors

| Ref | Value | Role |
|---|---:|---|
| C1 | 10 µF / 100 V X7S | Main local input ceramic |
| C2 | 100 nF | TLV1701 bypass |
| C3 | 1 µF / 100 V | L7987L VIN/VCC local bypass |
| C4 | 33 nF | Soft start |
| C6 | 100 nF | BOOT-to-LX bootstrap |
| C7 | 330 nF | AutoEN EN timing |
| C8 | 39 pF | Type-III CP |
| C9 | 18 nF | Type-III CF |
| C10 | 47 µF / 10 V X7R | Main buck output capacitor |
| C20 | 560 pF | Type-III CS |
| C21 | 1 µF / 25 V | VBIAS/output bypass |

### Buck and AutoEN resistors

| Ref | Value | Role |
|---|---:|---|
| R1 | 270 kΩ | AutoEN threshold top |
| R2 | 22 kΩ | AutoEN threshold bottom |
| R4 | 47 kΩ | FSW programming |
| R5 | 100 kΩ | TLV1701 open-collector pull-up |
| R6 | 100 kΩ | Comparator output to Q7 base |
| R29 | 47.5 kΩ | ILIM programming |
| R30 | 47 kΩ | Q7 base-emitter pull-down |
| R31 | 100 kΩ | EN pull-up |
| R32 | 15 kΩ | EN pull-down |
| R33 | 16 kΩ | Type-III RF |
| R34 | 16 kΩ | Feedback lower resistor |
| R35 | 1.13 kΩ | Type-III RS |
| R36 | 49.9 kΩ | Feedback upper resistor |

---

## 5. Switching frequency

ST relation:

`FSW [kHz] = 250 + 12500 / RFSW [kΩ]`

With R4 = 47 kΩ:

`FSW ≈ 516 kHz`

The layout is therefore treated as a practical ~500 kHz switcher.

---

## 6. Feedback divider

- R36 upper = 49.9 kΩ.
- R34 lower = 16 kΩ.
- nominal L7987L VFB = 0.800 V.

`VOUT = VFB × (1 + R36/R34) ≈ 3.295 V`

Using ST's feedback-reference limits 0.788–0.812 V gives approximately 3.246–3.344 V before resistor-tolerance/bias terms.

---

## 7. Inductor, ripple and catch diode

L1 = Bourns SRN6045-150M:

- 15 µH nominal;
- ±20% tolerance;
- DCR max ~95.8 mΩ;
- Irms ~1.9 A;
- Isat ~2.3 A, defined by Bourns at 30% inductance reduction.

At ~516 kHz over the 21.6–26.4 V design range:

- nominal inductor ripple ~0.36–0.37 A p-p;
- normal peak at 1.0 A load ~1.18–1.19 A;
- with L at -20%, normal high-line peak ~1.23 A.

The actual released D7 is **Nexperia PMEG6030EP,115, 60 V / 3 A Schottky**, CFP5/SOD-128 (KiCad MPN `PMEG6030EP.115`). Earlier STPS2L60A references in design notes are historical and **not the populated source**. First-board probing must still verify LX ringing, D7 temperature and switching stress; the rating does not by itself establish measured margin.

---

## 8. Current-limit programming — CLOSED

R29 = 47.5 kΩ.

ST relation:

`RILIM = 27 kΩ × 3 A / ILIM`

Nominal project value:

`ILIM,nom ≈ 1.705 A`

ST does not publish a guaranteed min/max specifically at 47.5 kΩ. For component-selection review, a deliberately conservative engineering envelope based on the published ST current-limit spread plus R29 tolerance was used:

- lower estimate ~1.42 A;
- upper stress estimate ~2.15 A.

The lower estimate remains above the normal worst-case operating peak (~1.23 A). The upper estimate remains below L1's ~2.3 A Isat. The 2.15 A number is an engineering stress estimate, not a manufacturer-guaranteed maximum at 47.5 kΩ.

**Gate: CLOSED for schematic sign-off.** Prototype fault-current/current-waveform measurement remains required.

---

## 9. Input/output capacitance — CLOSED FOR SCHEMATIC SELECTION

Sourced power capacitors:

- C1 = Murata `GRM32EC72A106KE05L`, 10 µF / 100 V / X7S / 1210.
- C10 = Taiyo Yuden `LMK325B7476KM-PR`, current number `MSASL32MSB7476KPNB25`, 47 µF ±10% / 10 V / X7R / 1210.
- C3 = 1 µF / 100 V local bypass.
- C21 = 1 µF / 25 V VBIAS bypass.
- C5 = 100 µF / 50 V upstream bulk.

Manufacturer data confirms nominal ratings and provides bias/temperature characterization or simulation data. Those curves/models are not treated as a guaranteed minimum capacitance specification. The schematic review therefore used conservative reduced-capacitance stress cases rather than inventing `Ceff,min`.

A severe 23.5 µF C10 stress case was checked during review; it did not invalidate the selected compensation architecture or the recorded crossover relative to the ~500 kHz switching frequency.

**Gate: CLOSED for schematic selection.** Physical load-transient behavior remains a bring-up measurement.

---

## 10. Type-III compensation

Final network:

- R33 / RF = 16 kΩ;
- C9 / CF = 18 nF;
- C8 / CP = 39 pF;
- R35 / RS = 1.13 kΩ;
- C20 / CS = 560 pF.

Recorded final-value simulation results:

- crossover ~59.1 kHz;
- phase margin ~64.9°;
- gain margin ~19.6 dB.

Recorded transient run (~0.33 A -> 1.32 A):

- VOUT,min ~3.254 V;
- VOUT,max ~3.361 V;
- inductor-current peak ~1.59 A;
- clean recovery.

These are session-recorded SIMPLIS/eDSim results, not measurements from physical hardware.

---

## 11. AutoEN rationale and implementation — CLOSED

Deep-short simulation of the L7987L model showed COMP could remain strongly saturated during a persistent fault. Resetting EN cleared the loop state and produced a clean restart. The final design therefore adds an external AutoEN detector that forces EN low when COMP rises above a chosen threshold.

U1 = TLV1701AIDBVR:

- IN+ -> L7987L COMP;
- IN− -> `VREF_FAULT`;
- V+ -> `24V_PROT`;
- V− -> GND;
- C2 = 100 nF local bypass;
- open-collector OUT uses R5 pull-up and R6/Q7 drive.

Threshold divider:

- R1 = 270 kΩ from `24V_PROT` to `VREF_FAULT`;
- R2 = 22 kΩ from `VREF_FAULT` to GND.

Nominal threshold:

- 21.6 V input -> ~1.627 V;
- 24.0 V -> ~1.808 V;
- 26.4 V -> ~1.989 V.

Corner review included input range, ±1% divider tolerance, conservative 100 ppm/K opposing TCR and TLV1701 input-error terms. Reviewed COMP trip window is approximately **1.56–2.07 V**.

EN network:

- R31 = 100 kΩ from 24V_PROT to EN;
- R32 = 15 kΩ EN to GND;
- C7 = 330 nF EN to GND;
- Q7 collector pulls EN low during fault.

Nominal VEN ~3.13 V at 24 V; reviewed low-line/tolerance EN-high remains ~2.7 V or higher, comfortably above the L7987L maximum enable-high threshold of 0.9 V.

Recorded persistent-fault simulation showed repeated shutdown/retry and clean eventual restart after the fault was removed.

### 11.1 Why AutoEN is required — foldback lock, identified 2026-09-09

The original justification recorded here was only "COMP remained saturated in a deep-short
simulation". The actual mechanism has since been isolated and is more specific than that.

Overload/recovery simulation, SIMPLIS, testbench derived from
`TB_L7987L_STEVAL-ISA198V1_LoadTransient` with project values
(24 V in, 3.3 V out, L1 15 µH / 95.8 mΩ, RILIM 47.5 kΩ, RFSW 47 kΩ,
COUT 23.5 µF and CIN 7.5 µF as DC-bias-derated effective values).
Load stepped 1 A -> 3 A at 20 ms, released at 45 ms, run to 60 ms.

Observed sequence:

1. During the fault FB falls below the 400 mV `VFOLD` threshold and the peak current limit
   folds back to one third. Inductor current drops from ~1.2 A to ~0.45 A, confirming that
   the model does implement the datasheet section 4.5 protections — the earlier suspicion
   that this was an incomplete-model artifact is disproved.
2. After the fault is removed the converter can only deliver the folded-back current.
3. ~0.45 A into the 3.3 Ω recovery load settles the output at ~1.5 V.
4. At 1.5 V the divider puts FB at ~0.36 V, still below the 400 mV threshold.
5. Foldback therefore never releases. The converter latches in a stable sub-threshold
   operating point and does not return to 3.3 V within the simulated window.

The margin is thin: leaving foldback needs roughly 1.65 V at the output and the circuit
settles at ~1.5 V, about 150 mV short. The datasheet statement that the full limit is
restored as soon as FB rises above `VFOLD` is correct but not sufficient — nothing in the
device drives FB back across the threshold.

The device protection prevents inductor current runaway. It does not guarantee recovery.
Those are separate properties and only the first is covered by the datasheet analysis.

### 11.2 Measured COMP levels and threshold placement

From the same simulation:

| Condition | COMP |
|---|---|
| Steady state, 1 A load | ~0.25 V |
| Persistent fault | ~3.3 V (saturated) |

The R1/R2 threshold window of approximately 1.56–2.07 V sits between the two with wide
margin on both sides. This supersedes the earlier tolerance-only corner analysis with
observed levels, and removes a previously raised concern that input-rail sag from heater
switching could drag the threshold into a false trip: at 0.25 V steady-state COMP the
available margin is roughly a factor of six.

### 11.3 Single-failure direction — R5 not populated at first assembly

U1 has an open-collector output. If U1 is absent, damaged, or has an unsoldered pin, the
output node is pulled up through R5, Q7 conducts and EN is held low. The board produces no
3.3 V rail at all.

This is the wrong failure direction for an auxiliary protection block: a fault-recovery
accessory should degrade to "unprotected but working", not to "dead board". The practical
exposure is assembly rather than field failure — U1 is a hand-soldered SOT-23-5, and a
single cold joint at first power-up presents as a dead buck, sending debugging effort to
U5, L1 and the feedback network while the regulator itself is healthy.

Resolution is assembly order, not a schematic change. R5 is left unpopulated for initial
bring-up; R30 then holds the Q7 base at ground, Q7 stays off and EN is free. See
`docs/ASSEMBLY.md`.

**Gate: CLOSED for schematic sign-off.** Comparator propagation, EN timing, COMP waveform and restart behavior remain prototype measurements.

---

## 12. ERC / schematic-regression state

The current saved `hardware/ERC.rpt` is dated **2026-09-17 22:18:03** and reports **0 errors / 0 warnings**. The regenerated `hardware/Filament_Dryer_Monitor.net` is dated 2026-09-17 21:13:01 and includes the corrected USB-C J2 D+/D− mapping. The older 2026-09-16 external-GND error and reversed netlist were pre-release checkpoints and are not current-source findings.

The current ERC report lists specific ignored check classes. The repository workflow/branch trigger should be checked before assuming an automatic ERC run on future feature-branch hardware changes. A documentation-only commit does not change the validated KiCad source or require a fresh hardware report.

**Gate:** schematic release evidence is recorded; electrical and thermal behavior remains to be verified on the assembled prototype.

## 13. Sourcing / footprint state

The current released purchasing BOM is the live Google Sheet `BOM TME`, reconciled against the **suffixed** `hardware/production/Filament_Dryer_Monitor_bom.csv` and the current schematic netlist. The legacy unsuffixed `hardware/production/bom.csv` still describes the previous AP66200/CP2102N build and must **not** be used for this design. The old suffixed export's `LCSC Part #` heading is not a reliable TME code; use `BOM TME` column `Codice TME`.

Important current package/footprint facts:
- C5 Panasonic EEEFK1H101P -> custom Panasonic size-F footprint.
- J2 GCT USB4216-03-A -> custom USB-C footprint.
- L1 SRN6045-150M -> custom Bourns footprint.
- U2 CP2102-GM -> custom QFN28 footprint with 0.95 × 0.28 mm perimeter pads, +0.06 mm mask expansion, 3.25 × 3.25 mm EP and 3 × 3 array of 0.9 mm paste apertures.
- SW1–SW6 GCT SWT0110-020010SSA -> custom footprint.
- BZ1 LD-BZEL-T67-0808 -> custom footprint with conservative terminal lands.
- U5 L7987L -> SamacSys_Parts:SOP65P640X120-17N HTSSOP-16 with exposed pad and six nearby peripheral ground/thermal vias; no via-in-pad for **U5**.

Current CP2102 REGIN bypass is C22 = GCM188R71E105KA64J, 1 µF / 25 V / X7R / 0603. VIN/VCC bypass choices are C1 10 µF / 100 V and C3 1 µF / 100 V; no C22 duplicate.

**Current supply-only exception, not a circuit change:** one-board TME order placed 2026-09-20. C11 = Samsung `CL21A226MAYNNNE`, 22 µF / 25 V / X5R / 0805 on `3V3_MCU` is **not confirmed available for dispatch**; TME is investigating an unlocatable warehouse item. A TDK 22 µF / 16 V / X5R / 0805 equivalent (`C2012X5R1C226M125AC`, TME `C2012X5R1C226MAC`) has been discussed **only as a possible substitution**. No product swap or cancellation is authorized. Keep C11 in the final hand assembly and wait for TME's answer; check usable capacitance at 3.3 V before changing the sourced MPN.

The U5 exposed-pad strategy remains closed for **manual** prototype assembly: lightly pre-tin the pad, use flux and hot air. If the assembly flow later switches to stencil/reflow or external PCBA, reopen solder-paste/via-treatment decisions explicitly.

The OLED, off-board SHT45, independent heater cutoff, cabling/mating JST parts and mechanical fasteners are not PCB-mounted BOM lines; check them separately. Full ordering details and open work: `hardware/docs/PROCUREMENT.md`.

## 14. PCB synchronization / routing checkpoint

Verified at released hardware checkpoint `74a3000`:

- current PCB contains U5 = L7987L on B.Cu;
- old AP66200 and /Power/VCC_AP66200 are absent;
- AutoEN is present on B.Cu;
- local B.Cu zones exist for 24V_PROT, /Power/3V3_BUCK, 3V3_MCU, GND, HEATER_SW and LX;
- both inner layers contain full-board GND zones;
- fresh DRC reports **0 unconnected pads**.

Therefore schematic-to-board integration and basic routing connectivity are no longer open tasks. The remaining gate is engineering/manufacturing review.

The stored netlist was regenerated on 2026-09-17 after the MCU correction and now maps J2 A6/B6 to D+ and A7/B7 to D−. Verify USB functionality on the assembled board; there is no current-source reversed-netlist blocker.

## 15. Current stackup and grounding decision

KiCad stackup:

- F.Cu 35 µm;
- 0.10 mm FR4, Er 4.5;
- In1.Cu 35 µm;
- 1.24 mm FR4 core, Er 4.5;
- In2.Cu 35 µm;
- 0.10 mm FR4, Er 4.5;
- B.Cu 35 µm;
- total 1.6 mm.

**Project decision:**

- In1.Cu = solid GND plane;
- In2.Cu = solid GND plane;
- no 3V3 or 24 V internal power planes;
- buck local power distribution uses B.Cu copper/tracks/zones.

For the B.Cu buck, In2 is the nearest GND reference only 0.10 mm away.

### PGND / SGND implementation

ST explicitly distinguishes power-ground and signal-ground current paths in the L7987L layout guidance. This project implements that intent **without separate GND nets and without splitting the internal planes**.

Same electrical `GND` net:

- high-current/pulsed return region: C1 negative, D7 anode, C10 negative;
- quiet/signal return region: U5 pin 16 + exposed pad, C3 negative, C21 negative and sensitive control returns.

Both use short local B.Cu connections/vias into the same continuous In1/In2 GND planes. The design goal is to prevent the pulsed input/catch-diode/output return current from flowing through the quiet local return geometry around FB/COMP.

Do not create a split In2 PGND/SGND plane as an interpretation of the ST diagram.

---

## 16. ST layout guidance applied to the current PCB

Verified ST guidance that controls the current layout:

- minimize the high-pulsed-current step-down loop;
- place a small ≥1 µF bypass as close as possible to the input-voltage pin for both VIN and VCC;
- keep FB divider close to the device and away from high-current paths;
- connect EP to signal GND while avoiding high current through signal-ground copper;
- use a short BOOT-to-LX capacitor connection;
- use the demonstration-board layout as the device-specific reference.

Reference-board component mapping used during placement:

| ST demo | Project |
|---|---|
| U1 | U5 |
| D1 | D7 |
| L1 | L1 |
| C1+C2 main input | C1 10 µF |
| local 1 µF input/VCC bypass | C3 1 µF |
| C5 output | C10 |
| C7 bootstrap | C6 |
| C10 soft start | C4 |
| R8 FSW | R4 |
| R9 ILIM | R29 |
| R6/C9/C11 | R33/C9/C8 compensation |
| R5/C8 | R35/C20 compensation |
| R7/R11 feedback | R36/R34 |

C21 has no direct demonstration-board equivalent in that mapping; it is the project VBIAS/output bypass.

### Placement facts to preserve

- In the current physical board view, VIN/VCC are on the **left side of U5**.
- C3 belongs closest to VIN/VCC.
- C1 belongs on the same VIN side and may be slightly farther out than C3.
- C21 stays close to pin 1 VBIAS; it does not need to be moved merely to make a continuous surface PGND strip between C1 and D7.
- C6 is BOOT-LX, not a GND return component.
- FB/COMP network stays in the quiet region.

### Bottom-side orientation process rule

Do not infer B.Cu pad direction from raw local footprint coordinates. A previous manual transform led to incorrect rotation recommendations. Use KiCad/pcbnew absolute pad positions or the actual board view with nets/pad numbers.

---

## 17. Current PCB open items

### U5 exposed pad / thermal implementation — CLOSED for planned hand assembly

U5 pad 17 is the 3.2 × 3.2 mm exposed GND / SGND pad centered with U5 at approximately `(174.649, 85.116)`.

The accepted implementation is deliberately **not via-in-pad**. Six GND vias are placed immediately outside the EP:

- 0.60 mm via diameter;
- 0.30 mm drill;
- three vias in a row above U5 at approximately y = 82.9 mm;
- three vias in a row below U5 at approximately y = 87.35 mm;
- x positions around 173.75 / 174.65 / 175.55 mm.

This arrangement was implemented in commit `c9dc4129` and remains present in the current PCB. It gives the exposed pad a short thermal/electrical path to the internal GND planes without placing open vias in the solderable EP, avoiding the via-in-pad solder-wicking problem that motivated the 3+3 peripheral arrangement.

For the planned prototype flow, U5 is assembled manually with a very light pre-tin on the exposed pad, flux and hot air. The footprint still contains full-area B.Paste on pad 17, but that layer is not used to meter solder in this hand-assembly process and is therefore not a prototype release blocker.

If the manufacturing method changes to stencil/reflow or external PCBA, reopen the paste-window and via-treatment decision for that process. First-board U5 temperature measurement remains a validation requirement. Decision D019 records the closure.

### VIN/VCC bypass status

Closed by D018. C1 provides the required local VIN ceramic capacitance (10 µF ≥ 1 µF) and C3 provides the required 1 µF VCC bypass. No additional dedicated buck C22 is required. Final layout review must still verify that C1/C3 and their respective GND return paths remain genuinely local.

### Buck routing review

All current nets are connected, but final quality review is still required for:

- C1/C3 -> VIN/VCC loop;
- C6 BOOT-LX;
- U5 LX -> D7/L1 switching region;
- L1 -> C10 -> 3V3_BUCK -> FB1;
- FB/COMP routing and quiet return;
- AutoEN routing;
- high-current versus quiet GND via entry.

No generic wide LX netclass is required; switch-node copper is controlled geometrically and kept compact.

## 18. Netclasses and USB geometry

Current classes:

| Class | Clearance | Track | Via dia/drill |
|---|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | 0.80/0.40 mm |
| USB | 0.20 mm | **0.20 mm** | 0.60/0.30 mm |

USB geometry is now closed for the current stackup:

- B.Cu signal layer;
- In2.Cu reference;
- 0.10 mm FR4, Er 4.5;
- 90 Ω differential target;
- width 0.20 mm;
- gap 0.25 mm;
- profile USB_90R.

Assignments use USB_D+, USB_D−, USB_CONN_D+, USB_CONN_D−.

The long U2-to-U3 pair stays on B.Cu with no vias. The short Type-C D+ duplicate-pad crossover uses two local vias and a short F.Cu bridge with nearby GND stitching; D− remains on B.Cu.

## 19. DFT / debug access

Existing access includes `24V_PROT`, `3V3_MCU` and GND.

Before routing freeze, decide whether to add convenient test access for:

- `3V3_BUCK` before FB1;
- COMP;
- EN / AutoEN control node;
- local GND near the buck.

Avoid creating a large LX test pad. If LX probing is needed, use an existing switch-node pad with a very short probe ground arrangement.

---

## 20. DRC / manufacturing state

The saved `hardware/DRC.rpt` was generated on **2026-09-17 22:18:19** from the released routed PCB: **0 active errors / 0 active warnings / 0 unconnected pads / 0 footprint errors**, plus one **excluded** U4 B.Silkscreen-to-board-edge `silk_edge_clearance` item. The old 2026-09-16 list of 14 active warnings is obsolete.

Current suffixed production BOM/position files and final Gerbers/drills were generated for the release-source checkpoint `74a3000`. The final Gerbers/drills were uploaded to JLCPCB for **bare-PCB** production; a supplier CAM/production ZIP was received and compared with the uploaded Gerbers. This review does not by itself confirm subsequent approval, manufacturing or shipment, and does not replace incoming physical inspection or prototype validation.

No hardware change is made by this documentation update. Do not regenerate or overwrite the released fabrication outputs merely to reflect a later supplier replacement of one same-footprint component; record an actual assembly substitution explicitly and require fresh design outputs only if the hardware itself changes.

## 21. Next implementation sequence

1. Wait for the TME warehouse investigation of C11 and confirm dispatch of the other ordered components. Resolve any substitute/refund/reorder cost **before** changing the purchasing source.
2. Audit actual delivered parts and the 33 component references marked in-house; verify availability of off-board OLED, SHT45, thermal cutoff, cabling/mating connectors and mounting hardware.
3. Inspect the delivered bare board (JLCPCB status/stackup, mask, drill and component fit), especially U2 solderability and U4 antenna edge.
4. Hand assemble with **R5 initially unpopulated**; verify 3.3 V before installing R5 and enabling AutoEN.
5. Perform first-board USB, buck/load-transient, LX ringing, D7/U5/L1 thermal and current-limit/AutoEN tests. Complete independent heater TCO/NTC safety validation before normal heating.
6. Any changes resulting from real-board tests belong to a new documented revision, not a silent rewrite of the already submitted fabrication checkpoint.

**Current project gate:** the accepted L7987L + AutoEN electrical decisions and the 2026-09-17 hardware release remain the source of truth. The open tasks are **physical procurement, assembly and testing**; C11 is the one supplier-disclosed unlocated PCB-mounted part, not a resolved substitution.
