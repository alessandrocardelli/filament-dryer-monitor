# Assembly and bring-up order

Hand assembly with hot air and a fine-tip iron. This document covers only the steps where
assembly order carries engineering meaning. Ordinary reflow/soldering sequence is not
prescribed here.

## Before assembly / fabrication release

Do not use an in-progress PCB revision for assembly merely because routing is complete or DRC has no errors.

For the current 2026-09-16 hardware checkpoint, fabrication release still requires the gates in docs/PROJECT_STATE.md to be closed, especially:

- regenerate netlist/ERC after the latest schematic edit;
- review the remaining DRC warnings;
- regenerate final fabrication/assembly outputs from the same released revision.

The U5 exposed-pad implementation is already closed for the planned hand-assembly process; see the section below and decision D019.

Once a fabrication revision is released, the R5 sequence below remains mandatory for first power-up.

## U5 exposed pad — hand-assembly implementation

U5 pad 17 is the 3.2 × 3.2 mm GND exposed pad on B.Cu. The board intentionally does **not** use via-in-pad. Instead, six GND vias (0.60 mm diameter / 0.30 mm drill) sit immediately outside the exposed pad in two rows of three, one above and one below U5. They provide the short thermal/GND path into the internal GND planes while avoiding solder wicking through open vias under the pad.

This arrangement was implemented in commit `c9dc4129` and remains present in the current PCB. For the planned prototype assembly, U5 is fitted by lightly pre-tinning the exposed pad, applying flux, placing the device and heating with hot air. Because no stencil is used for this operation, the full B.Paste definition on pad 17 is not a fabrication blocker for the prototype.

If the assembly process later changes to stencil/reflow or external PCBA, reopen the paste-aperture/via-treatment decision for that process. First-board thermal validation of U5 remains required, but it is a validation item, not unfinished PCB layout.

## R5 — do not fit at first assembly

**Rule: populate the whole board except R5. Fit R5 only after the 3.3 V rail is confirmed.**

Reason, in short: U1 (TLV1701) has an open-collector output. If U1 is missing, damaged, or
has one unsoldered pin, R5 pulls its output node up, Q7 conducts, and EN is held low. The
buck never starts and the board shows no 3.3 V — which looks exactly like a dead regulator.

U1 is a hand-soldered SOT-23-5. A single cold joint is a realistic first-power-up outcome,
and without this precaution it sends debugging effort to U5, L1 and the feedback network
while the regulator is actually healthy.

With R5 absent, R30 holds the Q7 base at ground, Q7 stays off, EN is free, and the buck runs
normally without AutoEN protection.

Background: `docs/BUCK_L7987L_DESIGN.md` section 11.3, decision D014.

### Bring-up sequence

1. Assemble the board complete **except R5**.
2. Power up. Verify the buck produces 3.3 V.
   - No 3.3 V here means the fault is in the buck itself. Debug U5, L1, D7, C10, feedback.
     AutoEN is not involved and cannot be the cause.
3. Fit R5. Power up again and verify 3.3 V is still present.
   - If the rail disappears at this step, the fault is U1 or its solder joints — not the
     regulator. Reflow or replace U1.
4. Only then proceed to fault-injection testing of AutoEN.

Step 3 is the point of the whole procedure: it separates "the buck does not work" from
"AutoEN is shutting down a working buck".

## Off-board items

The thermal cutoff (~100–110 °C) is off-board, in series on the HEATER+ wire. It is not a
PCB component and must not be omitted when wiring the assembly into the enclosure.
