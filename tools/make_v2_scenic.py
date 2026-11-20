#!/usr/bin/env python3
"""Generates the decorative (non-gameplay) dinosaur silhouettes for the V2 scenic director: Content/Images/HD/v2_scenic.png/.plist.
Silhouette art on purpose: distant, low-contrast, cool-toned shapes (PRODUCT_PLAN 21.5) that read as background and can never be
mistaken for a hazard. Register with:  python3 tools/v2_registry.py register-atlas v2_scenic.plist --id v2_scenic_art \
--group scenic_v2 --optional --role scenic --fps 2 --apply
"""
import os, sys
from PIL import Image, ImageDraw, ImageFilter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_v2_atlas import pack, write_plist  # noqa: E402

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Content", "Images", "HD")
S = 2  # supersample


def canvas(w, h):
    return Image.new("RGBA", (w * S, h * S), (0, 0, 0, 0))


def finish(im, w, h):
    im = im.resize((w, h), Image.LANCZOS)
    # cool gradient: lighter toward the top (atmospheric), darker body, soft edge
    px = im.load()
    for y in range(h):
        t = y / max(1, h - 1)
        base = (int(112 - 36 * t), int(138 - 40 * t), int(172 - 46 * t))
        for x in range(w):
            a = px[x, y][3]
            if a:
                px[x, y] = (*base, a)
    return im.filter(ImageFilter.GaussianBlur(0.6))


def poly(d, pts, fill=(255, 255, 255, 255)):
    d.polygon([(x * S, y * S) for x, y in pts], fill=fill)


def walker(frame):
    w, h = 512, 256
    im = canvas(w, h)
    d = ImageDraw.Draw(im)
    # body, long neck + small head, tail
    poly(d, [(150, 150), (200, 118), (290, 112), (360, 130), (380, 160), (330, 178), (210, 182)])
    poly(d, [(190, 128), (160, 80), (130, 40), (112, 22), (96, 22), (88, 32), (100, 40), (122, 66), (140, 112), (170, 152)])
    poly(d, [(96, 22), (80, 28), (82, 38), (98, 36)])
    poly(d, [(360, 140), (420, 150), (480, 176), (500, 192), (470, 186), (400, 170), (360, 168)])
    sw = 10 if frame == 0 else -10
    for lx, dx in ((196, sw), (236, -sw), (312, sw), (352, -sw)):   # four columnar legs, alternating
        poly(d, [(lx, 172), (lx + 22, 172), (lx + 18 + dx, 248), (lx + 2 + dx, 248)])
    return finish(im, w, h)


def peek():
    w, h = 320, 320
    im = canvas(w, h)
    d = ImageDraw.Draw(im)
    # T-Rex head and thick neck, side view, looking left
    poly(d, [(60, 130), (110, 95), (190, 86), (240, 100), (262, 150), (250, 320), (110, 320), (112, 200), (86, 190), (50, 176), (38, 150)])
    poly(d, [(40, 150), (62, 172), (100, 190), (120, 178), (98, 160)], (0, 0, 0, 0))   # mouth line (cut-out)
    for x in range(66, 110, 11):                                                        # teeth
        poly(d, [(x, 178), (x + 5, 178), (x + 2, 190)])
    im = finish(im, w, h)
    d2 = ImageDraw.Draw(im)
    d2.ellipse((122, 112, 134, 124), fill=(190, 210, 225, 230))                          # pale eye: low contrast, no warning colours
    return im


def main():
    sp = [("scenic_walker_0", walker(0)), ("scenic_walker_1", walker(1)), ("scenic_peek_0", peek())]
    sheet, frames = pack(sp, size=1024)
    os.makedirs(OUT, exist_ok=True)
    sheet.save(os.path.join(OUT, "v2_scenic.png"), optimize=True)
    write_plist(os.path.join(OUT, "v2_scenic.plist"), frames, 1024, "v2_scenic.png")
    print(len(frames), "frames ->", os.path.join(OUT, "v2_scenic.png"))


if __name__ == "__main__":
    main()
