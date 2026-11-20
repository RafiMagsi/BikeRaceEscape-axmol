#!/usr/bin/env python3
"""Registry tooling for Content/CompleteGameInfo.plist `v2.presentation` (PRODUCT_PLAN section 23.6).

  validate-content                      check every atlas / animation / dialogue cross-reference; exit 1 on any error
  register-atlas ATLAS_PLIST --id ID    add or update an atlas entry and derive one animation per `<name>_<N>` frame series
                  [--group G] [--optional] [--fps N] [--role R]
  contact-sheet ATLAS_ID OUT.png        labelled sheet of every frame of a registered atlas (needs Pillow)

Write commands are dry-run by default: they print the exact change and only touch the plist with --apply
(then `plutil -lint` and validate-content run). Pure standard library except contact-sheet.
"""
import argparse, os, plistlib, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLIST = os.path.join(ROOT, "Content", "CompleteGameInfo.plist")
SEARCH = [os.path.join(ROOT, "Content", "Images", "HD"), os.path.join(ROOT, "Content", "Images"), os.path.join(ROOT, "Content")]


def find(name):
    for d in SEARCH:
        p = os.path.join(d, name)
        if os.path.exists(p):
            return p
    return None


def load():
    with open(PLIST, "rb") as f:
        return plistlib.load(f)


def pres(root):
    return root.setdefault("v2", {}).setdefault("presentation", {})


def atlas_frames(plist_name):
    p = find(plist_name)
    if not p:
        return None, None
    with open(p, "rb") as f:
        d = plistlib.load(f)
    return set(d.get("frames", {}).keys()), d.get("metadata", {}).get("textureFileName")


def validate(root):
    errs = []
    v2 = root.get("v2", {})
    if int(v2.get("contentVersion", 0)) < 1:
        errs.append("v2.contentVersion must be >= 1")
    p = v2.get("presentation")
    if not p:
        return ["v2.presentation is missing"]
    if int(p.get("schemaVersion", 0)) < 1:
        errs.append("v2.presentation.schemaVersion must be >= 1")
    frames_of, owner, ids = {}, {}, set()
    for a in p.get("atlases", []):
        aid = a.get("id", "")
        if not aid:
            errs.append("an atlas has no id"); continue
        if aid in ids:
            errs.append(f"duplicate atlas id '{aid}'")
        ids.add(aid)
        fr, tex = atlas_frames(a.get("plist", ""))
        if fr is None:
            errs.append(f"atlas '{aid}': plist not found: {a.get('plist')}"); continue
        if not find(a.get("texture", "")):
            errs.append(f"atlas '{aid}': texture not found: {a.get('texture')}")
        if tex and tex != a.get("texture"):
            errs.append(f"atlas '{aid}': plist names texture '{tex}', registry says '{a.get('texture')}'")
        frames_of[aid] = fr
        if a.get("required", True):
            for f in fr:
                if f in owner:
                    errs.append(f"frame '{f}' is defined by two required atlases ('{owner[f]}', '{aid}')")
                owner[f] = aid
    for name, d in p.get("animations", {}).items():
        aid = d.get("atlasId")
        if aid not in ids:
            errs.append(f"animation '{name}': unknown atlasId '{aid}'"); continue
        if not d.get("frames"):
            errs.append(f"animation '{name}' has no frames")
        if not (1 <= float(d.get("fps", 8)) <= 60):
            errs.append(f"animation '{name}': unsafe fps {d.get('fps')}")
        for f in d.get("frames", []):
            if f not in frames_of.get(aid, set()):
                errs.append(f"animation '{name}': frame '{f}' is not in atlas '{aid}'")
    for name, d in p.get("dialogues", {}).items():
        lines = d.get("lines", [])
        if not lines:
            errs.append(f"dialogue '{name}' has no lines")
        for l in lines:
            if not l:
                errs.append(f"dialogue '{name}' has an empty line")
            elif len(l) > 40:
                errs.append(f"dialogue '{name}': line over 40 characters: {l}")
    for sid, ps in p.get("portraitSets", {}).items():
        for expr in ("neutral", "confident", "surprised", "frustrated"):
            if ps.get(expr) not in p.get("animations", {}):
                errs.append(f"portrait set '{sid}': expression '{expr}' has no registered animation")
    for name, q in p.get("dialogueSequences", {}).items():
        speaker_ids = {s.get("id") for s in q.get("speakers", [])}
        if not q.get("lines"):
            errs.append(f"dialogue sequence '{name}' has no lines")
        if q.get("oncePolicy") not in ("once", "session", "daily", "always"):
            errs.append(f"dialogue sequence '{name}': oncePolicy must be once/session/daily/always")
        for s in q.get("speakers", []):
            if s.get("portraitSet") not in p.get("portraitSets", {}):
                errs.append(f"dialogue sequence '{name}': unknown portraitSet '{s.get('portraitSet')}'")
        for l in q.get("lines", []):
            if l.get("speaker") not in speaker_ids:
                errs.append(f"dialogue sequence '{name}': unknown speaker '{l.get('speaker')}'")
            if l.get("expression") not in ("neutral", "confident", "surprised", "frustrated"):
                errs.append(f"dialogue sequence '{name}': unknown expression '{l.get('expression')}'")
            if not l.get("text") or len(l["text"]) > 40:
                errs.append(f"dialogue sequence '{name}': line empty or over 40 chars: {l.get('text')}")
    layers = {"BackgroundSky", "BackgroundFar", "BackgroundScenic", "BackgroundMid"}
    for name, b in p.get("scenicBeats", {}).items():
        if b.get("animationId") not in p.get("animations", {}):
            errs.append(f"scenic beat '{name}': unknown animation '{b.get('animationId')}'")
        if b.get("layer") not in layers:
            errs.append(f"scenic beat '{name}': layer '{b.get('layer')}' must be a background layer {sorted(layers)}")
        if b.get("collisionPolicy", "none") != "none":
            errs.append(f"scenic beat '{name}': scenic content must not have a collision policy")
        if float(b.get("cooldownMin", 0)) > float(b.get("cooldownMax", 0)) or float(b.get("duration", 0)) <= 0:
            errs.append(f"scenic beat '{name}': invalid cooldown/duration")
    for g, d in p.get("contentGroups", {}).items():
        for a in d.get("atlases", []):
            if a not in ids:
                errs.append(f"content group '{g}': unknown atlas '{a}'")
        for a in d.get("animations", []):
            if a not in p.get("animations", {}):
                errs.append(f"content group '{g}': unknown animation '{a}'")
    return errs


def cmd_validate(_):
    errs = validate(load())
    for e in errs:
        print("ERROR", e)
    print("validate-content:", "FAIL (%d errors)" % len(errs) if errs else "OK")
    return 1 if errs else 0


def cmd_register(a):
    frames, tex = atlas_frames(a.atlas_plist)
    if frames is None:
        print("atlas plist not found:", a.atlas_plist); return 1
    texture = tex or a.atlas_plist.replace(".plist", ".png")
    series = {}
    for f in sorted(frames):
        m = re.match(r"^(.*)_(\d+)$", f)
        if m:
            series.setdefault(m.group(1), []).append((int(m.group(2)), f))
    root = load()
    p = pres(root)
    p.setdefault("schemaVersion", 1)
    atlases = [x for x in p.setdefault("atlases", []) if x.get("id") != a.id]
    atlases.append({"id": a.id, "plist": a.atlas_plist, "texture": texture, "required": not a.optional, "preloadGroup": a.group})
    p["atlases"] = atlases
    anims = p.setdefault("animations", {})
    added = []
    for name, fr in sorted(series.items()):
        if name in anims and anims[name].get("atlasId") != a.id:
            print(f"SKIP '{name}': already registered by atlas '{anims[name].get('atlasId')}'"); continue
        entry = anims.get(name, {})
        entry.update({"atlasId": a.id, "frames": [f for _, f in sorted(fr)], "fps": entry.get("fps", a.fps), "loop": entry.get("loop", True),
                      "role": entry.get("role", a.role)})
        anims[name] = entry
        added.append(name)
    print(f"atlas '{a.id}' -> {a.atlas_plist} / {texture} (group {a.group}, required={not a.optional})")
    print("animations:", ", ".join(f"{n}[{len(anims[n]['frames'])}]" for n in added))
    if not a.apply:
        print("dry run: nothing written (pass --apply)"); return 0
    with open(PLIST, "wb") as f:
        plistlib.dump(root, f, fmt=plistlib.FMT_XML, sort_keys=False)
    subprocess.run(["plutil", "-lint", PLIST], check=False)
    return cmd_validate(a)


def cmd_sheet(a):
    from PIL import Image, ImageDraw
    root = load()
    atl = next((x for x in pres(root).get("atlases", []) if x["id"] == a.atlas_id), None)
    if not atl:
        print("unknown atlas"); return 1
    with open(find(atl["plist"]), "rb") as f:
        d = plistlib.load(f)
    sheet = Image.open(find(atl["texture"])).convert("RGBA")
    cells = []
    for name, fr in sorted(d["frames"].items()):
        nums = [int(x) for x in re.findall(r"-?\d+", fr.get("frame", fr.get("textureRect", "")))]
        x, y, w, h = nums[:4]
        if fr.get("textureRotated") or fr.get("rotated"):
            w, h = h, w
        cells.append((name, sheet.crop((x, y, x + w, y + h))))
    cw = max(c.width for _, c in cells) + 8
    ch = max(c.height for _, c in cells) + 22
    cols = 6
    rows = (len(cells) + cols - 1) // cols
    out = Image.new("RGBA", (cols * cw, rows * ch), (28, 30, 44, 255))
    dr = ImageDraw.Draw(out)
    for i, (name, c) in enumerate(cells):
        out.alpha_composite(c, ((i % cols) * cw + 4, (i // cols) * ch + 4))
        dr.text(((i % cols) * cw + 4, (i // cols) * ch + c.height + 6), name, fill=(255, 220, 120, 255))
    out.save(a.out)
    print("wrote", a.out, len(cells), "frames")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("validate-content").set_defaults(fn=cmd_validate)
    r = sub.add_parser("register-atlas")
    r.add_argument("atlas_plist"); r.add_argument("--id", required=True); r.add_argument("--group", default="gameplay_v2")
    r.add_argument("--optional", action="store_true"); r.add_argument("--fps", type=float, default=10.0)
    r.add_argument("--role", default="cosmetic"); r.add_argument("--apply", action="store_true")
    r.set_defaults(fn=cmd_register)
    c = sub.add_parser("contact-sheet"); c.add_argument("atlas_id"); c.add_argument("out"); c.set_defaults(fn=cmd_sheet)
    a = ap.parse_args()
    sys.exit(a.fn(a))


if __name__ == "__main__":
    main()
