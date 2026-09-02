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


def find_block(text, pattern, start=0):
    m = re.search(pattern, text[start:], re.M)
    if not m:
        return None
    s = start + m.start()
    return s, balanced_end(text, s), text[s:balanced_end(text, s)]


def replace_prop(block, name, value):
    pat = re.compile(r'(\(property\s+"' + re.escape(name) + r'"\s+")((?:\\.|[^"])*)"')
    out, n = pat.subn(lambda m: m.group(1) + value + '"', block, count=1)
    if n != 1:
        raise RuntimeError(f'Property {name!r} not found')
    return out


def get_prop(block, name):
    m = re.search(r'\(property\s+"' + re.escape(name) + r'"\s+"((?:\\.|[^"])*)"', block)
    return m.group(1) if m else None


def symbol_definition(text, name):
    pat = r'\(symbol\s+"' + re.escape(name) + r'"'
    found = find_block(text, pat)
    if not found:
        raise RuntimeError(f'Symbol definition {name!r} not found')
    return found


def instance_by_ref(text, ref):
    for m in re.finditer(r'\(symbol\s*\n?\s*\(lib_id\s+"[^"]+"\)', text):
        s = m.start()
        e = balanced_end(text, s)
        block = text[s:e]
        if get_prop(block, 'Reference') == ref:
            return s, e, block
    raise RuntimeError(f'Instance {ref} not found')


def append_to_lib_symbols(text, block):
    if not re.search(r'\(lib_symbols\b', text):
        raise RuntimeError('lib_symbols block not found')
    start = re.search(r'\(lib_symbols\b', text).start()
    end = balanced_end(text, start)
    insert_at = end - 1
    indented = '\n\t\t' + block.replace('\n', '\n\t\t') + '\n\t'
    return text[:insert_at] + indented + text[insert_at:]


def append_to_symbol_library(text, block):
    root = re.search(r'\(kicad_symbol_lib\b', text)
    if not root:
        raise RuntimeError('kicad_symbol_lib root not found')
    end = balanced_end(text, root.start())
    insert_at = end - 1
    indented = '\n\t' + block.replace('\n', '\n\t') + '\n'
    return text[:insert_at] + indented + text[insert_at:]


def replace_instance(text, ref, new_block):
    s, e, _ = instance_by_ref(text, ref)
    return text[:s] + new_block + text[e:]


def rename_definition(block, old_name, new_name):
    out, n = re.subn(
        r'^\(symbol\s+"' + re.escape(old_name) + r'"',
        '(symbol "' + new_name + '"',
        block,
        count=1,
    )
    if n != 1:
        raise RuntimeError(f'Cannot rename symbol {old_name} -> {new_name}')
    return out


def rewrite_pin(block, number=None, name=None, new_number=None, new_name=None, new_type=None):
    replacements = []
    for m in re.finditer(r'\(pin\s+', block):
        s = m.start()
        e = balanced_end(block, s)
        pb = block[s:e]
        mn = re.search(r'\(number\s+"([^"]+)"', pb)
        mname = re.search(r'\(name\s+"([^"]*)"', pb)
        if not mn:
            continue
        if number is not None and mn.group(1) != str(number):
            continue
        if name is not None and (not mname or mname.group(1) != name):
            continue
        out = pb
        if new_type is not None:
            out, n = re.subn(r'^\(pin\s+[^\s()]+\s+', f'(pin {new_type} ', out, count=1)
            if n != 1:
                raise RuntimeError('Cannot replace pin type')
        if new_name is not None:
            out, n = re.subn(r'(\(name\s+")([^"]*)"', lambda x: x.group(1) + new_name + '"', out, count=1)
            if n != 1:
                raise RuntimeError('Cannot replace pin name')
        if new_number is not None:
            out, n = re.subn(r'(\(number\s+")([^"]+)"', lambda x: x.group(1) + str(new_number) + '"', out, count=1)
            if n != 1:
                raise RuntimeError('Cannot replace pin number')
        replacements.append((s, e, out))
    if len(replacements) != 1:
        raise RuntimeError(f'Pin selector number={number!r} name={name!r} matched {len(replacements)} pins')
    for s, e, out in reversed(replacements):
        block = block[:s] + out + block[e:]
    return block


def pin_map(block):
    result = {}
    for m in re.finditer(r'\(pin\s+', block):
        s = m.start(); e = balanced_end(block, s); pb = block[s:e]
        mn = re.search(r'\(number\s+"([^"]+)"', pb)
        nm = re.search(r'\(name\s+"([^"]*)"', pb)
        typ = re.match(r'\(pin\s+([^\s()]+)', pb)
        if mn:
            result[mn.group(1)] = (nm.group(1) if nm else '', typ.group(1) if typ else '')
    return result


def set_lib_id(instance, new_id):
    out, n = re.subn(r'(\(lib_id\s+")([^"]+)"', lambda m: m.group(1) + new_id + '"', instance, count=1)
    if n != 1:
        raise RuntimeError('lib_id not found')
    return out


def ensure_absent_symbol(text, name):
    if re.search(r'\(symbol\s+"' + re.escape(name) + r'"', text):
        raise RuntimeError(f'Symbol {name} already exists; refusing duplicate insertion')


# ---------------------------------------------------------------------------
# D1: BZX84C15-7-F. Physical SOT-23 uses pin 1 = A, pin 3 = K; pin 2 is NC.
# Keep the diode graphic, remap its two electrical terminals to physical pins.
# ---------------------------------------------------------------------------
power_path = ROOT / 'Power.kicad_sch'
power = power_path.read_text(encoding='utf-8')
ensure_absent_symbol(power, 'Filament_Dryer_Custom:BZX84C15-7-F')
_, _, dbase = symbol_definition(power, 'Device:D_Zener')
dnew = rename_definition(dbase, 'Device:D_Zener', 'Filament_Dryer_Custom:BZX84C15-7-F')
dnew = dnew.replace('D_Zener_', 'BZX84C15_')
dnew = replace_prop(dnew, 'Value', 'BZX84C15-7-F')
dnew = replace_prop(dnew, 'Footprint', 'Package_TO_SOT_SMD:SOT-23')
dnew = replace_prop(dnew, 'Datasheet', 'https://www.diodes.com/datasheet/download/BZX84C2V4%20-%20BZX84C51.pdf')
dnew = replace_prop(dnew, 'Description', 'BZX84C15-7-F 15 V surface-mount Zener diode, SOT-23; pin 1 A, pin 2 NC, pin 3 K')
dnew = rewrite_pin(dnew, name='K', new_number='3')
dnew = rewrite_pin(dnew, name='A', new_number='1')
power = append_to_lib_symbols(power, dnew)
_, _, d1 = instance_by_ref(power, 'D1')
d1 = set_lib_id(d1, 'Filament_Dryer_Custom:BZX84C15-7-F')
d1 = replace_prop(d1, 'Value', 'BZX84C15-7-F')
d1 = replace_prop(d1, 'Footprint', 'Package_TO_SOT_SMD:SOT-23')
d1 = replace_prop(d1, 'Datasheet', 'https://www.diodes.com/datasheet/download/BZX84C2V4%20-%20BZX84C51.pdf')
power = replace_instance(power, 'D1', d1)
power_path.write_text(power, encoding='utf-8')

# ---------------------------------------------------------------------------
# Q5: IRLML2060 SOT-23 physical pinout is G-S-D (1-2-3), not G-D-S.
# Use the standard KiCad Q_NMOS_GSD logical symbol, embedding a cloned definition.
# ---------------------------------------------------------------------------
io_path = ROOT / 'IO_Power.kicad_sch'
io = io_path.read_text(encoding='utf-8')
ensure_absent_symbol(io, 'Transistor_FET:Q_NMOS_GSD')
_, _, qbase = symbol_definition(io, 'Transistor_FET:Q_NMOS_GDS')
qnew = rename_definition(qbase, 'Transistor_FET:Q_NMOS_GDS', 'Transistor_FET:Q_NMOS_GSD')
qnew = qnew.replace('Q_NMOS_GDS_', 'Q_NMOS_GSD_')
qnew = rewrite_pin(qnew, name='D', new_number='3')
qnew = rewrite_pin(qnew, name='S', new_number='2')
io = append_to_lib_symbols(io, qnew)
_, _, q5 = instance_by_ref(io, 'Q5')
q5 = set_lib_id(q5, 'Transistor_FET:Q_NMOS_GSD')
q5 = replace_prop(q5, 'Value', 'IRLML2060TRPBF')
q5 = replace_prop(q5, 'Datasheet', 'https://www.infineon.com/dgdl/Infineon-IRLML2060-DataSheet-v01_01-EN.pdf?fileId=5546d462533600a401535664b7fb25ee')
io = replace_instance(io, 'Q5', q5)
io_path.write_text(io, encoding='utf-8')

# ---------------------------------------------------------------------------
# U2: CP2102-GM classic. Pins 10 and 13-22 are NC; unlike CP2102N, the classic
# part has no CHR/GPIO functions on pins 13-22. The currently-used pins are
# physically compatible, but use a truthful symbol to prevent future mistakes.
# ---------------------------------------------------------------------------
mcu_path = ROOT / 'MCU.kicad_sch'
mcu = mcu_path.read_text(encoding='utf-8')
ensure_absent_symbol(mcu, 'Filament_Dryer_Custom:CP2102-GM')
_, _, ubase = symbol_definition(mcu, 'Interface_USB:CP2102N-Axx-xQFN28')
unew = rename_definition(ubase, 'Interface_USB:CP2102N-Axx-xQFN28', 'Filament_Dryer_Custom:CP2102-GM')
unew = unew.replace('CP2102N-Axx-xQFN28_', 'CP2102-GM_')
unew = replace_prop(unew, 'Value', 'CP2102-GM')
unew = replace_prop(unew, 'Footprint', 'FilamentDryer:CP2102_GM_QFN28_5x5_P0.5_EP3.25')
unew = replace_prop(unew, 'Datasheet', 'https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf')
unew = replace_prop(unew, 'Description', 'Silicon Labs CP2102 classic USB-to-UART bridge, QFN28 5x5 mm')
unew = rewrite_pin(unew, number='2', new_name='~{RI}')
unew = rewrite_pin(unew, number='7', new_name='REGIN')
for n in range(13, 23):
    unew = rewrite_pin(unew, number=str(n), new_name='NC', new_type='no_connect')
mcu = append_to_lib_symbols(mcu, unew)
_, _, u2 = instance_by_ref(mcu, 'U2')
u2 = set_lib_id(u2, 'Filament_Dryer_Custom:CP2102-GM')
u2 = replace_prop(u2, 'Value', 'CP2102-GM')
u2 = replace_prop(u2, 'Footprint', 'FilamentDryer:CP2102_GM_QFN28_5x5_P0.5_EP3.25')
u2 = replace_prop(u2, 'Datasheet', 'https://www.silabs.com/documents/public/data-sheets/CP2102-9.pdf')
mcu = replace_instance(mcu, 'U2', u2)
mcu_path.write_text(mcu, encoding='utf-8')

# ---------------------------------------------------------------------------
# Keep the project custom-symbol library in sync for future symbol updates.
# ---------------------------------------------------------------------------
custom_path = ROOT / 'Filament_Dryer_Custom.kicad_sym'
custom = custom_path.read_text(encoding='utf-8')
if not re.search(r'\(symbol\s+"BZX84C15-7-F"', custom):
    dlib = rename_definition(dnew, 'Filament_Dryer_Custom:BZX84C15-7-F', 'BZX84C15-7-F')
    custom = append_to_symbol_library(custom, dlib)
if not re.search(r'\(symbol\s+"CP2102-GM"', custom):
    ulib = rename_definition(unew, 'Filament_Dryer_Custom:CP2102-GM', 'CP2102-GM')
    custom = append_to_symbol_library(custom, ulib)
custom_path.write_text(custom, encoding='utf-8')

# ---------------------------------------------------------------------------
# Correct stale Value text that still named superseded components.
# This does not alter connectivity or footprints.
# ---------------------------------------------------------------------------
for path, changes in [
    (ROOT / 'Power.kicad_sch', {'Q1': 'NTD20P06LT4G', 'FB1': 'BLM31KN601SH1L'}),
    (ROOT / 'IO_Power.kicad_sch', {'Q5': 'IRLML2060TRPBF'}),
    (ROOT / 'MCU.kicad_sch', {'U3': 'AKS1201'}),
]:
    text = path.read_text(encoding='utf-8')
    for ref, value in changes.items():
        _, _, inst = instance_by_ref(text, ref)
        inst = replace_prop(inst, 'Value', value)
        text = replace_instance(text, ref, inst)
    path.write_text(text, encoding='utf-8')

# ---------------------------------------------------------------------------
# Validate the intended final mappings structurally.
# ---------------------------------------------------------------------------
power = power_path.read_text(encoding='utf-8')
io = io_path.read_text(encoding='utf-8')
mcu = mcu_path.read_text(encoding='utf-8')

_, _, d1 = instance_by_ref(power, 'D1')
assert re.search(r'\(lib_id\s+"Filament_Dryer_Custom:BZX84C15-7-F"\)', d1)
assert get_prop(d1, 'Footprint') == 'Package_TO_SOT_SMD:SOT-23'
_, _, ddef = symbol_definition(power, 'Filament_Dryer_Custom:BZX84C15-7-F')
dpins = pin_map(ddef)
assert dpins['1'][0] == 'A' and dpins['3'][0] == 'K' and '2' not in dpins, dpins

_, _, q5 = instance_by_ref(io, 'Q5')
assert re.search(r'\(lib_id\s+"Transistor_FET:Q_NMOS_GSD"\)', q5)
assert get_prop(q5, 'Footprint') == 'Package_TO_SOT_SMD:SOT-23'
_, _, qdef = symbol_definition(io, 'Transistor_FET:Q_NMOS_GSD')
qpins = pin_map(qdef)
assert qpins['1'][0] == 'G' and qpins['2'][0] == 'S' and qpins['3'][0] == 'D', qpins

_, _, u2 = instance_by_ref(mcu, 'U2')
assert re.search(r'\(lib_id\s+"Filament_Dryer_Custom:CP2102-GM"\)', u2)
assert get_prop(u2, 'Footprint') == 'FilamentDryer:CP2102_GM_QFN28_5x5_P0.5_EP3.25'
_, _, udef = symbol_definition(mcu, 'Filament_Dryer_Custom:CP2102-GM')
upins = pin_map(udef)
for n in ['10'] + [str(x) for x in range(13, 23)]:
    assert upins[n][0] == 'NC', (n, upins[n])
    assert upins[n][1] == 'no_connect', (n, upins[n])
assert upins['2'][0] == '~{RI}'
assert upins['7'][0] == 'REGIN'

print('Strict pinout fixes: PASS')
print('D1:', dpins)
print('Q5:', qpins)
print('U2 pins 10,13-22:', {k: upins[k] for k in ['10'] + [str(x) for x in range(13,23)]})
