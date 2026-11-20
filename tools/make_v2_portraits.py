#!/usr/bin/env python3
"""Generates PLACEHOLDER dialogue portraits (waist-up helmet busts, four expressions each) into Content/Images/HD/v2_portraits.png/.plist.
Flat placeholder art: the production portraits must be painted in the main-game style (PRODUCT_PLAN 21.6 / 21.8). Register with:
  python3 tools/v2_registry.py register-atlas v2_portraits.plist --id v2_portrait_art --group dialogue_v2 --optional --role portrait --fps 1 --apply
"""
import os, sys
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_v2_atlas import pack, write_plist  # noqa: E402

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Content", "Images", "HD")
W, H, S = 256, 320, 2
INK = (18, 14, 22, 255)


def bust(jacket, trim, expr):
    im = Image.new("RGBA", (W * S, H * S), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    P = lambda pts: [(x * S, y * S) for x, y in pts]
    # shoulders / jacket
    d.polygon(P([(14, 320), (30, 236), (88, 208), (168, 208), (226, 236), (242, 320)]), fill=jacket, outline=INK)
    d.polygon(P([(100, 212), (156, 212), (142, 268), (114, 268)]), fill=trim, outline=INK)
    d.rectangle(P([(108, 186), (148, 214)]), fill=(70, 54, 50, 255), outline=INK)
    # helmet
    d.ellipse(P([(52, 40), (204, 204)]), fill=jacket, outline=INK, width=3 * S)
    d.pieslice(P([(52, 40), (204, 204)]), 180, 360, fill=trim, outline=INK)
    # visor
    d.rounded_rectangle(P([(70, 98), (190, 152)]), radius=18 * S, fill=(24, 30, 44, 255), outline=INK, width=3 * S)
    ey = {"neutral": (0, 0), "confident": (0, -2), "surprised": (0, 0), "frustrated": (0, 2)}[expr]
    for ex in (106, 150):
        r = 9 if expr == "surprised" else 7
        d.ellipse(P([(ex - r, 126 - r + ey[1]), (ex + r, 126 + r + ey[1])]), fill=(225, 238, 250, 255))
    # brows
    slant = {"neutral": 0, "confident": -5, "surprised": 0, "frustrated": 6}[expr]
    by = 108 if expr != "surprised" else 100
    d.line(P([(94, by + slant), (122, by - slant)]), fill=(230, 230, 235, 255), width=4 * S)
    d.line(P([(134, by - slant), (162, by + slant)]), fill=(230, 230, 235, 255), width=4 * S)
    # mouth under the visor (chin area)
    my = 176
    if expr == "confident":
        d.arc(P([(108, my - 14), (148, my + 10)]), 20, 160, fill=INK, width=3 * S)
    elif expr == "surprised":
        d.ellipse(P([(120, my - 4), (136, my + 12)]), fill=INK)
    elif expr == "frustrated":
        d.arc(P([(108, my + 2), (148, my + 22)]), 200, 340, fill=INK, width=3 * S)
    else:
        d.line(P([(112, my + 4), (144, my + 4)]), fill=INK, width=3 * S)
    return im.resize((W, H), Image.LANCZOS)


def main():
    sets = {"player": ((196, 48, 44, 255), (236, 196, 70, 255)), "cool_kid": ((52, 112, 214, 255), (240, 240, 245, 255))}
    sp = []
    for name, (jacket, trim) in sets.items():
        for expr in ("neutral", "confident", "surprised", "frustrated"):
            sp.append((f"portrait_{name}_{expr}_0", bust(jacket, trim, expr)))
    sheet, frames = pack(sp, size=1024)
    os.makedirs(OUT, exist_ok=True)
    sheet.save(os.path.join(OUT, "v2_portraits.png"), optimize=True)
    write_plist(os.path.join(OUT, "v2_portraits.plist"), frames, 1024, "v2_portraits.png")
    print(len(frames), "frames")


if __name__ == "__main__":
    main()
