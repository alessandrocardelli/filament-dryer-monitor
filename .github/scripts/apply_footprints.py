from pathlib import Path
import re

TARGETS = {
    'C1':  'Capacitor_SMD:C_1210_3225Metric',
    'C2':  'Capacitor_SMD:C_0603_1608Metric',
    'C3':  'Capacitor_SMD:C_0805_2012Metric',
    'C4':  'Capacitor_SMD:C_0603_1608Metric',
    'C5':  'FilamentDryer:CP_Panasonic_F_8x10.2',
    'C6':  'Capacitor_SMD:C_0603_1608Metric',
    'C7':  'Capacitor_SMD:C_0805_2012Metric',
    'C8':  'Capacitor_SMD:C_0603_1608Metric',
    'C9':  'Capacitor_SMD:C_0805_2012Metric',
    'C10': 'Capacitor_SMD:C_1210_3225Metric',
    'C20': 'Capacitor_SMD:C_0603_1608Metric',
    'C21': 'Capacitor_SMD:C_0603_1608Metric',
    'D7':  'Diode_SMD:D_SMA',
    'J2':  'FilamentDryer:USB_C_GCT_USB4216-03-A',
    'L1':  'FilamentDryer:L_Bourns_SRN6045',
    'R1':  'Resistor_SMD:R_0603_1608Metric',
    'R2':  'Resistor_SMD:R_0603_1608Metric',
    'R4':  'Resistor_SMD:R_0603_1608Metric',
    'R5':  'Resistor_SMD:R_0603_1608Metric',
    'R6':  'Resistor_SMD:R_0603_1608Metric',
    'R29': 'Resistor_SMD:R_0603_1608Metric',
    'R30': 'Resistor_SMD:R_0603_1608Metric',
    'R31': 'Resistor_SMD:R_0603_1608Metric',
    'R32': 'Resistor_SMD:R_0603_1608Metric',
    'R33': 'Resistor_SMD:R_0603_1608Metric',
    'R34': 'Resistor_SMD:R_0603_1608Metric',
    'R35': 'Resistor_SMD:R_0603_1608Metric',
    'R36': 'Resistor_SMD:R_0603_1608Metric',
    'U5':  'SamacSys_Parts:SOP65P640X120-17N',
}

EXPECTED_OLD = {
    'C5': 'JLCPCB:CAP-SMD_BD8.0-L8.3-W8.3-FD',
    'J2': 'FilamentDryer:USB_C_Receptacle_HRO_TYPE-C-31-M-12',
    'U5': 'SOP65P640X120-17N',
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


def property_value(block, name):
    m = re.search(
        r'\(property\s+"' + re.escape(name) + r'"\s+"((?:\\.|[^"])*)"',
        block,
    )
    return None if not m else m.group(1)


def replace_property_value(block, name, value):
    pat = re.compile(
        r'(\(property\s+"' + re.escape(name) + r'"\s+")((?:\\.|[^"])*)"'
    )
    out, n = pat.subn(lambda m: m.group(1) + value + '"', block, count=1)
    if n != 1:
        raise ValueError(f'Cannot update {name}')
    return out


seen = set()
changed = set()
for path in sorted(Path('hardware').glob('*.kicad_sch')):
    text = path.read_text(encoding='utf-8')
    replacements = []
    for start, end, ref, block in symbol_spans(text):
        if ref not in TARGETS:
            continue
        seen.add(ref)
        old = property_value(block, 'Footprint')
        if old is None:
            raise SystemExit(f'{ref}: Footprint property missing in {path}')

        if ref in EXPECTED_OLD:
            if old not in (EXPECTED_OLD[ref], TARGETS[ref]):
                raise SystemExit(
                    f'{ref}: unexpected existing footprint {old!r}; '
                    f'expected {EXPECTED_OLD[ref]!r}'
                )
        elif old not in ('', TARGETS[ref]):
            raise SystemExit(f'{ref}: refusing to overwrite unexpected footprint {old!r}')

        if old != TARGETS[ref]:
            replacements.append(
                (start, end, replace_property_value(block, 'Footprint', TARGETS[ref]))
            )
            changed.add(ref)

    for start, end, block in sorted(replacements, reverse=True):
        text = text[:start] + block + text[end:]
    path.write_text(text, encoding='utf-8')

missing = set(TARGETS) - seen
if missing:
    raise SystemExit(f'Missing schematic refs: {sorted(missing)}')

# Bourns SRN6045 manufacturer recommended layout:
# two 2.0 x 6.0 mm lands, 2.3 mm clear gap.
Path('hardware/libs/FilamentDryer.pretty/L_Bourns_SRN6045.kicad_mod').write_text('''(footprint "L_Bourns_SRN6045"
  (version 20260206)
  (generator "pcbnew")
  (generator_version "10.0")
  (layer "F.Cu")
  (descr "Bourns SRN6045; recommended layout 2.0x6.0 mm lands, 2.3 mm gap")
  (tags "inductor SRN6045 Bourns")
  (property "Reference" "REF**" (at 0 -4.1 0) (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Value" "L_Bourns_SRN6045" (at 0 4.1 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Datasheet" "https://www.bourns.com/docs/Product-Datasheets/SRN6045.pdf" (at 0 0 0) (layer "F.Fab") (hide yes)
    (effects (font (size 1.27 1.27))))
  (attr smd)
  (fp_rect (start -3 -3) (end 3 3) (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_line (start -3.1 -3.1) (end -1.35 -3.1) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start 1.35 -3.1) (end 3.1 -3.1) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start -3.1 3.1) (end -1.35 3.1) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start 1.35 3.1) (end 3.1 3.1) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_rect (start -3.4 -3.4) (end 3.4 3.4) (stroke (width 0.05) (type solid)) (fill none) (layer "F.CrtYd"))
  (fp_text user "${REFERENCE}" (at 0 0 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (pad "1" smd roundrect (at -2.15 0) (size 2 6) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (pad "2" smd roundrect (at 2.15 0) (size 2 6) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.08))
  (embedded_fonts no)
)
''', encoding='utf-8')

# Panasonic FK standard size F, Ø8 x 10.2 mm.
# Catalog land pattern: a=3.1, b=4.0, c=2.0 mm.
# Rotated 90 degrees here; physical geometry is unchanged.
Path('hardware/libs/FilamentDryer.pretty/CP_Panasonic_F_8x10.2.kicad_mod').write_text('''(footprint "CP_Panasonic_F_8x10.2"
  (version 20260206)
  (generator "pcbnew")
  (generator_version "10.0")
  (layer "F.Cu")
  (descr "Panasonic FK standard size F, diameter 8 x 10.2 mm; land a=3.1 b=4.0 c=2.0 mm")
  (tags "capacitor electrolytic Panasonic FK size F 8mm")
  (property "Reference" "REF**" (at 0 -5.1 0) (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Value" "CP_Panasonic_F_8x10.2" (at 0 5.1 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Datasheet" "https://industrial.panasonic.com/cdbs/www-data/pdf/RDE0000/ABA0000C1181.pdf" (at 0 0 0) (layer "F.Fab") (hide yes)
    (effects (font (size 1.27 1.27))))
  (attr smd)
  (fp_rect (start -4.15 -4.15) (end 4.15 4.15) (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_circle (center 0 0) (end 4 0) (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_line (start -4.25 -4.25) (end -1.6 -4.25) (stroke (width 0.15) (type solid)) (layer "F.SilkS"))
  (fp_line (start -4.25 -4.25) (end -4.25 4.25) (stroke (width 0.15) (type solid)) (layer "F.SilkS"))
  (fp_line (start -4.25 4.25) (end -1.6 4.25) (stroke (width 0.15) (type solid)) (layer "F.SilkS"))
  (fp_text user "+" (at -5.05 0 0) (layer "F.SilkS")
    (effects (font (size 1.2 1.2) (thickness 0.2))))
  (fp_rect (start -5.8 -4.5) (end 5.8 4.5) (stroke (width 0.05) (type solid)) (fill none) (layer "F.CrtYd"))
  (fp_text user "${REFERENCE}" (at 0 0 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (pad "1" smd roundrect (at -3.55 0) (size 4 2) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.1))
  (pad "2" smd roundrect (at 3.55 0) (size 4 2) (layers "F.Cu" "F.Paste" "F.Mask") (roundrect_rratio 0.1))
  (embedded_fonts no)
)
''', encoding='utf-8')

# GCT USB4216-03-A recommended PCB geometry. Shell stakes are fully SMT.
Path('hardware/libs/FilamentDryer.pretty/USB_C_GCT_USB4216-03-A.kicad_mod').write_text('''(footprint "USB_C_GCT_USB4216-03-A"
  (version 20260206)
  (generator "pcbnew")
  (generator_version "10.0")
  (layer "F.Cu")
  (descr "GCT USB4216-03-A USB-C USB2 receptacle, 16 contacts, fully SMT shell")
  (tags "USB C USB2 GCT USB4216")
  (property "Reference" "REF**" (at 0 -5.2 0) (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Value" "USB_C_GCT_USB4216-03-A" (at 0 4.4 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (property "Datasheet" "https://gct.co/connector/usb4216" (at 0 0 0) (layer "F.Fab") (hide yes)
    (effects (font (size 1.27 1.27))))
  (attr smd)
  (fp_line (start -4.47 -1.75) (end -4.47 -0.15) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start -4.47 2.45) (end -4.47 3.25) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start 4.47 -1.75) (end 4.47 -0.15) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_line (start 4.47 2.45) (end 4.47 3.25) (stroke (width 0.12) (type solid)) (layer "F.SilkS"))
  (fp_circle (center -3.2 -4.55) (end -3.05 -4.55) (stroke (width 0.12) (type solid)) (fill none) (layer "F.SilkS"))
  (fp_rect (start -4.47 -3.25) (end 4.47 3.25) (stroke (width 0.1) (type solid)) (fill none) (layer "F.Fab"))
  (fp_rect (start -6.12 -4.25) (end 6.12 3.5) (stroke (width 0.05) (type solid)) (fill none) (layer "F.CrtYd"))
  (fp_text user "${REFERENCE}" (at 0 0 0) (layer "F.Fab")
    (effects (font (size 1 1) (thickness 0.15))))
  (pad "" np_thru_hole circle (at -3 -2.15) (size 0.65 0.65) (drill 0.65) (layers "*.Cu" "*.Mask"))
  (pad "" np_thru_hole circle (at 3 -2.15) (size 0.65 0.65) (drill 0.65) (layers "*.Cu" "*.Mask"))
  (pad "A1" smd rect (at -3.2 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B12" smd rect (at -3.2 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A4" smd rect (at -2.4 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B9" smd rect (at -2.4 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B8" smd rect (at -1.75 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A5" smd rect (at -1.25 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B7" smd rect (at -0.75 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A6" smd rect (at -0.25 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A7" smd rect (at 0.25 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B6" smd rect (at 0.75 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A8" smd rect (at 1.25 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B5" smd rect (at 1.75 -3.425) (size 0.3 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B4" smd rect (at 2.4 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A9" smd rect (at 2.4 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "B1" smd rect (at 3.2 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "A12" smd rect (at 3.2 -3.425) (size 0.6 1.15) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "SH" smd rect (at -5.095 -2.775) (size 1.55 1.25) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "SH" smd rect (at 5.095 -2.775) (size 1.55 1.25) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "SH" smd rect (at -5.095 1.15) (size 1.55 1.5) (layers "F.Cu" "F.Paste" "F.Mask"))
  (pad "SH" smd rect (at 5.095 1.15) (size 1.55 1.5) (layers "F.Cu" "F.Paste" "F.Mask"))
  (embedded_fonts no)
)
''', encoding='utf-8')

# L7987L footprint geometry was already correct; make its library reference/model portable.
u5fp = Path('hardware/libs/SamacSys_Parts.pretty/SOP65P640X120-17N.kicad_mod')
u5text = u5fp.read_text(encoding='utf-8')
old_model = r'C:\\Users\\utente\\Documents\\KiCad\\10.0\\projects\\Filament_Dryer_Monitor\\hardware\\libs\\SamacSys_Parts.3dshapes\\L7987L.stp'
new_model = r'${KIPRJMOD}/libs/SamacSys_Parts.3dshapes/L7987L.stp'
if old_model in u5text:
    u5text = u5text.replace(old_model, new_model, 1)
elif new_model not in u5text:
    raise SystemExit('Unexpected L7987L 3D model path')
u5fp.write_text(u5text, encoding='utf-8')

# Documentation: procurement is no longer deferred.
doc = Path('docs/BUCK_L7987L_DESIGN.md')
doctext = doc.read_text(encoding='utf-8')
stale = (
    'These MPN/rating entries are retained as the design record but final procurement approval is\n'
    'still deferred.'
)
fresh = (
    'These MPN/rating entries now reflect the sourced BOM selection; footprint/PCB validation remains\n'
    'the remaining manufacturing-preparation work.'
)
if stale in doctext:
    doctext = doctext.replace(stale, fresh, 1)
elif fresh not in doctext:
    raise SystemExit('Expected procurement-status sentence not found')
doc.write_text(doctext, encoding='utf-8')

# Re-read schematic assignments.
found = {}
for path in Path('hardware').glob('*.kicad_sch'):
    text = path.read_text(encoding='utf-8')
    for _, _, ref, block in symbol_spans(text):
        if ref in TARGETS:
            found[ref] = property_value(block, 'Footprint')
errors = [
    f'{ref}: {found.get(ref)!r} != {expected!r}'
    for ref, expected in TARGETS.items()
    if found.get(ref) != expected
]
if errors:
    raise SystemExit('\n'.join(errors))

# Geometry validation for custom patterns.
l1 = Path('hardware/libs/FilamentDryer.pretty/L_Bourns_SRN6045.kicad_mod').read_text(encoding='utf-8')
for token in ['(at -2.15 0) (size 2 6)', '(at 2.15 0) (size 2 6)']:
    if token not in l1:
        raise SystemExit('L1 geometry validation failed')

c5 = Path('hardware/libs/FilamentDryer.pretty/CP_Panasonic_F_8x10.2.kicad_mod').read_text(encoding='utf-8')
for token in ['(at -3.55 0) (size 4 2)', '(at 3.55 0) (size 4 2)']:
    if token not in c5:
        raise SystemExit('C5 geometry validation failed')

j2 = Path('hardware/libs/FilamentDryer.pretty/USB_C_GCT_USB4216-03-A.kicad_mod').read_text(encoding='utf-8')
for pad in ['A1','B12','A4','B9','A5','B5','A6','B6','A7','B7','A8','B8','A9','B4','A12','B1','SH']:
    if f'(pad "{pad}" ' not in j2:
        raise SystemExit(f'J2 missing pad {pad}')
if 'thru_hole oval' in j2:
    raise SystemExit('J2 incorrectly contains THT shell stakes')

u5 = u5fp.read_text(encoding='utf-8')
if new_model not in u5:
    raise SystemExit('U5 portable model path not set')

print(f'Validated footprint changes for {len(TARGETS)} refs; modified {len(changed)} assignments.')
