from pathlib import Path


def replace_once(text, old, new, label):
    if old not in text:
        raise RuntimeError(f'Missing documentation anchor: {label}')
    return text.replace(old, new, 1)

# README: make the project summary match the final sourced parts and the fact
# that the checked-in exported netlist predates the strict pinout corrections.
path = Path('README.md')
text = path.read_text(encoding='utf-8')
text = replace_once(
    text,
    '''> **Status: work in progress.** On branch `redesign/buck-sourcing` the schematic and exported\n> netlist implement the L7987L 24 V -> 3.3 V buck redesign, including the external AutoEN\n> fault-recovery circuit. The latest whole-schematic electrical review found no blocking\n> omission, and the schematic is now treated as electrically frozen for the sourcing/footprint\n> pass. The PCB is **not yet synchronized** and still contains the legacy AP66200 power stage,\n> so the current PCB/production outputs are not manufacturing-ready. Most firmware work is also\n> still in development.\n''',
    '''> **Status: work in progress.** On branch `redesign/buck-sourcing` the schematic implements the\n> final sourced L7987L 24 V -> 3.3 V redesign, including the external AutoEN fault-recovery\n> circuit. TME sourcing, MPN/manufacturer metadata, footprint selection and the strict\n> MPN-to-symbol pinout audit are complete. The checked-in exported netlist predates the final\n> D1/Q5/U2 symbol-pin corrections and must be regenerated from the current schematic before PCB\n> synchronization. The PCB is **not yet synchronized** and still contains the legacy AP66200\n> power stage, so the current PCB/production outputs are not manufacturing-ready.\n''',
    'README status',
)
text = replace_once(
    text,
    '''The current KiCad schematic and freshly exported netlist on the active hardware branch are the\nauthoritative electrical state; the PCB is currently one step behind the schematic because the\nnew L7987L power stage has not yet been placed/routed.\n''',
    '''The current KiCad schematic on the active hardware branch is the authoritative electrical state.\nThe checked-in exported netlist must be regenerated after the final strict pinout corrections;\nthe PCB is currently one step behind the schematic because the new L7987L power stage has not\nyet been placed/routed.\n''',
    'README hardware source of truth',
)
text = text.replace('| USB-UART | CP2102N | USB used for programming/debug only, with DTR/RTS auto-program circuit |',
                    '| USB-UART | **CP2102-GM** | Classic Silicon Labs CP2102, USB used for programming/debug only, with DTR/RTS auto-program circuit |')
text = text.replace('| Heater driver | IRLR3636TRPBF (DPAK) | 60 V logic-level N-MOSFET, ~1.6 A heater load, LCSC C67279 |',
                    '| Heater driver | IRLR3636TRPBF (DPAK) | 60 V logic-level N-MOSFET, ~1.6 A heater load |')
text = text.replace('| Fan driver | CJ2310 (SOT-23) | 24 V fan, ~0.2 A |',
                    '| Fan driver | **IRLML2060TRPBF** (SOT-23) | 24 V fan, ~0.2 A; final symbol uses physical 1=G, 2=S, 3=D pinout |')
text = replace_once(
    text,
    '''The working purchasing BOM is the Google Sheet `Filament Dryer Monitor — BOM finale Mouser`,\ntab **`BOM TME`**. It remains the working source for supplier codes, stock and purchasing\nchoices, but it has **not yet been reconciled to the newly integrated L7987L block**.\n\nThe next hardware task is now the **TME sourcing / final MPN / footprint pass before PCB\nsynchronization**. Existing MPN fields in the redesigned buck block are provisional until each\npart is deliberately checked against availability, package, pinout and footprint.\n\nMetadata cleanup such as the copied `Function` field on the new Q6 pull-down resistor can be\nperformed together in KiCad Symbol Fields Editor once sourcing decisions are complete.\n''',
    '''The working purchasing BOM is the Google Sheet `Filament Dryer Monitor — BOM finale Mouser`,\ntab **`BOM TME`**. It is the purchasing source for final MPNs, TME order codes and sourcing\nchoices. The current schematic has been synchronized to those selected MPNs/manufacturers.\n\nThe sourcing/footprint pass and strict MPN -> symbol pinout audit are complete. The strict pass\ncorrected `D1 = BZX84C15-7-F`, `Q5 = IRLML2060TRPBF` and the classic `U2 = CP2102-GM` symbol\nmappings. The next hardware step is to run a fresh ERC/regenerate the netlist from this\nschematic revision, then update the PCB from schematic and perform the layout/DRC pass.\n''',
    'README procurement status',
)
text = replace_once(
    text,
    '''3. Complete the TME sourcing / final MPN / footprint pass against the electrically frozen\n   schematic before transferring the redesigned block to PCB.\n4. Update KiCad symbol fields and approved footprints, then run a fresh ERC from that revision.\n5. Update PCB from schematic, replace/place/route the L7987L stage, and perform a dedicated\n   PCB/layout review.\n6. Run DRC and regenerate BOM, position files, netlist and fabrication outputs from that same\n   revision before ordering boards.\n7. Reconcile the generated BOM against the final `BOM TME` sheet before purchasing/production.\n''',
    '''3. Pull the current `redesign/buck-sourcing` branch so the final custom symbols/footprints are\n   present locally, then run a fresh ERC and regenerate the exported netlist.\n4. Update PCB from schematic, replace/place/route the L7987L stage, and perform a dedicated\n   PCB/layout review including the ESP32 antenna keepout and U5 exposed-pad paste strategy.\n5. Confirm the physical fit/polarization of the in-house HALJIA J5 connector against its assigned\n   JST-XH-compatible footprint.\n6. Run DRC and regenerate BOM, position files, netlist and fabrication outputs from that same\n   revision before ordering boards.\n7. Reconcile the generated BOM against the final `BOM TME` sheet before purchasing/production.\n''',
    'README build sequence',
)
path.write_text(text, encoding='utf-8')

# Buck design record: clarify that the schematic is current but the checked-in
# exported netlist must be regenerated after cross-project pinout corrections.
path = Path('docs/BUCK_L7987L_DESIGN.md')
text = path.read_text(encoding='utf-8')
text = text.replace('Status: **schematic/netlist implementation complete; PCB integration pending**.',
                    'Status: **schematic implementation complete; exported netlist regeneration and PCB integration pending**.')
text = replace_once(
    text,
    '''Sourcing and footprint audit closed on **2026-09-02** on this branch; the actual schematic/netlist at branch HEAD remain authoritative.\n\nThe actual KiCad schematic and exported netlist override this document if they ever disagree.\n''',
    '''Sourcing, footprint and strict component-pinout audit closed on **2026-09-02** on this branch.\nThe actual KiCad schematic at branch HEAD is authoritative. The checked-in exported netlist\npredates the final cross-project D1/Q5/U2 pinout corrections and must be regenerated before PCB\nsynchronization.\n\nThe actual KiCad schematic overrides this document if they ever disagree.\n''',
    'buck source of truth',
)
path.write_text(text, encoding='utf-8')

# Procurement record source-of-truth wording must make the same distinction.
path = Path('hardware/docs/PROCUREMENT.md')
text = path.read_text(encoding='utf-8')
text = replace_once(
    text,
    '''The current KiCad schematic and freshly exported netlist are authoritative for electrical connectivity. The `BOM TME` Google Sheet is the purchasing source for selected manufacturer MPNs and TME order codes. The schematic/netlist override documentation if they disagree.\n''',
    '''The current KiCad schematic is authoritative for electrical connectivity. The `BOM TME` Google Sheet is the purchasing source for selected manufacturer MPNs and TME order codes. The checked-in exported netlist predates the final D1/Q5/U2 strict-pinout corrections and must be regenerated before PCB synchronization. The schematic overrides documentation or stale exports if they disagree.\n''',
    'procurement source of truth',
)
path.write_text(text, encoding='utf-8')

print('Post-audit documentation sync: PASS')
