#!/usr/bin/env python3
"""Generate store graphics (screenshots, feature graphic, icon) from the game's own atlas art.

Usage: python3 tools/make_store_art.py
Output: store/ios, store/android, store/icon
"""
import os
import plistlib
import re
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HD = os.path.join(ROOT, "Content", "Images", "HD")
OUT = os.path.join(ROOT, "store")
FONT = "/System/Library/Fonts/Supplemental/Arial Black.ttf"

_sheets = {}
_frames = {}


def _load():
    for n in range(4):
        _sheets[n] = Image.open(os.path.join(HD, f"animation_s{n}.png")).convert("RGBA")
        d = plistlib.load(open(os.path.join(HD, f"animation_s{n}.plist"), "rb"))
        for k, v in d["frames"].items():
            x, y, w, h = map(int, re.findall(r"-?\d+", v["frame"]))
            _frames[k] = (n, (x, y, x + w, y + h))


def sprite(name):
    n, box = _frames[name]
    return _sheets[n].crop(box)


def shadow(img, blur=10, opacity=140, offset=(0, 8)):
    pad = blur * 3
    a = img.split()[3]
    sh = Image.new("RGBA", (img.width + pad * 2, img.height + pad * 2), (0, 0, 0, 0))
    black = Image.new("RGBA", img.size, (0, 0, 0, opacity))
    sh.paste(black, (pad + offset[0], pad + offset[1]), a)
    return sh.filter(ImageFilter.GaussianBlur(blur)), pad


def put(canvas, img, cx, base_y, height, flip=False, rot=0.0, glow=None):
    """Paste sprite centered on cx with its bottom at base_y, scaled to `height` px."""
    s = height / img.height
    im = img.resize((max(1, int(img.width * s)), height), Image.LANCZOS)
    if flip:
        im = im.transpose(Image.FLIP_LEFT_RIGHT)
    if rot:
        im = im.rotate(rot, resample=Image.BICUBIC, expand=True)
    if glow:
        pad_g = 60
        padded = Image.new("RGBA", (im.width + pad_g * 2, im.height + pad_g * 2), (0, 0, 0, 0))
        padded.paste(im, (pad_g, pad_g))
        a = padded.split()[3].filter(ImageFilter.GaussianBlur(18)).point(lambda v: min(255, v * 2))
        gl = Image.new("RGBA", padded.size, glow + (0,))
        gl.putalpha(a)
        canvas.alpha_composite(gl, (int(cx - gl.width / 2), int(base_y - im.height - pad_g)))
    sh, pad = shadow(im, blur=max(6, height // 30), offset=(0, height // 25))
    x = int(cx - im.width / 2)
    y = int(base_y - im.height)
    canvas.alpha_composite(sh, (x - pad, y - pad))
    canvas.alpha_composite(im, (x, y))


def cover(img, w, h, anchor_y=0.55):
    s = max(w / img.width, h / img.height)
    im = img.resize((int(img.width * s) + 1, int(img.height * s) + 1), Image.LANCZOS)
    left = (im.width - w) // 2
    top = int((im.height - h) * anchor_y)
    return im.crop((left, top, left + w, top + h))


def background(w, h, dim=0.0):
    bg = cover(sprite("InterfaceMM_0_0"), w, h, anchor_y=0.75).convert("RGBA")
    if dim:
        bg = Image.alpha_composite(bg, Image.new("RGBA", bg.size, (0, 0, 10, int(255 * dim))))
    # vignette
    v = Image.new("L", (w, h), 0)
    d = ImageDraw.Draw(v)
    d.ellipse((-w * 0.25, -h * 0.35, w * 1.25, h * 1.35), fill=255)
    v = v.filter(ImageFilter.GaussianBlur(min(w, h) // 5))
    dark = Image.new("RGBA", (w, h), (0, 0, 12, 190))
    dark.putalpha(Image.eval(v, lambda p: 190 - int(p * 190 / 255)))
    return Image.alpha_composite(bg, dark)


def headline(canvas, lines, x, y, size, anchor="la", colors=((255, 214, 64), (255, 110, 20))):
    f = ImageFont.truetype(FONT, size)
    d = ImageDraw.Draw(canvas)
    lh = int(size * 1.08)
    for i, line in enumerate(lines):
        yy = y + i * lh
        # gradient fill via mask
        bbox = d.textbbox((x, yy), line, font=f, anchor=anchor)
        w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
        mask = Image.new("L", canvas.size, 0)
        ImageDraw.Draw(mask).text((x, yy), line, font=f, fill=255, anchor=anchor)
        grad = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
        gd = ImageDraw.Draw(grad)
        top, bot = bbox[1], bbox[3]
        for row in range(top, bot + 1):
            t = (row - top) / max(1, bot - top)
            c = tuple(int(colors[0][k] * (1 - t) + colors[1][k] * t) for k in range(3))
            gd.line([(bbox[0] - 4, row), (bbox[2] + 4, row)], fill=c + (255,))
        # outline + drop shadow
        stroke = max(4, size // 14)
        sh = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
        ImageDraw.Draw(sh).text((x + stroke, yy + stroke * 1.4), line, font=f, fill=(0, 0, 0, 200), anchor=anchor,
                                stroke_width=stroke, stroke_fill=(0, 0, 0, 200))
        canvas.alpha_composite(sh.filter(ImageFilter.GaussianBlur(stroke // 2)))
        ol = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
        ImageDraw.Draw(ol).text((x, yy), line, font=f, fill=(40, 10, 0, 255), anchor=anchor,
                                stroke_width=stroke, stroke_fill=(40, 10, 0, 255))
        canvas.alpha_composite(ol)
        fill = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
        fill.paste(grad, (0, 0), mask)
        canvas.alpha_composite(fill)


SCENES = {
    "escape": dict(
        title=["ESCAPE THE", "DINOSAURS!"],
        sprites=lambda c, W, H: [
            ("Obstacles_1_4", 0.22, 0.86, 0.34, True, 0, (255, 90, 20)),
            ("Characters_Jump_0_2", 0.60, 0.76, 0.36, False, 12, (255, 60, 40)),
            ("Coins_0_0", 0.78, 0.50, 0.10, False, 0, (255, 220, 60)),
            ("Coins_0_0", 0.86, 0.44, 0.10, False, 0, (255, 220, 60)),
            ("Coins_0_0", 0.94, 0.38, 0.10, False, 0, (255, 220, 60)),
        ]),
    "jump": dict(
        title=["JUMP HUGE", "RAMPS!"],
        sprites=lambda c, W, H: [
            ("Obstacles_5_0", 0.50, 0.90, 0.22, False, 0, None),
            ("Characters_Jump_1_1", 0.70, 0.55, 0.36, False, 14, (80, 140, 255)),
            ("Obstacles_0_1", 0.24, 0.88, 0.17, False, 0, None),
        ]),
    "coins": dict(
        title=["COLLECT", "COINS!"],
        sprites=lambda c, W, H: [
            ("Characters_0_1", 0.50, 0.86, 0.34, False, 0, (255, 60, 40)),
            ("Coins_0_0", 0.66, 0.52, 0.11, False, 0, (255, 220, 60)),
            ("Coins_0_0", 0.76, 0.46, 0.11, False, 0, (255, 220, 60)),
            ("Coins_0_0", 0.86, 0.46, 0.11, False, 0, (255, 220, 60)),
            ("Coins_0_0", 0.96, 0.52, 0.11, False, 0, (255, 220, 60)),
            ("Obstacles_3_0", 0.20, 0.84, 0.27, False, 0, None),
        ]),
    "bikes": dict(
        title=["UNLOCK NEW", "BIKES!"],
        sprites=lambda c, W, H: [
            ("Characters_0_0", 0.52, 0.88, 0.30, False, 0, (255, 60, 40)),
            ("Characters_1_1", 0.74, 0.74, 0.28, False, 0, (60, 120, 255)),
            ("Characters_2_0", 0.96, 0.62, 0.26, False, 0, (200, 220, 255)),
        ]),
    "sewer": dict(
        title=["SURVIVE THE", "SEWER RUN!"],
        sprites=lambda c, W, H: [
            ("Obstacles_2_0", 0.20, 0.88, 0.17, True, 0, None),
            ("Obstacles_0_2", 0.40, 0.88, 0.2, False, 0, None),
            ("Characters_0_2", 0.72, 0.86, 0.34, False, 0, (255, 60, 40)),
        ]),
}


def make_scene(key, W, H):
    sc = SCENES[key]
    canvas = background(W, H, dim=0.12)
    # sprites first, headline on top-left
    for name, cx, by, hh, flip, rot, glow in sc["sprites"](canvas, W, H):
        put(canvas, sprite(name), W * cx, H * by, int(H * hh), flip, rot, glow)
    portrait_bias = W / H < 1.6
    size = int(H * (0.105 if portrait_bias else 0.125))
    headline(canvas, sc["title"], int(W * 0.05), int(H * 0.06), size)
    return canvas.convert("RGB")


def logo_on(canvas, cx, cy, height):
    lg = Image.open(os.path.join(HD, "gameLogo.png")).convert("RGBA")
    bb = lg.getbbox()
    lg = lg.crop(bb)
    s = height / lg.height
    lg = lg.resize((int(lg.width * s), height), Image.LANCZOS)
    sh, pad = shadow(lg, blur=14, offset=(0, 10))
    x, y = int(cx - lg.width / 2), int(cy - lg.height / 2)
    canvas.alpha_composite(sh, (x - pad, y - pad))
    canvas.alpha_composite(lg, (x, y))


def feature_graphic(W=1024, H=500):
    canvas = background(W, H, dim=0.18)
    put(canvas, sprite("Obstacles_1_5"), W * 0.52, H * 0.93, int(H * 0.46), True, 0, (255, 90, 20))
    put(canvas, sprite("Characters_Jump_0_2"), W * 0.84, H * 0.80, int(H * 0.50), False, 10, (255, 60, 40))
    put(canvas, sprite("Coins_0_0"), W * 0.60, H * 0.40, int(H * 0.11), False, 0, (255, 220, 60))
    logo_on(canvas, W * 0.29, H * 0.50, int(H * 0.60))
    return canvas.convert("RGB")


def hero_banner(W, H):
    canvas = background(W, H, dim=0.15)
    put(canvas, sprite("Obstacles_1_5"), W * 0.60, H * 0.93, int(H * 0.44), True, 0, (255, 90, 20))
    put(canvas, sprite("Characters_Jump_0_2"), W * 0.80, H * 0.78, int(H * 0.48), False, 10, (255, 60, 40))
    logo_on(canvas, W * 0.30, H * 0.50, int(H * 0.58))
    return canvas.convert("RGB")


def icon(size=1024):
    W = H = size
    bg = cover(sprite("InterfaceMM_0_0"), W, H, anchor_y=0.4).convert("RGBA")
    bg = Image.alpha_composite(bg, Image.new("RGBA", (W, H), (10, 0, 30, 70)))
    # warm glow under the action
    glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    ImageDraw.Draw(glow).ellipse((W * 0.1, H * 0.45, W * 0.95, H * 1.1), fill=(255, 110, 20, 120))
    bg = Image.alpha_composite(bg, glow.filter(ImageFilter.GaussianBlur(70)))
    put(bg, sprite("Obstacles_1_5"), W * 0.30, H * 0.95, int(H * 0.66), True, 0, (255, 90, 20))
    put(bg, sprite("Characters_Jump_0_2"), W * 0.66, H * 0.72, int(H * 0.60), False, 12, (255, 60, 40))
    return bg.convert("RGB")


def save(img, path, quality=None):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if quality:
        img.save(path, "JPEG", quality=quality)
    else:
        img.save(path, "PNG", optimize=True)
    print("wrote", os.path.relpath(path, ROOT), img.size)


def main():
    _load()
    keys = list(SCENES)
    # iOS: iPhone 6.9" landscape, iPad 13" landscape. (App Store forbids alpha -> RGB PNG.)
    for i, k in enumerate(keys, 1):
        save(make_scene(k, 2868, 1320), f"{OUT}/ios/iphone_6.9/{i:02d}_{k}.png")
        save(make_scene(k, 2622, 1206), f"{OUT}/ios/iphone_6.3/{i:02d}_{k}.png")
        save(make_scene(k, 2752, 2064), f"{OUT}/ios/ipad_13/{i:02d}_{k}.png")
        # Google Play phone + tablets (16:9, 24-bit PNG/JPEG, no alpha)
        save(make_scene(k, 1920, 1080), f"{OUT}/android/phone/{i:02d}_{k}.png")
        save(make_scene(k, 2560, 1440), f"{OUT}/android/tablet_10/{i:02d}_{k}.png")
        save(make_scene(k, 1920, 1080), f"{OUT}/android/tablet_7/{i:02d}_{k}.png")
    save(feature_graphic(), f"{OUT}/android/feature_graphic_1024x500.png")
    save(hero_banner(1920, 1080), f"{OUT}/ios/app_store_header_1920x1080.png")
    save(hero_banner(3840, 1646), f"{OUT}/ios/app_store_header_3840x1646.png")
    ic = icon(1024)
    save(ic, f"{OUT}/icon/icon_1024.png")
    save(ic.resize((512, 512), Image.LANCZOS), f"{OUT}/icon/play_icon_512.png")


if __name__ == "__main__":
    sys.exit(main())
