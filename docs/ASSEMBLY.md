# Assembly and bring-up order

Hand assembly with hot air and a fine-tip iron. This document covers only the steps where
assembly order carries engineering meaning. Ordinary reflow/soldering sequence is not
prescribed here.

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
