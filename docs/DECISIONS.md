# Engineering decisions

This file records durable project decisions. Do not silently replace them during later work. The actual KiCad sources remain authoritative for the current implementation; if new evidence forces a decision change, document the reason and update this file explicitly.

## D001 — L7987L buck architecture

**Status:** accepted / schematic signed off 2026-09-04.

The 24 V -> 3.3 V converter uses ST L7987L with an external TLV1701 + MMBT3904 AutoEN recovery circuit. The selected L7987L component values, compensation, ILIM and AutoEN thresholds are closed for schematic design. Reopen only if PCB/layout or physical validation provides a concrete reason.

Primary detailed record: `docs/BUCK_L7987L_DESIGN.md`.

## D002 — Four-layer stackup uses two solid GND inner planes

**Status:** accepted for current PCB layout.

- `In1.Cu` = full, continuous GND plane.
- `In2.Cu` = full, continuous GND plane.
- No 3V3 or 24 V distribution zones on either inner layer.
- Power distribution remains on the external layers with local copper/tracks as required.

Rationale: both outer routing layers receive a nearby continuous reference plane; the buck on B.Cu has In2 only 0.10 mm away. This also avoids return-current detours caused by split reference planes.

## D003 — L7987L PGND/SGND are current-path regions, not separate project nets

**Status:** accepted.

ST's power-ground / signal-ground guidance is implemented without splitting the internal GND planes and without creating separate electrical PGND/SGND nets.

Both are the same KiCad `GND` net. The distinction is made through B.Cu copper geometry and via placement:

- pulsed/high-current return: C1 negative, D7 anode, C10 negative;
- quiet/signal return: U5 pin 16 and exposed pad, C3 negative, C21 negative and sensitive control returns.

These paths use short local connections to the common solid GND planes. The high-current return geometry must not force switching current through the quiet feedback/compensation return region.

**Implementation clarification:** a continuous B.Cu PGND strip physically joining C1− to D7/C10− is not a project requirement. The high-current pads may enter the same solid inner GND planes through short nearby GND vias, provided the via placement and local copper keep the pulsed return current out of the quiet local return region. Do not move C21 merely to create a superficial PGND corridor across U5.

## D004 — Buck is implemented on B.Cu

**Status:** accepted/current implementation.

U5 and the new buck/AutoEN block are on B.Cu. In2 is therefore their nearest continuous GND reference plane. Local buck power copper/zones are also developed on B.Cu.

## D005 — ST reference layout controls buck placement topology

**Status:** accepted.

Use the ST L7987L datasheet and STEVAL-ISA198V1 layout/Gerbers as the primary layout reference, adapted to the project's four-layer stackup and actual footprints.

Current placement priorities:

- C3 = 1 µF local VIN/VCC bypass, closest practical connection to VIN/VCC;
- C1 = 10 µF main input ceramic, on the same VIN side and may be slightly farther out than C3;
- C6 = 100 nF BOOT-LX capacitor, very short BOOT/LX connection;
- D7/L1/LX copper form the switching region;
- C10 is the main pre-bead output capacitor;
- FB/COMP network remains compact and away from the switching/high-current region;
- C21 remains close to VBIAS.

In the current physical board view, the U5 VIN/VCC side is the **left side of the chip**. Do not reverse this based on top/bottom mirroring arguments.

## D006 — LX is controlled geometrically, not by a generic wide netclass

**Status:** accepted.

Do not create a generic wide `LX` netclass simply because it is a power node. Keep the switch-node copper short and controlled, with only the area needed to connect U5 LX, D7 cathode, C6 bootstrap return and L1 input. Minimize unnecessary switch-node area.

## D007 — Current power/default netclasses accepted; USB differential geometry remains open

**Status:** power/default classes accepted; USB DP geometry pending.

Accepted classes:

| Class | Clearance | Track | Via dia/drill |
|---|---:|---:|---:|
| Default | 0.20 mm | 0.25 mm | 0.60/0.30 mm |
| Power_3V3 | 0.20 mm | 0.50 mm | 0.60/0.30 mm |
| Power_24V | 0.20 mm | 1.00 mm | 0.80/0.40 mm |
| Power_Heat | 0.30 mm | 1.50 mm | 0.80/0.40 mm |
| USB | 0.20 mm | 0.25 mm | 0.60/0.30 mm |

USB `USB_DP`/`USB_DM` are assigned to the USB class, but USB differential-pair width/gap must be calculated/verified against the actual stackup before routing is frozen. Do not simply inherit the Default-class DP values.

## D008 — Bottom-side orientation checks must use transformed/absolute geometry

**Status:** process rule.

Never recommend a B.Cu component rotation based only on raw local pad coordinates from the `.kicad_pcb` text. KiCad's bottom-side transform must be accounted for.

Use one of:

- KiCad/pcbnew absolute global pad positions;
- the actual board view with pad numbers/nets visible;
- another KiCad-derived read-only report that applies the correct transform.

This rule was added after a previous manual-coordinate review produced incorrect rotation recommendations.

## D009 — Hardware Design Manual is a read-only review framework

**Status:** accepted.

The external Hardware Design Manual is used as a source-agnostic engineering checklist/reference. Do not modify it during normal Filament Dryer repository work. Device-specific manufacturer documentation and standards take precedence for device-specific requirements.

## D010 — Production release requires fresh DRC and regenerated outputs

**Status:** accepted.

The historical `hardware/DRC.rpt` is not a release gate for the current PCB. No fabrication release until the current routed PCB has a fresh reviewed DRC and production Gerbers/BOM/CPL/netlist are regenerated from that same revision and reconciled with the purchasing BOM.

## D011 — Existing schematic review remains closed during PCB work

**Status:** accepted.

PCB placement/routing refinements that do not change electrical topology/component values do not reopen schematic sign-off. If layout forces an electrical or component change, explicitly reopen the relevant schematic-review gate, update the schematic/netlist first, then resynchronize the PCB and documentation.

## D012 — C22 added as second 1 µF bypass for U5 VIN/VCC

**Status:** accepted.

The L7987L datasheet requires a 1 µF or larger ceramic local to both the VIN pins and the
VCC pin (sections 5.1 and 5.6); the ST demo board implements this as two separate 1 µF
parts. The design previously carried a single C3 because `24V_PROT` feeds VIN1, VIN2 and
VCC on one net. C22 (`GRJ21BC72A105KE11L`, 1 µF 100 V X7S 0805, identical to C3) was added
2026-09-09.

Electrically the two are in parallel on `24V_PROT`; the requirement is physical. C3 stays
as the VCC bypass at pin 4, C22 becomes the VIN bypass at pins 2/3. Their functional names
(`C_vcc_buck1`, `C_vin_byp1`) carry that placement intent and must not be made identical.

The 25 V `GCM188R71E105KA64J` used for C15/C17/C21 is not an acceptable substitute here —
it has no voltage margin on a 24 V rail.

## D013 — AutoEN is required; validated failure mechanism is foldback lock

**Status:** accepted. Supersedes the rationale recorded in `BUCK_L7987L_DESIGN.md` section 11.

A datasheet-only review argued that the L7987L handles a persistent output fault natively
via pulse-by-pulse OCP, pulse skipping and FB foldback, and that AutoEN might therefore be
redundant complexity protecting against a simulation artifact.

Simulation on 2026-09-09 disproved that argument. The device protections are present and
work as documented, but recovery does not occur: foldback holds the output at ~1.5 V, which
keeps FB at ~0.36 V, below the 400 mV release threshold, indefinitely. Full detail in
`BUCK_L7987L_DESIGN.md` section 11.1.

AutoEN stays. Two of the three objections raised during that review were withdrawn on
evidence; the third — failure direction of U1 — is addressed by assembly order (D014).

Process note: the existing simulation evidence was the strongest available and should have
been examined before a datasheet argument was built against a closed decision.

## D014 — R5 unpopulated for initial bring-up

**Status:** accepted.

R5 (100 kΩ, TLV1701 open-collector pull-up) is not fitted at first assembly. A missing or
badly soldered U1 otherwise holds EN low through R5/Q7 and the board produces no 3.3 V rail,
which at first power-up is indistinguishable from a buck failure.

With R5 absent, R30 holds the Q7 base at ground and EN is free. R5 is fitted after the
regulator is confirmed working. No schematic or netlist change; the schematic carries a
`DNP at first assembly` note on R5 and the procedure lives in `docs/ASSEMBLY.md`.
