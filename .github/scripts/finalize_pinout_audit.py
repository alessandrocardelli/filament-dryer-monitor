from pathlib import Path
import re

ROOT = Path('hardware')


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


def get_block(text, pattern):
    m = re.search(pattern, text, re.M)
    if not m:
        raise RuntimeError(f'Block not found: {pattern}')
    s = m.start()
    e = balanced_end(text, s)
    return s, e, text[s:e]


def replace_block(text, s, e, block):
    return text[:s] + block + text[e:]


def pin_blocks(block):
    out = []
    for m in re.finditer(r'\(pin\s+', block):
        s = m.start()
        e = balanced_end(block, s)
        pb = block[s:e]
        mn = re.search(r'\(number\s+"([^"]+)"', pb)
        nm = re.search(r'\(name\s+"([^"]*)"', pb)
        typ = re.match(r'\(pin\s+([^\s()]+)', pb)
        if mn:
            out.append((s, e, mn.group(1), nm.group(1) if nm else '', typ.group(1) if typ else '', pb))
    return out


def set_pin_type_by_number(symbol_block, number, pin_type):
    matches = [x for x in pin_blocks(symbol_block) if x[2] == str(number)]
    if len(matches) != 1:
        raise RuntimeError(f'Pin {number} matched {len(matches)} times')
    s, e, _, _, _, pb = matches[0]
    new_pb, n = re.subn(r'^\(pin\s+[^\s()]+\s+', f'(pin {pin_type} ', pb, count=1)
    if n != 1:
        raise RuntimeError(f'Could not set pin {number} type')
    return symbol_block[:s] + new_pb + symbol_block[e:]


def add_d1_nc_pin(text, outer_name):
    outer_pat = r'\(symbol\s+"' + re.escape(outer_name) + r'"'
    os, oe, outer = get_block(text, outer_pat)
    unit_pat = r'\(symbol\s+"BZX84C15_1_1"'
    us, ue, unit = get_block(outer, unit_pat)
    numbers = {x[2] for x in pin_blocks(unit)}
    if '2' not in numbers:
        nc_pin = '''\n\t\t(pin no_connect line
\t\t\t(at 0 3.81 270)
\t\t\t(length 2.54)
\t\t\t(name "NC"
\t\t\t\t(effects
\t\t\t\t\t(font
\t\t\t\t\t\t(size 1.27 1.27)
\t\t\t\t\t)
\t\t\t\t)
\t\t\t)
\t\t\t(number "2"
\t\t\t\t(effects
\t\t\t\t\t(font
\t\t\t\t\t\t(size 1.27 1.27)
\t\t\t\t\t)
\t\t\t\t)
\t\t\t)
\t\t)'''
        unit = unit[:-1] + nc_pin + '\n\t)'
        outer = replace_block(outer, us, ue, unit)
        text = replace_block(text, os, oe, outer)
    return text


def set_cp2102_pin2_input(text, outer_name):
    outer_pat = r'\(symbol\s+"' + re.escape(outer_name) + r'"'
    os, oe, outer = get_block(text, outer_pat)
    unit_pat = r'\(symbol\s+"CP2102-GM_1_1"'
    us, ue, unit = get_block(outer, unit_pat)
    unit = set_pin_type_by_number(unit, '2', 'input')
    outer = replace_block(outer, us, ue, unit)
    return replace_block(text, os, oe, outer)


def normalize_blank_ws(text):
    return re.sub(r'(?m)^[ \t]+$', '', text)


# Fix the embedded schematic symbol and the reusable project library symbol.
power_path = ROOT / 'Power.kicad_sch'
power = power_path.read_text(encoding='utf-8')
power = add_d1_nc_pin(power, 'Filament_Dryer_Custom:BZX84C15-7-F')
power = power.replace(
    'http://www.ti.com/lit/ds/symlink/lm397.pdf',
    'https://www.ti.com/lit/ds/symlink/tlv1701.pdf',
)
power_path.write_text(normalize_blank_ws(power), encoding='utf-8')

mcu_path = ROOT / 'MCU.kicad_sch'
mcu = mcu_path.read_text(encoding='utf-8')
mcu = set_cp2102_pin2_input(mcu, 'Filament_Dryer_Custom:CP2102-GM')
mcu_path.write_text(normalize_blank_ws(mcu), encoding='utf-8')

custom_path = ROOT / 'Filament_Dryer_Custom.kicad_sym'
custom = custom_path.read_text(encoding='utf-8')
custom = add_d1_nc_pin(custom, 'BZX84C15-7-F')
custom = set_cp2102_pin2_input(custom, 'CP2102-GM')
custom = custom.replace(
    'http://www.ti.com/lit/ds/symlink/lm397.pdf',
    'https://www.ti.com/lit/ds/symlink/tlv1701.pdf',
)
custom_path.write_text(normalize_blank_ws(custom), encoding='utf-8')

# Update the procurement/audit record with the stricter pin-level result and the
# small set of checks that are intentionally deferred to physical PCB review.
proc_path = ROOT / 'docs' / 'PROCUREMENT.md'
proc = proc_path.read_text(encoding='utf-8')
old = '''Standard 0603/0805/1210 passives, SOT-23/SOT-23-5/SOT-23-6 devices, DPAK/TO-252 devices, SMA/SMB/SOD-123/CFP3 diodes, JST connectors, ESP32 module and the remaining already-established footprints were checked against their selected package/MPN during the same pass.\n\n### BZ1 qualification note\n'''
new = '''Standard 0603/0805/1210 passives, SOT-23/SOT-23-5/SOT-23-6 devices, DPAK/TO-252 devices, SMA/SMB/SOD-123/CFP3 diodes, JST connectors, ESP32 module and the remaining already-established footprints were checked against their selected package/MPN during the same pass.\n\n### Strict pinout / symbol audit — closed 2026-09-02\n\nA second pass checked selected MPN pin numbering against the KiCad symbol and footprint pad numbering, not only package geometry. It found and corrected three real symbol-level issues:\n\n- `D1 = BZX84C15-7-F`: the generic two-pin Zener symbol could not represent the SOT-23 device correctly. The project symbol now maps **pin 1 = A, pin 2 = NC, pin 3 = K** while retaining the standard KiCad SOT-23 footprint.\n- `Q5 = IRLML2060TRPBF`: the previous generic MOSFET symbol used G-D-S numbering. The final device is **1=G, 2=S, 3=D**, so Q5 now uses the matching G-S-D symbol with the standard KiCad SOT-23 footprint.\n- `U2 = CP2102-GM`: the schematic had inherited a CP2102N symbol. A dedicated classic CP2102-GM symbol is now used; pins **10 and 13–22 are NC**, pin 2 is `~RI` input, and the custom QFN28 footprint remains the Silicon Labs classic CP2102 land pattern.\n\nThe remaining semiconductor mappings were checked without finding another pin-numbering mismatch. `U3 = AKS1201` retains the USBLC6-2SC6 topology and standard SOT-23-6 footprint; `Q1/Q4`, the MMBT3904 devices, `U1`, `U4` and `U5` retain their audited mappings.\n\nThe following items are deliberately **not** treated as unresolved electrical pinout errors, but still require normal physical/manufacturing review before fabrication:\n\n- `J5`: the real part is a HALJIA XH-compatible connector, so fit/polarization must be confirmed against the actual connector despite the 2.50 mm JST-XH footprint.\n- `U5`: HTSSOP-16 copper/pad geometry is verified, but the exposed-pad **stencil/paste aperture strategy** must be reviewed before final paste Gerbers/assembly.\n- `U4`: verify final ESP32 antenna keepout and board-edge placement in PCB layout.\n- optional OLED footprint: it is custom and excluded from the BOM; verify mechanically only if the display is installed.\n\n### BZ1 qualification note\n'''
if old not in proc:
    raise RuntimeError('Expected PROCUREMENT insertion anchor not found')
proc = proc.replace(old, new, 1)
proc_path.write_text(proc, encoding='utf-8')

# Final structural assertions.
power = power_path.read_text(encoding='utf-8')
_, _, d1outer = get_block(power, r'\(symbol\s+"Filament_Dryer_Custom:BZX84C15-7-F"')
_, _, d1unit = get_block(d1outer, r'\(symbol\s+"BZX84C15_1_1"')
d1pins = {x[2]: (x[3], x[4]) for x in pin_blocks(d1unit)}
assert d1pins == {'3': ('K', 'passive'), '1': ('A', 'passive'), '2': ('NC', 'no_connect')}, d1pins

mcu = mcu_path.read_text(encoding='utf-8')
_, _, u2outer = get_block(mcu, r'\(symbol\s+"Filament_Dryer_Custom:CP2102-GM"')
_, _, u2unit = get_block(u2outer, r'\(symbol\s+"CP2102-GM_1_1"')
u2pins = {x[2]: (x[3], x[4]) for x in pin_blocks(u2unit)}
assert u2pins['2'] == ('~{RI}', 'input'), u2pins['2']
for n in ['10'] + [str(x) for x in range(13, 23)]:
    assert u2pins[n] == ('NC', 'no_connect'), (n, u2pins[n])

assert 'lm397.pdf' not in power
assert 'lm397.pdf' not in custom_path.read_text(encoding='utf-8')
print('Final strict pinout audit cleanup: PASS')
print('D1 pins:', d1pins)
print('U2 pin 2:', u2pins['2'])
