from pathlib import Path

paths = [
    Path('hardware/Power.kicad_sch'),
    Path('hardware/Filament_Dryer_Custom.kicad_sym'),
]

replacements = {
    '"BZX84C15_0_1"': '"BZX84C15-7-F_0_1"',
    '"BZX84C15_1_1"': '"BZX84C15-7-F_1_1"',
}

for path in paths:
    text = path.read_text(encoding='utf-8')
    before = text
    for old, new in replacements.items():
        text = text.replace(old, new)
    if text == before:
        raise RuntimeError(f'No BZX84 unit-prefix replacements made in {path}')
    path.write_text(text, encoding='utf-8')

# KiCad requires nested unit symbol names to use the parent symbol name as prefix.
for path in paths:
    text = path.read_text(encoding='utf-8')
    assert '"BZX84C15_0_1"' not in text
    assert '"BZX84C15_1_1"' not in text
    assert '"BZX84C15-7-F_0_1"' in text
    assert '"BZX84C15-7-F_1_1"' in text

print('BZX84 symbol unit-prefix repair: PASS')
