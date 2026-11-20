#!/usr/bin/env python3
"""Draws the Version 2 road-event sprites and packs them into one atlas (cocos2d plist format 2).

Usage: python3 tools/make_v2_atlas.py
Output: Content/Images/HD/v2_events.png + v2_events.plist   (loaded with SpriteFrameCache, see Source/V2/V2Art.cpp)

Every sprite is drawn at 4x and downsampled (clean edges), faces LEFT unless noted, and has an animation cycle where
it moves. Frame names: <name>_<index>. Sizes are in design units (the game runs at 640 units high).
"""
import math
import os
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "Content", "Images", "HD")
SS = 4  # supersample

INK = (38, 24, 18, 255)
GREEN = (96, 178, 74, 255)
GREEN_D = (62, 132, 56, 255)
GREEN_L = (150, 214, 110, 255)
BELLY = (214, 226, 150, 255)
RED = (214, 58, 52, 255)
ORANGE = (244, 150, 40, 255)
YELLOW = (252, 214, 60, 255)
STEEL = (170, 176, 188, 255)
STEEL_D = (96, 102, 116, 255)
WHITE = (250, 248, 240, 255)
BROWN = (170, 110, 60, 255)
BROWN_D = (120, 74, 38, 255)


class Canvas:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.im = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
        self.d = ImageDraw.Draw(self.im)

    def P(self, x, y):
        return (x * SS, y * SS)

    def ell(self, cx, cy, rx, ry, fill, out=INK, ow=2.0):
        self.d.ellipse([(cx - rx) * SS, (cy - ry) * SS, (cx + rx) * SS, (cy + ry) * SS], fill=fill, outline=out, width=int(ow * SS) if out else 0)

    def poly(self, pts, fill, out=INK, ow=2.0):
        pp = [self.P(x, y) for x, y in pts]
        self.d.polygon(pp, fill=fill)
        if out:
            self.d.line(pp + [pp[0]], fill=out, width=int(ow * SS), joint="curve")

    def line(self, pts, fill=INK, w=2.0):
        self.d.line([self.P(x, y) for x, y in pts], fill=fill, width=int(w * SS), joint="curve")

    def rect(self, x0, y0, x1, y1, fill, out=INK, ow=2.0, r=0):
        box = [x0 * SS, y0 * SS, x1 * SS, y1 * SS]
        if r:
            self.d.rounded_rectangle(box, r * SS, fill=fill, outline=out, width=int(ow * SS) if out else 0)
        else:
            self.d.rectangle(box, fill=fill, outline=out, width=int(ow * SS) if out else 0)

    def done(self):
        return self.im.resize((self.w, self.h), Image.LANCZOS)


def wheel(c, cx, cy, r, ang):
    c.ell(cx, cy, r, r, (36, 36, 42, 255), INK, 1.6)
    c.ell(cx, cy, r * 0.62, r * 0.62, STEEL, INK, 1.2)
    for k in range(5):
        a = ang + k * 2 * math.pi / 5
        c.line([(cx, cy), (cx + math.cos(a) * r * 0.6, cy + math.sin(a) * r * 0.6)], STEEL_D, 1.2)
    c.ell(cx, cy, r * 0.16, r * 0.16, YELLOW, INK, 1.0)


# ------------------------------------------------------------------------------------------------ dinosaurs (face left)
def dino(frame, scale=1.0, angry=False, body=GREEN, shade=GREEN_D):
    W, H = int(86 * scale), int(78 * scale)
    c = Canvas(W, H)
    s = scale
    t = frame * math.pi / 2
    bob = math.sin(t * 2) * 1.6 * s
    # tail
    c.poly([(60 * s, 40 * s + bob), (84 * s, 28 * s + bob), (82 * s, 40 * s + bob), (62 * s, 54 * s + bob)], body)
    # far legs
    lf = math.sin(t * 2) * 7 * s
    c.poly([(40 * s, 52 * s), (48 * s, 52 * s), (46 * s + lf, 72 * s), (36 * s + lf, 72 * s)], shade)
    c.poly([(36 * s + lf, 72 * s), (46 * s + lf, 72 * s), (46 * s + lf, 77 * s), (30 * s + lf, 77 * s)], shade)
    # body
    c.ell(46 * s, 42 * s + bob, 22 * s, 17 * s, body)
    c.ell(42 * s, 48 * s + bob, 15 * s, 9 * s, BELLY, None)
    # back spikes
    for i in range(5):
        x = 34 * s + i * 8 * s
        y = 26 * s + bob + abs(i - 2) * 1.5 * s
        c.poly([(x - 4 * s, y + 3 * s), (x, y - 7 * s), (x + 4 * s, y + 3 * s)], ORANGE if not angry else RED, INK, 1.4)
    # head
    c.ell(24 * s, 28 * s + bob, 15 * s, 12 * s, body)
    c.poly([(10 * s, 28 * s + bob), (2 * s, 32 * s + bob), (3 * s, 38 * s + bob), (16 * s, 37 * s + bob)], body)
    c.line([(4 * s, 35 * s + bob), (14 * s, 35 * s + bob)], INK, 1.4)
    c.poly([(7 * s, 35 * s + bob), (9 * s, 40 * s + bob), (11 * s, 35 * s + bob)], WHITE, INK, 1.0)   # tooth
    c.ell(20 * s, 24 * s + bob, 5 * s, 5.4 * s, WHITE, INK, 1.4)
    c.ell(18.6 * s, 24.6 * s + bob, 2.4 * s, 2.6 * s, INK, None)
    c.line([(14 * s, 17 * s + bob), (25 * s, 20.5 * s + bob)] if angry else [(15 * s, 18.5 * s + bob), (25 * s, 18.5 * s + bob)], INK, 2.0)
    # near legs
    lb = -lf
    c.poly([(54 * s, 52 * s), (62 * s, 52 * s), (60 * s + lb, 72 * s), (50 * s + lb, 72 * s)], body)
    c.poly([(50 * s + lb, 72 * s), (60 * s + lb, 72 * s), (60 * s + lb, 77 * s), (44 * s + lb, 77 * s)], body)
    # tiny arm
    c.poly([(30 * s, 44 * s + bob), (22 * s, 48 * s + bob), (24 * s, 51 * s + bob), (33 * s, 49 * s + bob)], body, INK, 1.4)
    return c.done()


# ------------------------------------------------------------------------------------------------ riders (face RIGHT)
def rider(frame, kind, hue=RED):
    W, H = 150, 112
    c = Canvas(W, H)
    t = frame * math.pi / 2
    bob = math.sin(t * 2) * 1.2
    ang = frame * 0.9
    # wheels
    wheel(c, 34, 88, 21, -ang)
    wheel(c, 118, 88, 21, -ang)
    # frame / fork / engine
    c.poly([(34, 88 + 0), (58, 56 + bob), (92, 56 + bob), (118, 88)], hue, INK, 2.4)
    c.poly([(58, 56 + bob), (78, 70), (96, 70), (92, 56 + bob)], STEEL, INK, 1.8)
    c.line([(104, 52 + bob), (118, 86)], STEEL_D, 4.5)
    c.line([(98, 50 + bob), (112, 42 + bob)], INK, 3.0)
    c.ell(60, 52 + bob, 14, 7, (60, 60, 70, 255), INK, 1.6)    # seat
    # dino rider (sits on seat, faces right)
    body = GREEN
    c.poly([(54, 52 + bob), (44, 40 + bob), (36, 52 + bob)], body, INK, 1.6)              # tail
    c.ell(66, 36 + bob, 15, 17, body)                                                   # torso
    c.ell(68, 40 + bob, 9, 11, BELLY, None)
    c.poly([(58, 48 + bob), (74, 48 + bob), (86, 60), (78, 64), (70, 56)], GREEN_D, INK, 1.6)   # leg
    c.line([(74, 26 + bob), (100, 44 + bob)], body, 6)                                    # arm to the bars
    c.ell(100, 44 + bob, 3.2, 3.2, GREEN_D, INK, 1.0)
    # head
    c.ell(76, 14 + bob, 13, 11, body)
    c.poly([(84, 12 + bob), (98, 15 + bob), (97, 21 + bob), (84, 22 + bob)], body)
    c.line([(86, 21 + bob), (97, 20 + bob)], INK, 1.4)
    if kind == "cool":
        c.rect(78, 8 + bob, 100, 15 + bob, (20, 20, 28, 255), INK, 1.2, 3)               # sunglasses
        c.line([(66, 10 + bob), (78, 11 + bob)], INK, 1.6)
        c.poly([(58, 6 + bob), (68, 0 + bob), (84, 0 + bob), (92, 6 + bob), (66, 10 + bob)], (40, 120, 220, 255), INK, 1.6)   # cap
    elif kind == "delivery":
        c.ell(84, 11 + bob, 4, 4.4, WHITE, INK, 1.2)
        c.ell(85.4, 11.4 + bob, 1.8, 2, INK, None)
        c.poly([(60, 6 + bob), (70, 0 + bob), (88, 0 + bob), (96, 7 + bob), (70, 9 + bob)], RED, INK, 1.6)                      # cap
        # pizza stack on the rack (wobbles)
        w = math.sin(t * 2) * 2.0
        for i in range(4):
            c.rect(14 + w * (i + 1) * 0.5, 64 - i * 11, 50 + w * (i + 1) * 0.5, 74 - i * 11, (236, 190, 110, 255), INK, 1.6, 2)
            c.line([(14 + w * (i + 1) * 0.5, 69 - i * 11), (50 + w * (i + 1) * 0.5, 69 - i * 11)], RED, 1.6)
    elif kind == "grandma":
        c.ell(84, 11 + bob, 4, 4.4, WHITE, INK, 1.2)
        c.ell(85.4, 11.4 + bob, 1.8, 2, INK, None)
        c.ell(80, -1 + bob, 9, 8, (170, 150, 200, 255), INK, 1.6)                       # hair bun
        c.ell(86, 11 + bob, 7.5, 6.5, None, INK, 1.4)                                    # glasses
        c.poly([(60, 6 + bob), (68, 2 + bob), (80, 2 + bob), (84, 8 + bob), (66, 10 + bob)], (236, 150, 190, 255), INK, 1.6)   # scarf
        # shopping basket on the rack with coins
        c.poly([(8, 66), (46, 66), (42, 84), (12, 84)], BROWN, INK, 1.8)
        for i in range(3):
            c.ell(16 + i * 10, 62 + math.sin(t * 2 + i) * 1.5, 5.5, 5.5, YELLOW, INK, 1.2)
    else:  # ambient
        c.ell(84, 11 + bob, 4, 4.4, WHITE, INK, 1.2)
        c.ell(85.4, 11.4 + bob, 1.8, 2, INK, None)
        c.poly([(62, 8 + bob), (70, -1 + bob), (86, -1 + bob), (92, 8 + bob)], hue, INK, 1.6)                                 # helmet
        c.line([(100, 44 + bob), (108, 30 + bob)], GREEN, 5) if frame % 2 else None                                            # waves
    return c.done()


# ------------------------------------------------------------------------------------------------ props
def sign():
    c = Canvas(76, 118)
    c.rect(35, 52, 41, 116, STEEL, INK, 2.0)
    c.poly([(38, 2), (74, 58), (2, 58)], YELLOW, INK, 3.0)
    c.poly([(38, 12), (66, 54), (10, 54)], (252, 230, 110, 255), None)
    # dino silhouette inside the triangle
    c.ell(36, 42, 11, 7, INK, None)
    c.ell(26, 33, 6, 5, INK, None)
    c.poly([(46, 42), (58, 38), (48, 46)], INK, None)
    c.poly([(30, 46), (34, 46), (33, 54), (29, 54)], INK, None)
    c.poly([(40, 46), (44, 46), (43, 54), (39, 54)], INK, None)
    return c.done()


def pizza_box():
    c = Canvas(48, 40)
    c.rect(3, 12, 45, 37, (236, 190, 110, 255), INK, 2.4, 3)
    c.poly([(3, 12), (45, 12), (40, 4), (8, 4)], (250, 216, 150, 255), INK, 2.0)
    c.ell(24, 24, 8, 6, RED, None)
    c.ell(21, 22, 2, 2, YELLOW, None)
    c.ell(27, 26, 2, 2, YELLOW, None)
    return c.done()


def impact_star():
    c = Canvas(64, 64)
    pts = []
    for i in range(16):
        r = 30 if i % 2 == 0 else 14
        a = i * math.pi / 8
        pts.append((32 + math.cos(a) * r, 32 + math.sin(a) * r))
    c.poly(pts, YELLOW, INK, 2.4)
    c.poly([(32 + (p[0] - 32) * 0.55, 32 + (p[1] - 32) * 0.55) for p in pts], (255, 246, 190, 255), None)
    return c.done()


def shadow():
    c = Canvas(64, 16)
    c.ell(32, 8, 30, 6, (0, 0, 0, 110), None)
    return c.done()



def sign_work():
    c = Canvas(76, 118)
    c.rect(35, 52, 41, 116, STEEL, INK, 2.0)
    c.poly([(38, 2), (74, 38), (38, 74), (2, 38)], ORANGE, INK, 3.0)
    c.poly([(38, 12), (64, 38), (38, 64), (12, 38)], (255, 190, 90, 255), None)
    c.rect(35, 20, 41, 44, INK, None)
    c.ell(38, 52, 3.6, 3.6, INK, None)
    return c.done()


def cone():
    c = Canvas(40, 56)
    c.rect(2, 48, 38, 55, (40, 40, 48, 255), INK, 1.6, 2)
    c.poly([(20, 2), (34, 48), (6, 48)], ORANGE, INK, 2.4)
    c.poly([(20, 14), (27, 32), (13, 32)], WHITE, None)
    c.line([(11, 40), (29, 40)], WHITE, 3.4)
    return c.done()


def barrel():
    c = Canvas(60, 60)
    c.ell(30, 30, 28, 28, (214, 70, 60, 255), INK, 2.6)
    c.ell(30, 30, 20, 20, (236, 110, 90, 255), INK, 1.6)
    c.ell(30, 30, 9, 9, (250, 214, 90, 255), INK, 1.6)
    for a in (0.5, 2.1, 3.7, 5.3):
        c.line([(30 + math.cos(a) * 11, 30 + math.sin(a) * 11), (30 + math.cos(a) * 27, 30 + math.sin(a) * 27)], INK, 2.0)
    return c.done()


def worker(frame):
    im = dino(frame, 0.95)
    c = Canvas(im.size[0] + 30, im.size[1] + 36)
    c.im.alpha_composite(im.resize((im.size[0] * SS, im.size[1] * SS), Image.LANCZOS), (30 * SS, 36 * SS))
    c.poly([(41, 56), (47, 43), (60, 43), (66, 56)], YELLOW, INK, 2.0)                     # hard hat
    c.line([(39, 56), (68, 56)], INK, 2.2)
    c.line([(18, 36), (18, 110)], STEEL_D, 4.0)                                  # sign pole
    c.poly([(2, 4), (34, 4), (34, 26), (2, 26)], RED, INK, 2.4)
    c.rect(7, 11, 29, 19, WHITE, None)
    return c.done()


def meteor(frame):
    c = Canvas(64, 96)
    f = frame * 0.7
    c.poly([(32, 2), (14, 56 + math.sin(f) * 4), (24, 78), (40, 78), (50, 56 + math.cos(f) * 4)], ORANGE, INK, 2.0)
    c.poly([(32, 14), (22, 52), (42, 52)], YELLOW, None)
    c.ell(32, 74, 20, 19, (110, 96, 92, 255), INK, 2.6)
    c.ell(25, 69, 6, 5, (150, 134, 128, 255), None)
    c.ell(38, 79, 4, 3.4, (84, 72, 70, 255), None)
    return c.done()


def rock():
    c = Canvas(66, 48)
    c.poly([(4, 46), (12, 18), (26, 6), (44, 8), (58, 22), (62, 46)], (118, 104, 98, 255), INK, 2.6)
    c.poly([(16, 44), (22, 24), (34, 14), (40, 30), (30, 44)], (150, 134, 128, 255), None)
    c.ell(34, 44, 24, 4, (255, 120, 40, 200), None)                              # ember glow at the base
    return c.done()


def coin_gold():
    c = Canvas(30, 30)
    c.ell(15, 15, 13, 13, YELLOW, INK, 2.2)
    c.ell(15, 15, 8, 8, (250, 232, 130, 255), (200, 140, 20, 255), 1.6)
    c.line([(15, 9), (15, 21)], (200, 140, 20, 255), 2.2)
    return c.done()


def flame(frame):
    c = Canvas(70, 40)
    k = 1.0 + 0.25 * math.sin(frame * 1.6)
    c.poly([(68, 20), (30 * k, 4), (4, 20), (30 * k, 36)], ORANGE, None)
    c.poly([(62, 20), (36 * k, 10), (16, 20), (36 * k, 30)], YELLOW, None)
    c.poly([(54, 20), (40, 15), (30, 20), (40, 25)], WHITE, None)
    return c.done()


# ------------------------------------------------------------------------------------------------ packing
def pack(sprites, size=1024, pad=2):
    sheet = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    x = y = row = 0
    frames = {}
    for name, im in sprites:
        w, h = im.size
        if x + w + pad > size:
            x, y, row = 0, y + row + pad, 0
        assert y + h <= size, "atlas full"
        sheet.alpha_composite(im, (x, y))
        frames[name] = (x, y, w, h)
        x += w + pad
        row = max(row, h)
    return sheet, frames


def write_plist(path, frames, size, texture):
    L = ['<?xml version="1.0" encoding="UTF-8"?>',
         '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">',
         '<plist version="1.0"><dict><key>frames</key><dict>']
    for n, (x, y, w, h) in sorted(frames.items()):
        L += [f"<key>{n}</key><dict><key>frame</key><string>{{{{{x},{y}}},{{{w},{h}}}}}</string>",
              "<key>offset</key><string>{0,0}</string><key>rotated</key><false/>",
              f"<key>sourceColorRect</key><string>{{{{0,0}},{{{w},{h}}}}}</string><key>sourceSize</key><string>{{{w},{h}}}</string></dict>"]
    L += ["</dict><key>metadata</key><dict><key>format</key><integer>2</integer>",
          f"<key>size</key><string>{{{size},{size}}}</string><key>textureFileName</key><string>{texture}</string>",
          "<key>realTextureFileName</key><string>%s</string></dict></dict></plist>" % texture]
    open(path, "w").write("\n".join(L))


def main():
    sp = []
    for f in range(4):
        sp.append((f"dino_small_{f}", dino(f, 0.9)))
    for f in range(4):
        sp.append((f"dino_big_{f}", dino(f, 1.55, angry=True, body=(176, 84, 62, 255), shade=(130, 56, 44, 255))))
    for f in range(4):
        sp.append((f"rider_cool_{f}", rider(f, "cool", (40, 120, 220, 255))))
    for f in range(4):
        sp.append((f"rider_delivery_{f}", rider(f, "delivery", (232, 70, 60, 255))))
    for f in range(4):
        sp.append((f"rider_ambient_a_{f}", rider(f, "ambient", (240, 170, 40, 255))))
    for f in range(4):
        sp.append((f"rider_ambient_b_{f}", rider(f, "ambient", (150, 90, 210, 255))))
    for f in range(4):
        sp.append((f"rider_grandma_{f}", rider(f, "grandma", (236, 120, 170, 255))))
    for f in range(4):
        sp.append((f"meteor_{f}", meteor(f)))
    for f in range(4):
        sp.append((f"flame_{f}", flame(f)))
    for f in range(2):
        sp.append((f"worker_{f}", worker(f)))
    sp += [("sign_work_0", sign_work()), ("cone_0", cone()), ("barrel_0", barrel()), ("rock_0", rock()), ("coin_gold_0", coin_gold())]
    sp += [("sign_dino_0", sign()), ("pizza_box_0", pizza_box()), ("impact_star_0", impact_star()), ("shadow_0", shadow())]
    sheet, frames = pack(sp)
    os.makedirs(OUT, exist_ok=True)
    sheet.save(os.path.join(OUT, "v2_events.png"), optimize=True)
    write_plist(os.path.join(OUT, "v2_events.plist"), frames, 1024, "v2_events.png")
    print(len(frames), "frames ->", os.path.join(OUT, "v2_events.png"))


if __name__ == "__main__":
    main()
