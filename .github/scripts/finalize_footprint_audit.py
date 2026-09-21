from pathlib import Path
import re

ROOT = Path('hardware')
LIB = ROOT / 'libs' / 'FilamentDryer.pretty'
LIB.mkdir(parents=True, exist_ok=True)

TARGETS = {
    'BZ1': 'FilamentDryer:BUZ_Loudity_SMT67_8.5x8.5',
    'U2': 'FilamentDryer:CP2102_GM_QFN28_5x5_P0.5_EP3.25',
    'SW1': 'FilamentDryer:SW_GCT_SWT0110',
    'SW2': 'FilamentDryer:SW_GCT_SWT0110',
    'SW3': 'FilamentDryer:SW_GCT_SWT0110',
    'SW4': 'FilamentDryer:SW_GCT_SWT0110',
    'SW5': 'FilamentDryer:SW_GCT_SWT0110',
    'SW6': 'FilamentDryer:SW_GCT_SWT0110',
}

EXPECTED_OLD = {
    'BZ1': 'JLCPCB:BUZ-SMD_4P-L8.5-W8.5-P8.50-BR',
    'U2': 'FilamentDryer:CP2102N_GQFN28_5x5mm_P0.5mm_EP3.35x3.35mm',
    'SW1': 'JLCPCB:SW-SMD_L3.9-W3.0-P4.45',
    'SW2': 'JLCPCB:SW-SMD_L3.9-W3.0-P4.45',
    'SW3': 'JLCPCB:SW-SMD_L3.9-W3.0-P4.45',
    'SW4': 'JLCPCB:SW-SMD_L3.9-W3.0-P4.45',
    'SW5': 'JLCPCB:SW-SMD_L3.9-W3.0-P4.45',
    'SW6': 'JLCPCB:SW-SMD_L3.9-W3.0-P4.45',
}

VALUE_UPDATES = {
    'BZ1': 'LD-BZEL-T67-0808',
    'U2': 'CP2102-GM',
}


def balanced_end(text, start):
    depth = 0
    in_string = False
    escape = False
    for i in range(start, len(text)):
        ch = text[i]
        if in_string:
            if escape:
                escape = False
            elif ch == '\\':
                escape = True
            elif ch == '"':
                in_string = False
        else:
            if ch == '"':
                in_string = True
            elif ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
                if depth == 0:
                    return i + 1
    raise ValueError('Unbalanced S-expression')


def symbol_spans(text):
    for match in re.finditer(r'\(symbol(?:\s|\")', text):
        start = match.start()
        end = balanced_end(text, start)
        block = text[start:end]
        mref = re.search(r'\(property\s+"Reference"\s+"([A-Za-z]+\d+)"', block)
        if mref:
            yield start, end, mref.group(1), block


def prop(block, name):
    m = re.search(r'\(property\s+"' + re.escape(name) + r'"\s+"((?:\\.|[^"])*)"', block)
    return None if not m else m.group(1)


def replace_prop(block, name, value):
    pat = re.compile(r'(\(property\s+"' + re.escape(name) + r'"\s+")((?:\\.|[^"])*)"')
    out, n = pat.subn(lambda m: m.group(1) + value + '"', block, count=1)
    if n != 1:
        raise SystemExit(f'Cannot update {name}')
    return out


# Safety checks against the exported authoritative netlist.
net = (ROOT / 'Filament_Dryer_Monitor.net').read_text(encoding='utf-8')

def node_pins(ref):
    out = {}
    pat = re.compile(
        r'\(node\s+\(ref\s+"' + re.escape(ref) + r'"\)\s+'
        r'\(pin\s+"([^"]+)"\)\s+'
        r'\(pinfunction\s+"([^"]*)"\)\s+'
        r'\(pintype\s+"([^"]*)"\)'
    )
    for m in pat.finditer(net):
        out[m.group(1)] = (m.group(2), m.group(3))
    return out

bz = node_pins('BZ1')
if set(bz) != {'1', '2', '3', '4'}:
    raise SystemExit(f'BZ1 unexpected pins: {bz}')
if 'no_connect' not in bz['3'][1] or 'no_connect' not in bz['4'][1]:
    raise SystemExit(f'BZ1 pins 3/4 are not NC: {bz}')
if 'no_connect' in bz['1'][1] or 'no_connect' in bz['2'][1]:
    raise SystemExit(f'BZ1 electrical pins 1/2 unexpectedly NC: {bz}')

u2 = node_pins('U2')
# CP2102-GM: pins 10 and 13..22 are NC. The CP2102N symbol has functions on
# some of these pins, so every one must remain explicitly unconnected here.
for pin in ['10'] + [str(i) for i in range(13, 23)]:
    if pin not in u2 or 'no_connect' not in u2[pin][1]:
        raise SystemExit(f'U2 pin {pin} must be NC for CP2102-GM compatibility: {u2.get(pin)}')
# Project-used CP2102 pins must retain the classic CP2102 functions/pin numbers.
for pin in ['3', '4', '5', '6', '7', '8', '24', '25', '26', '28', '29']:
    if pin not in u2 or 'no_connect' in u2[pin][1]:
        raise SystemExit(f'U2 required pin {pin} is not connected as expected: {u2.get(pin)}')

# Update only the audited schematic instances.
seen = set()
for path in sorted(ROOT.glob('*.kicad_sch')):
    text = path.read_text(encoding='utf-8')
    replacements = []
    for start, end, ref, block in symbol_spans(text):
        if ref not in TARGETS:
            continue
        seen.add(ref)
        old_fp = prop(block, 'Footprint')
        if old_fp not in (EXPECTED_OLD[ref], TARGETS[ref]):
            raise SystemExit(f'{ref}: unexpected footprint {old_fp!r} in {path}')
        new = block
        if old_fp != TARGETS[ref]:
            new = replace_prop(new, 'Footprint', TARGETS[ref])
        if ref in VALUE_UPDATES and prop(new, 'Value') != VALUE_UPDATES[ref]:
            new = replace_prop(new, 'Value', VALUE_UPDATES[ref])
        if new != block:
            replacements.append((start, end, new))
    for start, end, block in sorted(replacements, reverse=True):
        text = text[:start] + block + text[end:]
    path.write_text(text, encoding='utf-8')

missing = set(TARGETS) - seen
if missing:
    raise SystemExit(f'Missing audited refs: {sorted(missing)}')

# -----------------------------------------------------------------------------
# BZ1 — Loudity LD-BZEL-T67-0808 / SMT67
# Manufacturer drawing establishes an 8.5 x 8.5 x 4 mm body, four corner
# terminal zones, with + at upper-left and - at lower-left. It does NOT publish
# a numeric recommended PCB land size. We therefore retain the conservative
# 2.4 x 2.4 mm land size from the previous board footprint, but correct the pad
# mapping and make the footprint specific to this selected part. Pads 3/4 are
# solderable mechanical/NC terminal zones, matching the symbol's explicit NCs.
# -----------------------------------------------------------------------------
(LIB / 'BUZ_Loudity_SMT67_8.5x8.5.kicad_mod').write_text(r'''(footprint "BUZ_Loudity_SMT67_8.5x8.5"
  (version 20260206)
  (generator "pcbnew")
  (generator_version "10.0")
  (layer "F.Cu")
  (descr "LOUDITY LD-BZEL-T67-0808 / SMT67; 8.5x8.5x4 mm. Terminal locations from manufacturer drawing; conservative 2.4x2.4 mm lands because no recommended PCB land dimensions are published.")
  (tags "buzzer Loudity SMT67 LD-BZEL-T67-0808")
  (property "Reference" "REF**" (at 0 -5.5 0) (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Value" "LD-BZEL-T67-0808" (at 0 5.5 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (attr smd)
  (fp_poly
    (pts (xy -3.55 -4.25) (xy 3.55 -4.25) (xy 4.25 -3.55) (xy 4.25 3.55) (xy 3.55 4.25) (xy -3.55 4.25) (xy -4.25 3.55) (xy -4.25 -3.55))
    (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_line (start -3.4 -4.4) (end 3.4 -4.4) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start 4.4 -3.4) (end 4.4 3.4) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start 3.4 4.4) (end -3.4 4.4) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start -4.4 3.4) (end -4.4 -3.4) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_text user "+" (at -3.55 -2.25 0) (layer "F.SilkS")
    (effects (font (size 1.4 1.4) (thickness 0.22))))
  (fp_rect (start -5 -5) (end 5 5) (stroke (width 0.05) (type solid)) (fill none) (layer "F.CrtYd"))
  (fp_text user "${REFERENCE}" (at 0 0 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (pad "1" smd roundrect (at -3.6 -3.6) (size 2.4 2.4) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (pad "2" smd roundrect (at -3.6 3.6) (size 2.4 2.4) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (pad "3" smd roundrect (at 3.6 -3.6) (size 2.4 2.4) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (pad "4" smd roundrect (at 3.6 3.6) (size 2.4 2.4) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (embedded_fonts no)
)
''', encoding='utf-8')

# -----------------------------------------------------------------------------
# U2 — Silicon Labs CP2102-GM, exact classic QFN28 recommended PCB land pattern.
# Silicon Labs CP2102/9 Rev 1.8: 5x5 mm, 0.50 mm pitch, perimeter land
# X1=0.20..0.30 / Y1=0.85..0.95, center X2/Y2=3.20..3.30 and 3x3 paste
# apertures 0.90 mm on 1.10 mm pitch. We use the upper perimeter values and
# midpoint 3.25 mm center copper.
# -----------------------------------------------------------------------------
(LIB / 'CP2102_GM_QFN28_5x5_P0.5_EP3.25.kicad_mod').write_text(r'''(footprint "CP2102_GM_QFN28_5x5_P0.5_EP3.25"
  (version 20260206)
  (generator "pcbnew")
  (generator_version "10.0")
  (layer "F.Cu")
  (descr "Silicon Labs CP2102-GM QFN28 5x5 P0.5; manufacturer recommended land pattern, EP 3.25x3.25 and 3x3 paste apertures")
  (tags "CP2102 CP2102-GM QFN28 USB UART")
  (property "Reference" "REF**" (at 0 -3.7 0) (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Value" "CP2102-GM" (at 0 3.7 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Datasheet" "https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf" (at 0 0 0) (layer "F.Fab") (hide yes)
    (effects (font (size 1.27 1.27))))
  (attr smd)
  (fp_rect (start -2.5 -2.5) (end 2.5 2.5) (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_line (start -2.62 -2.62) (end -1.8 -2.62) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start -2.62 -2.62) (end -2.62 -1.8) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_poly (pts (xy -3.05 -1.5) (xy -3.38 -1.26) (xy -3.38 -1.74)) (stroke (width 0.12) (type solid)) (fill yes) (layer "F.SilkS"))
  (fp_rect (start -3.05 -3.05) (end 3.05 3.05) (stroke (width 0.05) (type solid)) (fill none) (layer "F.CrtYd"))
  (fp_text user "${REFERENCE}" (at 0 0 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (pad "" smd roundrect (at -1.1 -1.1) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at 0 -1.1) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at 1.1 -1.1) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at -1.1 0) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at 0 0) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at 1.1 0) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at -1.1 1.1) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at 0 1.1) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "" smd roundrect (at 1.1 1.1) (size 0.9 0.9) (layers "F.Paste") (roundrect_rratio 0.12))
  (pad "1" smd roundrect (at -2.4 -1.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "2" smd roundrect (at -2.4 -1.0) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "3" smd roundrect (at -2.4 -0.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "4" smd roundrect (at -2.4 0) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "5" smd roundrect (at -2.4 0.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "6" smd roundrect (at -2.4 1.0) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "7" smd roundrect (at -2.4 1.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "8" smd roundrect (at -1.5 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "9" smd roundrect (at -1.0 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "10" smd roundrect (at -0.5 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "11" smd roundrect (at 0 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "12" smd roundrect (at 0.5 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "13" smd roundrect (at 1.0 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "14" smd roundrect (at 1.5 2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "15" smd roundrect (at 2.4 1.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "16" smd roundrect (at 2.4 1.0) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "17" smd roundrect (at 2.4 0.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "18" smd roundrect (at 2.4 0) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "19" smd roundrect (at 2.4 -0.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "20" smd roundrect (at 2.4 -1.0) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "21" smd roundrect (at 2.4 -1.5) (size 0.95 0.30) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "22" smd roundrect (at 1.5 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "23" smd roundrect (at 1.0 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "24" smd roundrect (at 0.5 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "25" smd roundrect (at 0 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "26" smd roundrect (at -0.5 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "27" smd roundrect (at -1.0 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "28" smd roundrect (at -1.5 -2.4) (size 0.30 0.95) (layers "F.Cu" "F.Mask" "F.Paste") (roundrect_rratio 0.25))
  (pad "29" smd rect (at 0 0) (size 3.25 3.25) (layers "F.Cu" "F.Mask") (zone_connect 2))
  (embedded_fonts no)
  (model "${KICAD10_3DMODEL_DIR}/Package_DFN_QFN.3dshapes/QFN-28-1EP_5x5mm_P0.5mm_EP3.35x3.35mm.step"
    (offset (xyz 0 0 0)) (scale (xyz 1 1 1)) (rotate (xyz 0 0 0)))
)
''', encoding='utf-8')

# -----------------------------------------------------------------------------
# SW1..SW6 — GCT SWT0110-020010SSA exact recommended PCB layout.
# Drawing: body 3.90 x 2.93 mm, 2.00 mm actuator height; lands 1.05 x 2.00 mm,
# inner gap 3.40 mm => 4.45 mm center-to-center.
# -----------------------------------------------------------------------------
(LIB / 'SW_GCT_SWT0110.kicad_mod').write_text(r'''(footprint "SW_GCT_SWT0110"
  (version 20260206)
  (generator "pcbnew")
  (generator_version "10.0")
  (layer "F.Cu")
  (descr "GCT SWT0110-020010SSA tactile switch; exact recommended PCB pads 1.05x2.00 mm, 4.45 mm pitch")
  (tags "switch tactile GCT SWT0110 3.9x2.93")
  (property "Reference" "REF**" (at 0 -2.6 0) (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Value" "SWT0110-020010SSA" (at 0 2.6 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Datasheet" "https://gct.co/connector/swt0110" (at 0 0 0) (layer "F.Fab") (hide yes)
    (effects (font (size 1.27 1.27))))
  (attr smd)
  (fp_rect (start -1.95 -1.465) (end 1.95 1.465) (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_line (start -1.95 -1.56) (end 1.95 -1.56) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start -1.95 1.56) (end 1.95 1.56) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_circle (center 0 0) (end 0.9 0) (stroke (width 0.18) (type solid)) (fill none) (layer "F.SilkS"))
  (fp_rect (start -2.85 -1.65) (end 2.85 1.65) (stroke (width 0.05) (type solid)) (fill none) (layer "F.CrtYd"))
  (fp_text user "${REFERENCE}" (at 0 0 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (pad "1" smd roundrect (at -2.225 0) (size 1.05 2.00) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (pad "2" smd roundrect (at 2.225 0) (size 1.05 2.00) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (embedded_fonts no)
  (model "${KIPRJMOD}/libs/JLCPCB.3dshapes/SW-SMD_L3.9-W2.9-H2.0-LS4.8.step"
    (offset (xyz 0 0 0)) (scale (xyz 1 1 1)) (rotate (xyz 0 0 0)))
)
''', encoding='utf-8')

# Correct the stale project procurement record by replacing it with the closed audit state.
proc = ROOT / 'docs' / 'PROCUREMENT.md'
proc.write_text('''# Procurement and footprint state

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
''', encoding='utf-8')

# Align the buck design record with the closed sourcing/footprint state.
design = Path('docs/BUCK_L7987L_DESIGN.md')
txt = design.read_text(encoding='utf-8')
replacements = {
'''Repository checkpoint verified for this document: schematic at commit `9ae20baf45efc8dad69b3fa07e15d245c30e2893`, regenerated netlist at commit `3f02fa303bba51f2bb647fd7bd7127f1000f8ca1`.''':
'''Sourcing and footprint audit closed on **2026-09-02** on this branch; the actual schematic/netlist at branch HEAD remain authoritative.''',
'''> **Sourcing / footprint status**
>
> The `BOM TME` sourcing pass and KiCad MPN/manufacturer metadata synchronization have been
> completed for the selected parts. The next manufacturing-preparation step is the footprint
> audit against the selected MPNs, followed by PCB synchronization and layout work.''':
'''> **Sourcing / footprint status**
>
> The `BOM TME` sourcing pass, KiCad MPN/manufacturer synchronization and footprint audit were
> completed on 2026-09-02. Sourcing/footprints are no longer a blocker; the next step is PCB
> synchronization, placement/routing and PCB-level verification.''',
'''The KiCad metadata for the current TLV1701 symbol still contains an old LM397 datasheet URL;
that is a known metadata cleanup item and does not describe the actual selected comparator.''':
'''The KiCad metadata for the selected TLV1701 has been synchronized to the final sourced part and datasheet.''',
'''The 3.3 V loads include the ESP32-WROOM-32E, CP2102N, sensor/display interfaces, pull-ups,
buzzer/logic and related circuitry.''':
'''The 3.3 V loads include the ESP32-WROOM-32E, CP2102 USB-UART bridge, sensor/display interfaces, pull-ups,
buzzer/logic and related circuitry.''',
'''At the current checkpoint, the pre-FB1 output net has no explicit `3V3_BUCK` label and KiCad
exports it as **`Net-(U5-VBIAS)`** because U5 VBIAS is tied to that output node. This is
currently an electrical-equivalent naming issue only. Restoring the historical `3V3_BUCK`
label is a cleanup option before PCB synchronization.''':
'''The pre-FB1 regulated output node is explicitly labeled **`3V3_BUCK`** and then feeds `FB1` toward `3V3_MCU`.''',
'''These MPN/rating entries now reflect the sourced BOM selection; footprint/PCB validation remains
the remaining manufacturing-preparation work.''':
'''These MPN/rating entries reflect the final sourced BOM selection, and the corresponding L1 footprint has been validated against the Bourns recommended land pattern. PCB placement/routing remains pending.''',
}
for old, new in replacements.items():
    if old not in txt:
        raise SystemExit('Expected design-doc text not found:\n' + old)
    txt = txt.replace(old, new, 1)
design.write_text(txt, encoding='utf-8')

# Basic syntax/geometry and assignment validation.
def check_sexpr(path):
    text = path.read_text(encoding='utf-8')
    depth = 0
    ins = False
    esc = False
    for ch in text:
        if ins:
            if esc:
                esc = False
            elif ch == '\\':
                esc = True
            elif ch == '"':
                ins = False
        else:
            if ch == '"':
                ins = True
            elif ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
                if depth < 0:
                    raise SystemExit(f'{path}: negative S-expression depth')
    if depth != 0 or ins:
        raise SystemExit(f'{path}: malformed S-expression depth={depth}, string={ins}')

for name in [
    'BUZ_Loudity_SMT67_8.5x8.5.kicad_mod',
    'CP2102_GM_QFN28_5x5_P0.5_EP3.25.kicad_mod',
    'SW_GCT_SWT0110.kicad_mod',
    'L_Bourns_SRN6045.kicad_mod',
    'CP_Panasonic_F_8x10.2.kicad_mod',
    'USB_C_GCT_USB4216-03-A.kicad_mod',
]:
    check_sexpr(LIB / name)

# Re-read schematics and prove every audited reference now has the final assignment.
found = {}
for path in sorted(ROOT.glob('*.kicad_sch')):
    text = path.read_text(encoding='utf-8')
    for _, _, ref, block in symbol_spans(text):
        if ref in TARGETS:
            found[ref] = prop(block, 'Footprint')
for ref, fp in TARGETS.items():
    if found.get(ref) != fp:
        raise SystemExit(f'{ref}: final footprint validation failed: {found.get(ref)!r} != {fp!r}')

# Make sure no sourced BOM reference still has an empty footprint.  Expand the
# reference groups used by the current BOM TME sheet.
bom_groups = [
    'BZ1','C11','C12','C2,C6,C13,C14,C18,C19','C15,C17,C21','C16','C4','C5','C1','C3','C7','C8','C9','C10','C20',
    'D1','D2','D3','D4,D5','D6','D7','F1','FB1','J1','J2','J3','J5','J7','J6','L1','Q1','Q2,Q3,Q6,Q7','Q4','Q5',
    'R3,R5,R6,R21,R26,R31,R37','R10,R11','R12,R13,R14,R15,R19,R22,R23,R24','R16,R17','R18','R20,R25','R27','R4,R28,R30',
    'R1','R7','R8,R29','R9','R2','R32','R33,R34','R35','R36','SW1,SW2,SW3,SW4,SW5,SW6','U1','U2','U3','U4','U5'
]
bom_refs = set()
for group in bom_groups:
    bom_refs.update(x.strip() for x in group.split(','))

all_instances = {}
for path in sorted(ROOT.glob('*.kicad_sch')):
    text = path.read_text(encoding='utf-8')
    for _, _, ref, block in symbol_spans(text):
        if ref in bom_refs:
            all_instances[ref] = prop(block, 'Footprint')
missing_refs = sorted(bom_refs - set(all_instances))
empty_fp = sorted(ref for ref, fp in all_instances.items() if not fp)
if missing_refs:
    raise SystemExit(f'BOM refs missing from schematics: {missing_refs}')
if empty_fp:
    raise SystemExit(f'BOM refs with empty footprint: {empty_fp}')

print('Footprint audit final validation: PASS')
print('Audited final assignments:')
for ref in sorted(TARGETS):
    print(f'  {ref}: {TARGETS[ref]}')
print(f'BOM references with non-empty footprints: {len(all_instances)} / {len(bom_refs)}')
