#!/usr/bin/env python3
"""Compose App Store screenshots (background + caption + real simulator screenshot).

Usage: python3 tools/make_store_screens.py <raw_iphone_dir> <raw_ipad_dir>
Raw dirs hold simulator screenshots named 01_menu.png, 02_gameplay.png, 05_jetpack.png, 04_gadgets.png, 03_shop.png,
06_missions.png (portrait framebuffers straight from `simctl io screenshot`; they are rotated here).
Output: store/ios/iphone_6.9 (2868x1320) and store/ios/ipad_13 (2752x2064).
"""
import os, sys, math, random
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONT = os.path.join(ROOT, "Content", "Fonts", "LilitaOne-Regular.ttf")
SHOTS = [("01_menu", "RIDE THE SEWER RUN", "A fast one-tap bike runner"),
         ("02_gameplay", "JUMP, DODGE & COLLECT COINS", "Tap to jump over gaps and obstacles"),
         ("05_jetpack", "GRAB POWER-UPS ON THE FLY", "Magnet, jetpack, turbo, shield and more"),
         ("04_gadgets", "USE GADGETS ANYTIME", "Tap the lightning button in a run"),
         ("03_shop", "UPGRADE YOUR POWER-UPS", "Stock up and ride further"),
         ("06_missions", "DAILY MISSIONS & FREE GIFTS", "Complete missions, claim rewards")]
TARGETS = {"iphone_6.9": ((2868, 1320), 0.20), "ipad_13": ((2752, 2064), 0.17)}


def background(size, seed):
    w, h = size
    rnd = random.Random(seed)
    top, bot = (9, 14, 38), (30, 52, 112)
    img = Image.new("RGB", size)
    px = ImageDraw.Draw(img)
    for y in range(h):
        t = y / h
        px.line([(0, y), (w, y)], fill=tuple(int(top[i] + (bot[i] - top[i]) * t) for i in range(3)))
    glow = Image.new("RGB", size, (0, 0, 0))
    g = ImageDraw.Draw(glow)
    g.ellipse([-w * 0.25, h * 0.45, w * 0.55, h * 1.5], fill=(255, 120, 20))
    g.ellipse([w * 0.55, -h * 0.5, w * 1.25, h * 0.45], fill=(60, 110, 255))
    glow = glow.filter(ImageFilter.GaussianBlur(h * 0.18))
    img = Image.blend(img, Image.composite(glow, img, Image.new("L", size, 255)), 0.38)
    # light rays
    rays = Image.new("RGBA", size, (0, 0, 0, 0))
    r = ImageDraw.Draw(rays)
    for i in range(7):
        x = rnd.uniform(0, w)
        wid = rnd.uniform(w * 0.02, w * 0.06)
        r.polygon([(x, 0), (x + wid, 0), (x + wid * 3 - h * 0.4, h), (x - wid * 2 - h * 0.4, h)], fill=(255, 220, 150, 16))
    img = Image.alpha_composite(img.convert("RGBA"), rays)
    # sparkles
    sp = ImageDraw.Draw(img)
    for _ in range(46):
        x, y, s = rnd.uniform(0, w), rnd.uniform(0, h), rnd.uniform(3, 9)
        sp.ellipse([x - s, y - s, x + s, y + s], fill=(255, 226, 140, rnd.randint(40, 120)))
    return img.convert("RGB")


def caption(img, title, sub, band_h):
    w, h = img.size
    d = ImageDraw.Draw(img)
    f1 = ImageFont.truetype(FONT, int(band_h * 0.44))
    f2 = ImageFont.truetype(FONT, int(band_h * 0.20))
    tb = d.textbbox((0, 0), title, font=f1, stroke_width=int(band_h * 0.03))
    tx = (w - (tb[2] - tb[0])) // 2
    ty = int(band_h * 0.10)
    d.text((tx + 6, ty + 8), title, font=f1, fill=(0, 0, 0), stroke_width=int(band_h * 0.03), stroke_fill=(0, 0, 0))
    d.text((tx, ty), title, font=f1, fill=(255, 222, 80), stroke_width=int(band_h * 0.03), stroke_fill=(150, 60, 0))
    sb = d.textbbox((0, 0), sub, font=f2)
    d.text(((w - (sb[2] - sb[0])) // 2, ty + int(band_h * 0.56)), sub, font=f2, fill=(230, 238, 255))


def compose(raw, out, size, band_frac, title, sub, seed):
    w, h = size
    img = background(size, seed)
    band = int(h * (0.20 if w > h * 1.5 else 0.17))
    margin = int(h * 0.035)
    shot = Image.open(raw).rotate(90, expand=True).convert("RGB")
    avail_h = h - band - margin * 2
    sc = min(avail_h / shot.height, (w - margin * 2) / shot.width)
    shot = shot.resize((int(shot.width * sc), int(shot.height * sc)), Image.LANCZOS)
    x, y = (w - shot.width) // 2, band + margin // 2
    rad = int(shot.height * 0.035)
    mask = Image.new("L", shot.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, shot.width - 1, shot.height - 1], rad, fill=255)
    shadow = Image.new("RGBA", size, (0, 0, 0, 0))
    ImageDraw.Draw(shadow).rounded_rectangle([x, y + 14, x + shot.width, y + shot.height + 14], rad, fill=(0, 0, 0, 170))
    shadow = shadow.filter(ImageFilter.GaussianBlur(24))
    img = Image.alpha_composite(img.convert("RGBA"), shadow).convert("RGB")
    frame = Image.new("RGB", (shot.width + 12, shot.height + 12), (255, 190, 60))
    fm = Image.new("L", frame.size, 0)
    ImageDraw.Draw(fm).rounded_rectangle([0, 0, frame.width - 1, frame.height - 1], rad + 6, fill=255)
    img.paste(frame, (x - 6, y - 6), fm)
    img.paste(shot, (x, y), mask)
    caption(img, title, sub, band)
    img.save(out, "PNG", optimize=True)


def main():
    raw_dirs = {"iphone_6.9": sys.argv[1], "ipad_13": sys.argv[2]}
    for name, (size, frac) in TARGETS.items():
        out_dir = os.path.join(ROOT, "store", "ios", name)
        os.makedirs(out_dir, exist_ok=True)
        for f in os.listdir(out_dir):
            os.remove(os.path.join(out_dir, f))
        for i, (key, title, sub) in enumerate(SHOTS, 1):
            out = os.path.join(out_dir, f"{i:02d}_{key.split('_', 1)[1]}.png")
            compose(os.path.join(raw_dirs[name], key + ".png"), out, size, frac, title, sub, i)
            print(out, Image.open(out).size)


if __name__ == "__main__":
    main()
