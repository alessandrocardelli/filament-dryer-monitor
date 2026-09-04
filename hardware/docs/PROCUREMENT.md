# Procurement and footprint state

Status: **2026-09-04 — TME sourcing, KiCad MPN/manufacturer synchronization and footprint audit complete; schematic review closure pending** on branch `redesign/buck-sourcing`.

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
- `R33 = 16 kΩ`, `R34 = 16 kΩ`, `R35 = 1.13 kΩ 0.1%`, `R36 = 49.9 kΩ`;
- the capacitor values/dielectrics and resistor tolerances in the schematic/BOM are the sourced values selected during this pass.

Sourcing and footprint selection are therefore **closed** for the present schematic. They should only be reopened if an electrical review item forces a component change.

## 2026-09-04 schematic-review checkpoint

A schematic review using the project Hardware Design Manual as the review framework found no blocking omission in the implemented L7987L + AutoEN topology. This does **not** yet constitute final schematic sign-off.

The following gates remain open before PCB synchronization:

1. **Fresh ERC** — the committed `hardware/ERC.rpt` is dated 2026-08-09 and predates the L7987L redesign, so it is stale for the current schematic.
2. **ILIM/L1 worst case** — the nominal programmed L7987L peak limit is approximately 1.705 A, while normal-operation peak inductor current at the 1 A design load is approximately 1.18–1.19 A. The maximum current-limit threshold over IC tolerance must still be compared explicitly with the SRN6045-150M saturation behavior and relevant fault stresses.
3. **Effective capacitance** — final sourced input/output MLCCs must be checked using manufacturer capacitance-versus-DC-bias/temperature data so the effective values remain compatible with the L7987L input/output, startup and loop-compensation requirements.
4. **AutoEN corners** — the R1/R2 threshold is derived from `24V_PROT` and therefore varies nominally from approximately 1.63 V to 1.99 V over the project 21.6–26.4 V input range. Trip/recovery behavior must be checked across VIN, component tolerance and temperature.
5. **DFT/debug access** — before placement, decide whether to add convenient probe/test pads for `3V3_BUCK`, L7987L COMP and EN/AutoEN. Existing access to `24V_PROT`, `3V3_MCU` and GND is already present.

Detailed calculations and the review record are maintained in `docs/BUCK_L7987L_DESIGN.md`.

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

- `D1 = BZX84C15-7-F`: the generic two-pin Zener symbol could not represent the SOT-23 device correctly. The project symbol now maps **pin 1 = A, pin 2 = NC, pin 3 = K** while retaining the standard KiCad SOT-23 footprint.
- `Q5 = IRLML2060TRPBF`: the previous generic MOSFET symbol used G-D-S numbering. The final device is **1=G, 2=S, 3=D**, so Q5 now uses the matching G-S-D symbol with the standard KiCad SOT-23 footprint.
- `U2 = CP2102-GM`: the schematic had inherited a CP2102N symbol. A dedicated classic CP2102-GM symbol is now used; pins **10 and 13–22 are NC**, pin 2 is `~RI` input, and the custom QFN28 footprint remains the Silicon Labs classic CP2102 land pattern.

The remaining semiconductor mappings were checked without finding another pin-numbering mismatch. `U3 = AKS1201` retains the USBLC6-2SC6 topology and standard SOT-23-6 footprint; `Q1/Q4`, the MMBT3904 devices, `U1`, `U4` and `U5` retain their audited mappings.

The following items are deliberately **not** treated as unresolved electrical pinout errors, but still require normal physical/manufacturing review before fabrication:

- `J5`: the real part is a HALJIA XH-compatible connector, so fit/polarization must be confirmed against the actual connector despite the 2.50 mm JST-XH footprint.
- `U5`: HTSSOP-16 copper/pad geometry is verified, but the exposed-pad **stencil/paste aperture strategy** must be reviewed before final paste Gerbers/assembly.
- `U4`: verify final ESP32 antenna keepout and board-edge placement in PCB layout.
- optional OLED footprint: it is custom and excluded from the BOM; verify mechanically only if the display is installed.

### BZ1 qualification note

BZ1 is the only footprint for which the manufacturer drawing does not provide explicit recommended land dimensions. This is not an unresolved package mismatch: terminal locations, polarity, body envelope and SMD mounting are defined and the footprint has been made deliberately conservative. It should nevertheless receive the normal first-board visual solderability check, like any custom land pattern.

## Next manufacturing-preparation sequence

Sourcing and footprint selection are no longer blockers. Proceed in this order:

1. close the remaining schematic-review gates listed above;
2. decide the additional buck DFT/debug access before placement;
3. run a fresh ERC on the exact current schematic and resolve or explicitly justify every item;
4. update the PCB from that signed-off schematic, removing the legacy AP66200 stage;
5. place and route the L7987L switching loop according to ST layout guidance;
6. review grounding/return paths, thermal paths, USB routing, ESP32 antenna keepout, heater/fan high-current paths and U5 exposed-pad paste strategy;
7. run DRC;
8. regenerate BOM, CPL/position data, fabrication outputs and production netlist from the same revision;
9. reconcile generated production data against the final `BOM TME` before ordering.

Until schematic closure, PCB synchronization, routing and DRC are complete, the existing PCB/production outputs remain historical and are **not manufacturing-ready**.
