# Procurement and footprint state

Status: **2026-09-04 — TME sourcing, KiCad MPN/manufacturer synchronization, footprint audit and engineering schematic review complete** on branch `redesign/buck-sourcing`.

## Source of truth

The current KiCad schematic and freshly exported netlist are authoritative for electrical connectivity. The `BOM TME` Google Sheet is the purchasing source for selected manufacturer MPNs and TME order codes. The schematic/netlist override documentation if they disagree.

The PCB is **not yet synchronized** with the L7987L redesign. `hardware/Filament_Dryer_Monitor.kicad_pcb` still contains the legacy AP66200 power stage and must not be fabricated in its present state.

## Closed sourcing state

The final purchasing pass is complete for the current BOM. Manufacturer and MPN fields were synchronized into the KiCad schematic; obsolete LCSC sourcing metadata is no longer used as the project purchasing source.

Important final buck values include:

- `U5 = L7987L`;
- `U1 = TLV1701AIDBVR`;
- `L1 = SRN6045-150M`, 15 µH;
- `D7 = STPS2L60A`;
- `R29 = 47.5 kΩ` for ILIM;
- `R33 = 16 kΩ`, `R34 = 16 kΩ`, `R35 = 1.13 kΩ 0.1%`, `R36 = 49.9 kΩ`;
- `C1 = GRM32EC72A106KE05L`, 10 µF / 100 V / X7S;
- `C10 = LMK325B7476KM-PR` (current Taiyo Yuden number `MSASL32MSB7476KPNB25`), 47 µF / 10 V / X7R;
- the remaining capacitor values/dielectrics and resistor tolerances in the schematic/BOM are the sourced values selected during the pass.

Sourcing and footprint selection are therefore **closed** for the present schematic. They should only be reopened if later PCB/layout or physical validation forces a component change.

## 2026-09-04 schematic-review sign-off

A schematic review using the project Hardware Design Manual as the review framework is now **complete** for the current L7987L + AutoEN implementation.

The four electrical gates previously listed as open are closed for engineering schematic sign-off:

### 1. ERC — closed and CI-enforced

A KiCad 10 GitHub Actions workflow now runs ERC on the active schematic.

The fresh report contains:

- one `power_pin_not_driven` error on `#PWR02` GND;
- seven `unconnected_wire_endpoint` warnings;
- two `lib_symbol_mismatch` warnings;
- two `pin_to_pin` warnings.

The single error is a reviewed ERC-modeling condition: external power enters through a passive connector, so KiCad does not see a `Power output` source on the GND net. It is not an electrical open.

The CI gate explicitly waives **only that exact error** and permits only the reviewed warning classes up to their current counts. A new error, changed waiver target, new warning class or warning-count increase fails the gate. `hardware/ERC.rpt` has been refreshed to the current L7987L schematic.

### 2. ILIM / L1 — closed at engineering-review level

- `R29 = 47.5 kΩ` gives approximately **1.705 A nominal** programmed current limit from the ST relation.
- Normal inductor peak at the 1 A design load is approximately **1.18–1.19 A**, or about **1.23 A** with the 15 µH inductor at its -20% tolerance stress case.
- Bourns `SRN6045-150M` is rated approximately **1.9 A Irms / 2.3 A Isat**.

ST does not publish a guaranteed min/max specifically at RILIM = 47.5 kΩ. The review therefore does **not** claim an exact guaranteed `ILIM,max`. A deliberately conservative engineering envelope derived from ST's published current-limit data and R29 tolerance gives approximately **1.42–2.15 A**. The upper estimate remains below L1 Isat and the lower estimate remains above the normal worst-case operating peak.

This closes component selection for the schematic while retaining first-board fault-current measurement as physical validation.

### 3. Effective capacitance / stability — closed for schematic selection

Actual sourced power-stage capacitors were reviewed rather than generic nominal values:

- `C1 = Murata GRM32EC72A106KE05L`, 10 µF / 100 V / X7S;
- `C10 = Taiyo Yuden LMK325B7476KM-PR` / `MSASL32MSB7476KPNB25`, 47 µF / 10 V / X7R.

Manufacturer documentation confirms nominal ratings and provides class-II MLCC bias/temperature characterization or simulation data. These curves/models are not treated as guaranteed minimum capacitance specifications. The review therefore used deliberately reduced-capacitance stress cases and did not invent a guaranteed `Ceff,min`.

The topology retains the required local input/VCC/output capacitors, the upstream `C5 = 100 µF / 50 V` bulk reservoir and the final Type-III compensation network. No capacitor change is justified at schematic stage. Real load-transient behavior remains a bring-up measurement.

### 4. AutoEN corners — closed

The R1/R2 threshold varies nominally from approximately **1.63 V to 1.99 V** over the 21.6–26.4 V engineering input range.

The corner review included input range, ±1% divider tolerance, conservative resistor TCR and TLV1701 input-error terms. The reviewed COMP trip window is approximately **1.56–2.07 V** and remains well separated from the L7987L high-COMP fault state. The EN-high level also remains comfortably above the L7987L enable threshold at the reviewed low-line/tolerance corner.

The recorded persistent-fault simulation continues to show shutdown/retry and clean eventual restart. Real AutoEN timing/waveforms remain first-board validation.

### Review classification

**Engineering schematic sign-off: PASS.**

This does not mean the hardware is manufacturing-ready. PCB/layout review and physical prototype validation remain mandatory. Detailed calculations, caveats and source hierarchy are maintained in `docs/BUCK_L7987L_DESIGN.md`.

## Footprint audit — closed 2026-09-02

All BOM components now have a deliberate footprint assignment consistent with the selected MPN/package. The non-trivial decisions were:

| Ref | Final MPN | Footprint decision |
|---|---|---|
| C5 | Panasonic `EEEFK1H101P` | Custom `FilamentDryer:CP_Panasonic_F_8x10.2`; Panasonic FK **size F**, Ø8 × 10.2 mm, using the manufacturer land pattern. |
| J2 | GCT `USB4216-03-A` | Custom `FilamentDryer:USB_C_GCT_USB4216-03-A`; the final connector has fully-SMT shell stakes and is not mechanically interchangeable with the previous HRO THT-shell footprint. |
| L1 | Bourns `SRN6045-150M` | Custom `FilamentDryer:L_Bourns_SRN6045` from the Bourns recommended PCB layout. |
| U2 | Silicon Labs `CP2102-GM` | Custom `FilamentDryer:CP2102_GM_QFN28_5x5_P0.5_EP3.25`, using the CP2102/9 recommended land pattern and 3×3 exposed-pad paste stencil. The project uses only pins that are compatible with the classic CP2102; pins 10 and 13–22 are explicitly NC in the netlist. |
| SW1–SW6 | GCT `SWT0110-020010SSA` | Custom `FilamentDryer:SW_GCT_SWT0110`, exact 1.05 × 2.00 mm lands at 4.45 mm pitch from the GCT drawing. |
| BZ1 | Loudity `LD-BZEL-T67-0808` | Custom `FilamentDryer:BUZ_Loudity_SMT67_8.5x8.5`. Loudity specifies the 8.5 × 8.5 × 4 mm body and terminal locations/polarity but does **not** publish a numeric recommended PCB land size. The project therefore uses conservative 2.4 × 2.4 mm lands over the documented corner terminal zones; pin 1 is `+`, pin 2 is `−`, pads 3/4 are NC mechanical terminal zones. |
| U5 | ST `L7987L` | `SamacSys_Parts:SOP65P640X120-17N`; HTSSOP-16 exposed-pad geometry checked against ST, with the 3D model path made project-relative. |

Standard 0603/0805/1210 passives, SOT-23/SOT-23-5/SOT-23-6 devices, DPAK/TO-252 devices, SMA/SMB/SOD-123/CFP3 diodes, JST connectors, ESP32 module and the remaining already-established footprints were checked against their selected package/MPN during the same pass.

### Strict pinout / symbol audit — closed 2026-09-02

A second pass checked selected MPN pin numbering against the KiCad symbol and footprint pad numbering, not only package geometry. It found and corrected three real symbol-level issues:

- `D1 = BZX84C15-7-F`: the project symbol maps **pin 1 = A, pin 2 = NC, pin 3 = K** while retaining the standard KiCad SOT-23 footprint.
- `Q5 = IRLML2060TRPBF`: the final device is **1=G, 2=S, 3=D**, so Q5 uses the matching G-S-D symbol with the standard KiCad SOT-23 footprint.
- `U2 = CP2102-GM`: a dedicated classic CP2102-GM symbol is used; pins **10 and 13–22 are NC**, pin 2 is `~RI` input, and the custom QFN28 footprint remains the Silicon Labs classic CP2102 land pattern.

The remaining semiconductor mappings were checked without finding another pin-numbering mismatch. `U3 = AKS1201` retains the USBLC6-2SC6 topology and standard SOT-23-6 footprint; `Q1/Q4`, the MMBT3904 devices, `U1`, `U4` and `U5` retain their audited mappings.

The following items are deliberately **not** unresolved electrical pinout errors, but still require normal physical/manufacturing review before fabrication:

- `J5`: the real part is a HALJIA XH-compatible connector, so fit/polarization must be confirmed against the actual connector despite the 2.50 mm JST-XH footprint;
- `U5`: HTSSOP-16 copper/pad geometry is verified, but the exposed-pad **stencil/paste aperture strategy** must be reviewed before final paste Gerbers/assembly;
- `U4`: verify final ESP32 antenna keepout and board-edge placement in PCB layout;
- optional OLED footprint: custom and excluded from the BOM; verify mechanically only if installed.

### BZ1 qualification note

BZ1 is the only footprint for which the manufacturer drawing does not provide explicit recommended land dimensions. This is not an unresolved package mismatch: terminal locations, polarity, body envelope and SMD mounting are defined and the footprint has been made deliberately conservative. It should nevertheless receive normal first-board visual solderability inspection.

## DFT / debug decision before placement

The schematic electrical gates are closed, but it remains useful to decide before PCB placement whether to add convenient probe/test pads for:

- `3V3_BUCK` before FB1;
- L7987L COMP;
- L7987L EN / AutoEN control node.

Existing access to `24V_PROT`, `3V3_MCU` and GND is already present. These are DFT recommendations, not electrical sign-off blockers.

## Next manufacturing-preparation sequence

Sourcing, footprint selection and schematic electrical review are no longer blockers. Proceed in this order:

1. decide optional additional buck DFT/debug access before placement;
2. update the PCB from the signed-off schematic, removing the legacy AP66200 stage;
3. place and route the L7987L switching loop according to ST layout guidance;
4. review grounding/return paths, switching-node ringing risk, thermal paths, USB routing, ESP32 antenna keepout, heater/fan high-current paths and U5 exposed-pad paste strategy;
5. run DRC;
6. regenerate BOM, CPL/position data, fabrication outputs and production netlist from the same revision;
7. reconcile generated production data against the final `BOM TME` before ordering;
8. during first-board bring-up, validate output regulation/transient response, component temperatures, switching-node stress, fault current and AutoEN shutdown/recovery timing.

Until PCB synchronization, routing and DRC are complete, the existing PCB/production outputs remain historical and are **not manufacturing-ready**.
