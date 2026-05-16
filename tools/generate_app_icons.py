#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


@dataclass(frozen=True)
class IOSIconSpec:
    size: int
    scale: int
    idiom: str

    @property
    def px(self) -> int:
        return self.size * self.scale

    @property
    def size_str(self) -> str:
        return f"{self.size}x{self.size}"

    @property
    def scale_str(self) -> str:
        return f"{self.scale}x"

    @property
    def filename(self) -> str:
        # Keep filenames deterministic and human-readable.
        if self.scale == 1:
            return f"AppIcon-{self.size}x{self.size}.png"
        return f"AppIcon-{self.size}x{self.size}@{self.scale}x.png"


IOS_SPECS: list[IOSIconSpec] = [
    # iPhone
    IOSIconSpec(20, 2, "iphone"),
    IOSIconSpec(20, 3, "iphone"),
    IOSIconSpec(29, 2, "iphone"),
    IOSIconSpec(29, 3, "iphone"),
    IOSIconSpec(40, 2, "iphone"),
    IOSIconSpec(40, 3, "iphone"),
    IOSIconSpec(60, 2, "iphone"),
    IOSIconSpec(60, 3, "iphone"),
    # iPad
    IOSIconSpec(20, 1, "ipad"),
    IOSIconSpec(20, 2, "ipad"),
    IOSIconSpec(29, 1, "ipad"),
    IOSIconSpec(29, 2, "ipad"),
    IOSIconSpec(40, 1, "ipad"),
    IOSIconSpec(40, 2, "ipad"),
    IOSIconSpec(76, 1, "ipad"),
    IOSIconSpec(76, 2, "ipad"),
    IOSIconSpec(83, 2, "ipad"),  # 83.5 rounded to 83 for legacy compatibility
    # App Store
    IOSIconSpec(1024, 1, "ios-marketing"),
]


ANDROID_DENSITIES: dict[str, int] = {
    "mipmap-mdpi": 48,
    "mipmap-hdpi": 72,
    "mipmap-xhdpi": 96,
    "mipmap-xxhdpi": 144,
    "mipmap-xxxhdpi": 192,
}


def _load_and_square(img_path: Path) -> Image.Image:
    img = Image.open(img_path).convert("RGBA")
    w, h = img.size
    if w == h:
        return img

    # Center-crop to square (keeps subject centered).
    side = min(w, h)
    left = (w - side) // 2
    top = (h - side) // 2
    return img.crop((left, top, left + side, top + side))


def _save_png(img: Image.Image, out_path: Path, px: int) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    resized = img.resize((px, px), Image.Resampling.LANCZOS)
    resized.save(out_path, format="PNG", optimize=True)


def generate_ios_appicon(src: Image.Image, out_appiconset: Path) -> None:
    out_appiconset.mkdir(parents=True, exist_ok=True)

    images: list[dict[str, str]] = []
    seen: set[tuple[str, str, str]] = set()
    for spec in IOS_SPECS:
        # iPad 83.5x83.5 is a special case; we approximate via 83 with 2x = 166px to keep file naming simple.
        # If you want exact 167px icon, add a dedicated spec and adjust here.
        _save_png(src, out_appiconset / spec.filename, spec.px)
        key = (spec.idiom, spec.size_str, spec.scale_str)
        if key in seen:
            continue
        seen.add(key)
        images.append(
            {
                "idiom": spec.idiom,
                "size": spec.size_str,
                "scale": spec.scale_str,
                "filename": spec.filename,
            }
        )

    contents = {
        "images": images,
        "info": {"version": 1, "author": "xcode"},
    }
    (out_appiconset / "Contents.json").write_text(json.dumps(contents, indent=2) + "\n", encoding="utf-8")


def generate_android_icons(src: Image.Image, out_res_dir: Path) -> None:
    for folder, px in ANDROID_DENSITIES.items():
        _save_png(src, out_res_dir / folder / "ic_launcher.png", px)

    # Play Store listing icon (not used at runtime, but handy to keep around)
    _save_png(src, out_res_dir / "play-store" / "ic_launcher_512.png", 512)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate iOS/Android app icon PNGs from a single square source PNG.")
    parser.add_argument("--input", required=True, help="Path to a source PNG (prefer 1024x1024 or higher).")
    parser.add_argument(
        "--ios-out",
        default="platform/ios/Assets.xcassets/AppIcon.appiconset",
        help="Output AppIcon.appiconset folder path.",
    )
    parser.add_argument(
        "--android-out",
        default="platform/android/app/src/main/res",
        help="Output Android res/ folder path.",
    )
    args = parser.parse_args()

    img_path = Path(args.input)
    if not img_path.is_file():
        raise SystemExit(f"Input file not found: {img_path}")

    src = _load_and_square(img_path)
    generate_ios_appicon(src, Path(args.ios_out))
    generate_android_icons(src, Path(args.android_out))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

