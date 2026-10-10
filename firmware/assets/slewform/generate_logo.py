#!/usr/bin/env python3
"""Regenerate the Slewform 90x60 monochrome XBM splash from canonical SVG.

Requirements: pip install cairosvg pillow
Usage: python firmware/assets/slewform/generate_logo.py
       python firmware/assets/slewform/generate_logo.py --select strong
"""
from __future__ import annotations

import argparse
import io
import re
from pathlib import Path

import cairosvg
from PIL import Image, ImageDraw

WIDTH, HEIGHT = 90, 60
OLED_WIDTH, OLED_HEIGHT = 128, 64
THRESHOLDS = {"light": 165, "balanced": 115, "strong": 72}


def rasterize(svg: Path) -> Image.Image:
    source = svg.read_text(encoding="utf-8")
    # Mask of ALL source geometry, regardless of original orange/blue colors.
    # A luminance conversion of those colors would incorrectly lose orange lines.
    mono_source = re.sub(r"#[0-9a-fA-F]{6}\b", "#ffffff", source)
    png = cairosvg.svg2png(
        bytestring=mono_source.encode("utf-8"),
        output_width=1536,
        output_height=1024,
    )
    alpha = Image.open(io.BytesIO(png)).convert("RGBA").getchannel("A")
    box = alpha.getbbox()
    if box is None:
        raise ValueError("Empty SVG alpha mask")
    # Artwork has approximately 1.5 aspect ratio. Leave a 1-pixel border.
    return alpha.crop(box).resize((WIDTH - 2, HEIGHT - 2), Image.Resampling.LANCZOS)


def logo_from_alpha(alpha: Image.Image, threshold: int) -> Image.Image:
    mask = alpha.point(lambda value: 255 if value >= threshold else 0)
    logo = Image.new("L", (WIDTH, HEIGHT), 0)
    logo.paste(mask, (1, 1))
    return logo


def pack_xbm(logo: Image.Image) -> bytes:
    # drawXBMP expects row-major, each row padded to ceil(width/8), LSB-first.
    pixels = logo.load()
    packed = bytearray()
    for y in range(HEIGHT):
        for x_byte in range((WIDTH + 7) // 8):
            value = 0
            for bit in range(8):
                x = x_byte * 8 + bit
                if x < WIDTH and pixels[x, y] >= 128:
                    value |= 1 << bit
            packed.append(value)
    if len(packed) != 720:
        raise AssertionError("Invalid XBM length")
    return bytes(packed)


def unpack_xbm(data: bytes) -> Image.Image:
    if len(data) != 720:
        raise ValueError("Invalid XBM byte count")
    image = Image.new("L", (WIDTH, HEIGHT), 0)
    pixels = image.load()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            if data[y * 12 + x // 8] & (1 << (x % 8)):
                pixels[x, y] = 255
    return image


def write_header(path: Path, packed: bytes) -> None:
    lines = [
        "  " + ", ".join(f"0x{value:02X}" for value in packed[i:i + 12]) + ","
        for i in range(0, len(packed), 12)
    ]
    lines[-1] = lines[-1][:-1]
    header = (
        "#pragma once\n#include <Arduino.h>\n\n"
        "// Generated from Slewform_logo.svg by generate_logo.py. Do not edit manually.\n"
        "// Monochrome 90x60 XBM, LSB-first per byte, row-major with row padding.\n"
        "// Display at x=19, y=2 using U8g2 drawXBMP() on the 128x64 OLED.\n\n"
        f"#define SLEWFORM_FULL_LOGO_WIDTH {WIDTH}\n"
        f"#define SLEWFORM_FULL_LOGO_HEIGHT {HEIGHT}\n\n"
        "static const unsigned char slewform_full_logo[] PROGMEM = {\n"
        + "\n".join(lines) + "\n};\n"
    )
    path.write_text(header, encoding="utf-8")


def oled_frame(logo: Image.Image) -> Image.Image:
    frame = Image.new("L", (OLED_WIDTH, OLED_HEIGHT), 0)
    frame.paste(logo, ((OLED_WIDTH - WIDTH) // 2, (OLED_HEIGHT - HEIGHT) // 2))
    return frame


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    folder = Path(__file__).resolve().parent
    parser.add_argument("--svg", type=Path, default=folder / "Slewform_logo.svg")
    parser.add_argument("--output-dir", type=Path, default=folder)
    parser.add_argument("--select", choices=list(THRESHOLDS), default="balanced")
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    alpha = rasterize(args.svg)
    candidates = {}
    for label, threshold in THRESHOLDS.items():
        logo = logo_from_alpha(alpha, threshold)
        packed = pack_xbm(logo)
        assert unpack_xbm(packed).tobytes() == logo.tobytes(), "XBM round-trip failed"
        candidates[label] = (logo, packed)
        oled_frame(logo).save(args.output_dir / f"slewform_logo_{label}_128x64.png")

    selected_logo, selected_data = candidates[args.select]
    write_header(args.output_dir / "slewform_logo_128x64.h", selected_data)
    oled_frame(selected_logo).resize(
        (OLED_WIDTH * 6, OLED_HEIGHT * 6), Image.Resampling.NEAREST
    ).save(args.output_dir / "slewform_logo_selected_6x.png")

    panel_width, panel_height = OLED_WIDTH * 5, OLED_HEIGHT * 5
    comparison = Image.new("RGB", (panel_width * 3, panel_height + 32), (13, 17, 24))
    draw = ImageDraw.Draw(comparison)
    for col, (label, (logo, packed)) in enumerate(candidates.items()):
        preview = oled_frame(logo).resize(
            (panel_width, panel_height), Image.Resampling.NEAREST
        ).convert("RGB")
        comparison.paste(preview, (col * panel_width, 32))
        title = f"{label.upper()}   threshold={THRESHOLDS[label]}"
        if label == args.select:
            title += "   [SELECTED]"
        draw.text((col * panel_width + 10, 10), title, fill="white")
        print(f"{label}: {len(packed)} XBM bytes")

    comparison.save(args.output_dir / "slewform_logo_comparison.png")
    print(f"Selected: {args.select}; generated header and previews in {args.output_dir}")


if __name__ == "__main__":
    main()
