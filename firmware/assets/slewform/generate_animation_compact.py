#!/usr/bin/env python3
"""Rebuild the compact 128x64 Slewform boot animation from Slewform_logo.svg.

Dependencies: pip install cairosvg pillow numpy
Usage: python firmware/assets/slewform/generate_animation_compact.py

The first eight SVG Geometria children form the fixed crown; the other 13
form the roots. The second SVG group defines the separate SLEWFORM wordmark.
The 11 growth steps are applied at runtime by the firmware, not stored in flash.
"""
from __future__ import annotations
import copy
import io
import math
import re
import xml.etree.ElementTree as ET
from pathlib import Path

import cairosvg
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent
SVG = ROOT / 'Slewform_logo.svg'
NS = '{http://www.w3.org/2000/svg}'
THRESHOLD = 90
SYMBOL_SIZE = (54, 60)
WORDMARK_SIZE = (122, 18)
STEPS = 11


def layers():
    source = ET.fromstring(SVG.read_text(encoding='utf-8'))
    emblem = next(g for g in source if g.get('id') == 'Geometria')
    wordmark = next(g for g in source if g.get('id') == 'Livello_3')
    assert len(emblem) == 21 and len(wordmark) == 12, 'SVG artwork changed; revisit layer split'

    def render(elems):
        doc = copy.deepcopy(source)
        for g in list(doc):
            if g.tag == NS + 'g':
                doc.remove(g)
        group = ET.SubElement(doc, NS + 'g')
        for e in elems:
            group.append(copy.deepcopy(e))
        xml = re.sub(r'#[0-9a-fA-F]{6}\b', '#ffffff', ET.tostring(doc, encoding='unicode'))
        png = cairosvg.svg2png(bytestring=xml.encode('utf-8'), output_width=1536, output_height=1024)
        return Image.open(io.BytesIO(png)).convert('RGBA').getchannel('A')

    return render(list(emblem)[:8]), render(list(emblem)[8:]), render(list(wordmark))


def mono(im, size):
    return im.resize(size, Image.Resampling.LANCZOS).point(
        lambda v: 255 if v >= THRESHOLD else 0).convert('L')


def pack(im):
    w, h = im.size
    a = np.asarray(im) > 0
    stride = (w + 7)//8
    packed = bytes(sum(int(a[y, xb*8+bit]) << bit
                       for bit in range(8) if xb*8+bit < w)
                   for y in range(h) for xb in range(stride))
    assert len(packed) == stride*h
    assert all(bool(packed[y*stride+x//8] & (1 << (x & 7))) == bool(a[y, x])
               for y in range(h) for x in range(w)), 'XBM round trip mismatch'
    return packed


def emit(name, image):
    data = pack(image)
    lines = ['  '+', '.join(f'0x{b:02X}' for b in data[i:i+16])+','
             for i in range(0, len(data), 16)]
    lines[-1] = lines[-1].removesuffix(',')
    return f'static const uint8_t {name}[{len(data)}] PROGMEM = {{\n'+ '\n'.join(lines)+'\n};\n\n'


def main():
    upper, roots, word = layers()
    union = Image.fromarray(np.maximum(np.asarray(upper), np.asarray(roots)).astype('uint8'))
    bounds = union.getbbox()
    assert bounds, 'Symbol empty'
    upper = mono(upper.crop(bounds), SYMBOL_SIZE)
    roots = mono(roots.crop(bounds), SYMBOL_SIZE)
    word = mono(word.crop(word.getbbox()), WORDMARK_SIZE)
    w, h = SYMBOL_SIZE
    origin_y = (437 - bounds[1])*h/(bounds[3]-bounds[1])
    yy, xx = np.ogrid[:h, :w]
    distances = np.sqrt((xx-(w-1)/2)**2 + (yy-origin_y)**2)
    rmax = float(distances[np.asarray(roots) > 0].max())
    radii = []
    for i in range(STEPS):
        p = i/(STEPS-1)
        r = 2+(rmax-2)*(1-(1-p)**1.35)
        radii.append(math.ceil(4*r*r))
    assert max(radii) <= 65535 and all(a < b for a,b in zip(radii,radii[1:]))

    out = f'''#pragma once
#include <Arduino.h>

// Generated from canonical Slewform_logo.svg by generate_animation_compact.py.
// 1-bit, byte-padded XBM LSB-first. Upper emblem remains fixed; root mask
// grows in firmware by radius. Wordmark is a separate full-screen phase.
#define SLEWFORM_SYMBOL_WIDTH {w}
#define SLEWFORM_SYMBOL_HEIGHT {h}
#define SLEWFORM_SYMBOL_X {(128-w)//2}
#define SLEWFORM_SYMBOL_Y {(64-h)//2}
#define SLEWFORM_ROOT_ORIGIN_X2 {w-1}
#define SLEWFORM_ROOT_ORIGIN_Y2 {round(2*origin_y)}
#define SLEWFORM_WORDMARK_WIDTH {word.width}
#define SLEWFORM_WORDMARK_HEIGHT {word.height}
#define SLEWFORM_WORDMARK_X {(128-word.width)//2}
#define SLEWFORM_WORDMARK_Y {(64-word.height)//2}
#define SLEWFORM_ROOT_STEP_MS 150UL
#define SLEWFORM_ROOT_STEPS {STEPS}

'''
    out += emit('slewform_symbol_upper', upper)
    out += emit('slewform_symbol_roots', roots)
    out += emit('slewform_wordmark', word)
    out += 'static const uint16_t slewform_root_radius_sq_x4[SLEWFORM_ROOT_STEPS] = {' + ', '.join(map(str,radii)) + '};\n'
    path = ROOT / 'slewform_animation_compact.h'
    path.write_text(out, encoding='utf-8')
    print(f'Wrote {path}; 420 + 420 + 288 = 1128 bitmap bytes')


if __name__ == '__main__':
    main()
