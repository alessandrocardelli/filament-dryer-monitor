# Procurement and footprint state

Status: **2026-09-02 — TME sourcing, KiCad MPN/manufacturer synchronization and footprint audit complete** on branch `redesign/buck-sourcing`.

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

### BZ1 qualification note

BZ1 is the only footprint for which the manufacturer drawing does not provide explicit recommended land dimensions. This is not an unresolved package mismatch: terminal locations, polarity, body envelope and SMD mounting are defined and the footprint has been made deliberately conservative. It should nevertheless receive the normal first-board visual solderability check, like any custom land pattern.

## Next manufacturing-preparation step

Sourcing and footprint selection are no longer blockers. Proceed with:

1. run a fresh ERC on the current schematic;
2. update the PCB from the current schematic, removing the legacy AP66200 stage;
3. place and route the L7987L switching loop according to ST layout guidance;
4. review grounding, thermal paths, USB routing, ESP32 antenna keepout and heater/fan high-current paths;
5. run DRC;
6. regenerate BOM, CPL/position data, fabrication outputs and production netlist;
7. reconcile generated production data against the final `BOM TME` before ordering.

Until PCB synchronization, routing and DRC are complete, the existing PCB/production outputs remain historical and are **not manufacturing-ready**.
