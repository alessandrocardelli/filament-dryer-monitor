#!/usr/bin/env python3
"""Repair KiCad symbol fields after a row-shifted Symbol Fields Table paste.

The script uses the main branch as the baseline *by Reference*, preserving the current
variant's symbol geometry, wiring and library IDs. It then overlays the approved Mouser
MPNs/manufacturers and the intentionally changed footprints.
"""

from __future__ import annotations

import csv
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HW = ROOT / "hardware"
FILES = ["Power.kicad_sch", "MCU.kicad_sch", "IO_Power.kicad_sch"]

RESET_FIELDS = (
    "Value", "Footprint", "Datasheet", "Description", "Function", "MPN",
    "LCSC", "Manufacturer", "Voltage", "Tolerance", "Dielectric", "Power",
    "TCR", "Mouser Part Number",
)

# Approved final sourcing data. Value is only overridden when the old value would name
# a different actual part; passive Value fields remain functional values (22u, 10k, etc.).
FINAL_GROUPS = [
    ("BZ1", "CSS-I4B20-SMT-TR", "Same Sky"),
    ("C6,C8,C11", "CC0805MKX5R8BB226", "YAGEO"),
    ("C12", "CL10A475KO8NNNC", "Samsung Electro-Mechanics"),
    ("C3,C9,C13,C14,C18,C19", "CC0603KRX7R9BB104", "YAGEO"),
    ("C2,C15,C17", "GCM188R71E105KA64J", "Murata Electronics"),
    ("C16", "CC0805KRX5R8BB106", "YAGEO"),
    ("C4", "CL10C470JC8NNNC", "Samsung Electro-Mechanics"),
    ("C5", "EEE-FTH101XAP", "Panasonic"),
    ("C7", "CL32B106KMVNNWE", "Samsung Electro-Mechanics"),
    ("D1", "BZX84C15-7-F", "Diodes Incorporated"),
    ("D2", "SMBJ28A", "Littelfuse"),
    ("D3", "HSMG-C170", "Broadcom"),
    ("D4,D5", "PMEG6020ER-QX", "Nexperia"),
    ("D6", "1N4148W-7-F", "Diodes Incorporated"),
    ("F1", "SF-1206S300W-2", "Bourns"),
    ("FB1", "BLM31KN601SH1L", "Murata Electronics"),
    ("J1", "B3B-XH-A(LF)(SN)", "JST"),
    ("J2", "USB4216-03-A", "GCT"),
    ("J3", "B4B-PH-SM4-TB(LF)(SN)", "JST"),
    ("J5,J7", "Kit HALJIA XH-compatible 2P", "HALJIA"),
    ("J6", "B2B-PH-SM4-TB(LF)(SN)", "JST"),
    ("L1", "XAL4040-822MEC", "Coilcraft"),
    ("Q1", "DMP6180SK3-13", "Diodes Incorporated"),
    ("Q2,Q3,Q6", "MMBT3904-7-F", "Diodes Incorporated"),
    ("Q4", "IRLR3636TRPBF", "Infineon"),
    ("Q5", "DMN6140L-13", "Diodes Incorporated"),
    ("R1,R3,R5,R21,R26", "RC0603FR-07100KL", "YAGEO"),
    ("R10,R11", "RC0603FR-075K1L", "YAGEO"),
    ("R12,R13,R14,R15,R19,R22,R23,R24", "RC0603FR-0710KL", "YAGEO"),
    ("R16,R17", "RC0603FR-0733RL", "YAGEO"),
    ("R18", "RC0603FR-07330RL", "YAGEO"),
    ("R20,R25", "CR0603-FX-1000ELF", "Bourns"),
    ("R27", "RC0603FR-07470RL", "YAGEO"),
    ("R28", "CRCW060347K0FKEA", "Vishay"),
    ("R6", "RC0603FR-1331K6L", "YAGEO"),
    ("R7", "RC0603FR-1322K1L", "YAGEO"),
    ("R8", "RC0603FR-0747K5L", "YAGEO"),
    ("R9", "RC0603FR-071KL", "YAGEO"),
    ("SW1,SW2,SW3,SW4,SW5,SW6", "SWT0110-020010SSA", "GCT"),
    ("U1", "AP66200QFVBW-13", "Diodes Incorporated"),
    ("U2", "CP2102N-A02-GQFN28R", "Silicon Labs"),
    ("U3", "82400102", "Würth Elektronik"),
    ("U4", "ESP32-WROOM-32E-N4", "Espressif Systems"),
]

FINAL = {}
for refs, mpn, manufacturer in FINAL_GROUPS:
    for ref in refs.split(","):
        FINAL[ref.strip()] = {"MPN": mpn, "Manufacturer": manufacturer}

SPECIAL = {
    "BZ1": {
        "Value": "CSS-I4B20-SMT-TR",
        "Footprint": "SamacSys_Parts:CSSI4B20SMTTR",
        "Datasheet": "https://www.sameskydevices.com/product/resource/css-i4b20-smt-tr.pdf",
        "Description": "8.5 mm surface-mount magnetic audio transducer buzzer",
    },
    "C5": {
        "Footprint": "SamacSys_Parts:EEE0JA331XP",
        "Voltage": "50V", "Tolerance": "20%", "Dielectric": "Aluminum Electrolytic",
    },
    "J2": {
        "Value": "USB4216-03-A",
        "Footprint": "SamacSys_Parts:USB421603A",
        "Datasheet": "https://eu.mouser.com/datasheet/2/837/usb4216-3577180.pdf",
        "Description": "USB-C receptacle, USB 2.0, 16-pin, horizontal SMT",
        "Mouser Part Number": "640-USB4216-03-A",
    },
    "J5": {"Footprint": "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"},
    "J7": {"Footprint": "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"},
    "L1": {
        "Footprint": "FilamentDryer:XAL4040-822MEC_HandSolder",
        "Datasheet": "https://www.coilcraft.com/pdfs/xal4000.pdf",
        "Description": "Coilcraft XAL4040 shielded power inductor, 8.2 uH",
    },
    "Q5": {"Value": "DMN6140L-13"},
    "U1": {"Value": "AP66200QFVBW-13"},
    "U3": {"Value": "82400102", "Datasheet": ""},
}
for n in range(1, 7):
    SPECIAL[f"SW{n}"] = {
        "Footprint": "SamacSys_Parts:SWT0110020010SSA",
        "Datasheet": "https://gct.co/files/drawings/swt0110.pdf",
        "Description": "GCT SWT0110 tactile switch, SPST-NO, SMT, 2.0 mm actuator height",
    }

CAP_SPECS = {
    **{r: {"Voltage": "25V", "Tolerance": "20%", "Dielectric": "X5R"} for r in ("C6", "C8", "C11")},
    "C16": {"Voltage": "25V", "Tolerance": "10%", "Dielectric": "X5R"},
    **{r: {"Voltage": "25V", "Tolerance": "10%", "Dielectric": "X7R"} for r in ("C2", "C15", "C17")},
    "C4": {"Voltage": "100V", "Tolerance": "5%", "Dielectric": "C0G"},
    "C7": {"Voltage": "63V", "Tolerance": "10%", "Dielectric": "X7R"},
    "C12": {"Voltage": "16V", "Tolerance": "10%", "Dielectric": "X5R"},
    **{r: {"Voltage": "50V", "Tolerance": "10%", "Dielectric": "X7R"} for r in ("C3", "C9", "C13", "C14", "C18", "C19")},
}

# Old Value strings that name a different selected component should not survive.
SPECIAL["FB1"] = {"Value": "BLM31KN601SH1L"}

PROP_RE = re.compile(r'\(property\s+"((?:\\.|[^"\\])*)"\s+"((?:\\.|[^"\\])*)"')
REF_RE = re.compile(r'\(property\s+"Reference"\s+"([^"\\]+)"')


def matching_paren(text: str, start: int) -> int:
    depth = 0
    in_string = False
    escaped = False
    for i in range(start, len(text)):
        ch = text[i]
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch == '(':
            depth += 1
        elif ch == ')':
            depth -= 1
            if depth == 0:
                return i + 1
    raise ValueError(f"Unbalanced s-expression at offset {start}")


def symbol_spans(text: str):
    pos = 0
    while True:
        start = text.find("(symbol", pos)
        if start < 0:
            return
        after = start + len("(symbol")
        if after < len(text) and not text[after].isspace():
            pos = after
            continue
        end = matching_paren(text, start)
        block = text[start:end]
        # Placed symbol instances contain lib_id. Embedded library definitions do not.
        if re.search(r'\(lib_id\s+"', block):
            m = REF_RE.search(block)
            if m:
                yield start, end, m.group(1), block
        pos = end


def props(block: str) -> dict[str, str]:
    return {m.group(1): m.group(2) for m in PROP_RE.finditer(block)}


def escape_value(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def set_prop(block: str, name: str, value: str, *, required: bool = False) -> tuple[str, bool]:
    pat = re.compile(r'(\(property\s+"' + re.escape(name) + r'"\s+")((?:\\.|[^"\\])*)(")')
    repl_value = escape_value(value)
    new, n = pat.subn(lambda m: m.group(1) + repl_value + m.group(3), block, count=1)
    if required and n != 1:
        raise RuntimeError(f"Missing required property {name!r}")
    return new, n == 1


def baseline_text(path: Path) -> str:
    rel = path.relative_to(ROOT).as_posix()
    proc = subprocess.run(
        ["git", "show", f"origin/main:{rel}"],
        cwd=ROOT, check=True, capture_output=True, text=True,
    )
    return proc.stdout


def map_props(text: str) -> dict[str, dict[str, str]]:
    out = {}
    for _, _, ref, block in symbol_spans(text):
        if ref in out:
            raise RuntimeError(f"Duplicate placed symbol reference {ref}")
        out[ref] = props(block)
    return out


def repair_file(path: Path, report: list[str]) -> None:
    current = path.read_text(encoding="utf-8")
    base = baseline_text(path)
    base_map = map_props(base)
    current_refs = {ref for _, _, ref, _ in symbol_spans(current)}

    missing_base = sorted(current_refs - set(base_map))
    if missing_base:
        raise RuntimeError(f"{path.name}: refs absent from main baseline: {missing_base}")

    replacements = []
    for start, end, ref, block in symbol_spans(current):
        before = props(block)
        new_block = block
        bprops = base_map[ref]

        # First undo the row-shift corruption using the same reference on main.
        for field in RESET_FIELDS:
            if field in before:
                new_block, _ = set_prop(new_block, field, bprops.get(field, ""))

        # Then overlay the approved final sourcing fields.
        if ref in FINAL:
            for field, value in FINAL[ref].items():
                new_block, _ = set_prop(new_block, field, value, required=True)
            # Do not leave an LCSC code pointing at a different MPN.
            old_mpn = bprops.get("MPN", "")
            if FINAL[ref]["MPN"] != old_mpn and "LCSC" in props(new_block):
                new_block, _ = set_prop(new_block, "LCSC", "")

        for field, value in CAP_SPECS.get(ref, {}).items():
            new_block, _ = set_prop(new_block, field, value, required=True)

        for field, value in SPECIAL.get(ref, {}).items():
            # All standard fields must exist. Mouser Part Number is optional except J2,
            # where the imported symbol already contains it.
            req = field in {"Value", "Footprint", "Datasheet", "Description"} or (ref == "J2" and field == "Mouser Part Number")
            new_block, found = set_prop(new_block, field, value, required=req)
            if not found and value:
                report.append(f"WARN {path.name} {ref}: property {field!r} absent; override not inserted")

        after = props(new_block)
        if ref in FINAL:
            assert after.get("MPN") == FINAL[ref]["MPN"], (ref, after.get("MPN"))
            assert after.get("Manufacturer") == FINAL[ref]["Manufacturer"], (ref, after.get("Manufacturer"))
        if ref in SPECIAL and "Footprint" in SPECIAL[ref]:
            assert after.get("Footprint") == SPECIAL[ref]["Footprint"], (ref, after.get("Footprint"))

        changed = [f for f in RESET_FIELDS if before.get(f, "") != after.get(f, "")]
        if changed:
            report.append(f"{path.name} {ref}: " + ", ".join(changed))
        replacements.append((start, end, new_block))

    for start, end, new_block in reversed(replacements):
        current = current[:start] + new_block + current[end:]
    path.write_text(current, encoding="utf-8", newline="\n")


def collect_repaired_props() -> dict[str, dict[str, str]]:
    all_props = {}
    for name in FILES:
        data = map_props((HW / name).read_text(encoding="utf-8"))
        overlap = set(all_props) & set(data)
        if overlap:
            raise RuntimeError(f"Duplicate refs across sheets: {sorted(overlap)}")
        all_props.update(data)
    return all_props


def update_csv(all_props: dict[str, dict[str, str]], report: list[str]) -> None:
    path = HW / "Filament_Dryer_Monitor.csv"
    if not path.exists():
        return
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        fieldnames = reader.fieldnames
    if not fieldnames or "Reference" not in fieldnames:
        raise RuntimeError("Unexpected Filament_Dryer_Monitor.csv format")

    col_to_prop = {
        "Value": "Value", "Footprint": "Footprint", "Datasheet": "Datasheet",
        "Descrizione": "Description", "Description": "Description", "MPN": "MPN",
        "LCSC": "LCSC", "Manufacturer": "Manufacturer", "Function": "Function",
        "Voltage": "Voltage", "Tolerance": "Tolerance", "Dielectric": "Dielectric",
        "Power": "Power", "Mouser Part Number": "Mouser Part Number",
    }
    for row in rows:
        ref = row.get("Reference", "").strip()
        if ref not in all_props:
            report.append(f"WARN CSV {ref}: reference not found in repaired schematics")
            continue
        p = all_props[ref]
        for col, prop_name in col_to_prop.items():
            if col in row:
                row[col] = p.get(prop_name, "")

    with path.open("w", encoding="utf-8-sig", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, quoting=csv.QUOTE_ALL, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def verify(all_props: dict[str, dict[str, str]], report: list[str]) -> None:
    missing = sorted(set(FINAL) - set(all_props))
    if missing:
        raise RuntimeError(f"Final BOM references missing from schematics: {missing}")

    errors = []
    for ref, expected in FINAL.items():
        actual = all_props[ref]
        for field in ("MPN", "Manufacturer"):
            if actual.get(field) != expected[field]:
                errors.append(f"{ref} {field}: {actual.get(field)!r} != {expected[field]!r}")
    for ref, expected in SPECIAL.items():
        if ref not in all_props:
            continue
        for field, value in expected.items():
            if field in all_props[ref] and all_props[ref].get(field) != value:
                errors.append(f"{ref} {field}: {all_props[ref].get(field)!r} != {value!r}")
    if errors:
        raise RuntimeError("Verification failed:\n" + "\n".join(errors))

    report.append("")
    report.append(f"Verified final MPN/manufacturer pairs: {len(FINAL)} references")
    report.append("PCB file intentionally untouched.")
    report.append("Generated netlist/ERC/DRC/production BOM intentionally not regenerated (KiCad CLI unavailable in this repair job).")


def main() -> None:
    report = [
        "Mouser symbol-field repair",
        "==========================",
        "Baseline: origin/main, matched strictly by Reference",
        "Current variant geometry/wiring/library IDs preserved",
        "",
    ]
    for name in FILES:
        repair_file(HW / name, report)
    all_props = collect_repaired_props()
    update_csv(all_props, report)
    verify(all_props, report)

    out = HW / "review" / "mouser_field_repair_report.txt"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(report) + "\n", encoding="utf-8")
    print("\n".join(report[-8:]))


if __name__ == "__main__":
    main()
