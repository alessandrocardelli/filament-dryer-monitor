# Procurement and TME sourcing

Status: 2026-08-21

## Source-of-truth rules

The KiCad schematic, PCB and netlist on `main` remain the authoritative source for the
implemented electrical and mechanical design.

The working purchasing BOM is maintained in the Google Sheet:

- `Filament Dryer Monitor — BOM finale Mouser`
- tab: `BOM TME`
- https://docs.google.com/spreadsheets/d/14pd4d5PjS7goH_W73SR9czaZ9BhpDt5480ON451nyR0

The sheet is the working source for supplier codes, stock, prices and purchasing choices.
Stock, price and delivery dates are transient data and must not be treated as permanent design
facts. A component selected in the sheet does not become part of the hardware implementation
until the corresponding KiCad fields, footprint and PCB have been reviewed and updated.

## Sourcing strategy

TME is now the preferred supplier for the prototype BOM. The goal is to use parts with good,
repeatable availability rather than forcing an exact historical JLCPCB/LCSC or Mouser part
when an electrically and mechanically suitable TME alternative is better stocked.

Using another supplier is acceptable when an exact mechanical part is required or when a
sensible TME substitution would require unnecessary redesign. The final order therefore does
not have to be 100% TME.

The `BOM TME` sheet is API-assisted: TME stock, unit prices, delivery information and
available datasheet links are refreshed automatically. The purchasing decision must still be
reviewed against the actual design before ordering.

## Current procurement status

As of 2026-08-21 almost all BOM lines have an immediately available TME selection.

### Resolved during TME conversion

- **R6** — feedback resistor corrected to Yageo `RC0603FR-0731K6L`, 31.6 kΩ, 1%, 0603.
  This is the correct TME-available `-07` part; the earlier `RC0603FR-1331K6L` choice was not
  the useful TME ordering code.
- **C5** — Panasonic `EEEFK1H101P`, 100 µF / 50 V SMD electrolytic, selected as the available
  TME replacement. **PCB footprint must be verified before applying the change.**
- **Q5** — Infineon `IRLML2060TRPBF`, 60 V SOT-23 N-MOSFET, selected for the fan switch.
  Pinout/footprint must be verified when the KiCad design is updated.
- **U3** — Akyga Semi `AKS1201`, TME symbol `USBLC6-2SC6-AKS`, selected as the available
  two-line USB ESD protector. The current `main` design still uses ST `USBLC6-2SC6`; pinout,
  line capacitance and footprint must be confirmed before replacement.

The Google Sheet contains the complete current purchasing selection; do not duplicate the
entire live BOM here because stock and supplier choices can change.

## Known design-impact substitutions

These purchasing choices differ from the current `main` implementation or require an explicit
mechanical/electrical check before being copied into KiCad:

| Ref | Current `main` design | TME purchasing choice | Required action |
|---|---|---|---|
| C5 | 100 µF / 50 V SMD electrolytic, current 8 mm-class footprint | Panasonic `EEEFK1H101P` | Verify land pattern/body clearance |
| J2 | HRO Type-C connector footprint | GCT `USB4216-03-A` | Replace/verify footprint, pin mapping and 3D model |
| L1 | `ZD0650-8R2M` | Eaton `HCM0703-8R2-R` | Verify electrical ratings and footprint before PCB update |
| Q1 | `DMP6180SK3-13` | onsemi `NTD20P06LT4G` | Verify package/pinout and update KiCad fields |
| Q5 | `CJ2310` | Infineon `IRLML2060TRPBF` | Verify SOT-23 pinout and update KiCad fields |
| U2 | CP2102N family | Silicon Labs `CP2102-GM` | Re-check pin/package compatibility for this circuit before applying |
| U3 | ST `USBLC6-2SC6` | Akyga Semi `AKS1201` | Verify pinout, ESD characteristics and line capacitance |

Other BOM substitutions may also require footprint review. The `BOM TME` sheet notes should
be checked line by line when the TME-oriented hardware branch is created.

## Remaining sourcing exceptions

### J7 — JST GH NTC connector

Current part: `BM02B-GHS-TBT`.

The design uses the exact vertical/top-entry JST GH footprint. TME currently reports zero local
stock and an expected replenishment date of 2026-10-09. Do not substitute a side-entry GH part
just to satisfy TME availability. If the exact part is not available when ordering, buy it from
another supplier rather than changing the PCB without a mechanical reason.

### U1 — AP66200 buck converter

Current design: Diodes Incorporated `AP66200FVBW-13`, 60 V / 2 A synchronous buck converting
24 V to 3.3 V.

TME currently reports zero stock and an expected replenishment date of 2026-11-27. Availability
at other distributors has also been inconsistent, so U1 is now considered a sourcing risk.

A redesign around a more broadly stocked high-voltage buck is being evaluated before the first
prototype order. ST `L7987L` is a current candidate because it supports 61 V input / 2 A and has
substantially broader distributor stock, but **no replacement has been approved yet**. It is not
a drop-in replacement and would require a new regulator block/PCB review.

Until that redesign is explicitly approved and implemented, `AP66200FVBW-13` remains the
actual design part.

## Next hardware branch

A TME-oriented hardware branch should start from the documented `main` state. For every
procurement substitution that changes the implemented part:

1. verify the manufacturer datasheet and electrical requirements;
2. verify pinout, land pattern, body clearance and 3D model where relevant;
3. update schematic fields and PCB footprint deliberately;
4. rerun ERC/DRC;
5. regenerate production BOM, positions and netlist from the resulting KiCad revision;
6. compare the generated BOM against the purchasing sheet before ordering.
