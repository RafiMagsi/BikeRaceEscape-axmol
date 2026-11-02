#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageChops, ImageOps


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = REPO_ROOT / "store/rebrand-v3/icon-1024.png"
DEFAULT_IOS_OUT = REPO_ROOT / "proj.ios_mac/ios/targets/ios/Images.xcassets/AppIcon.appiconset"
DEFAULT_ANDROID_OUT = REPO_ROOT / "proj.android/app/res"


@dataclass(frozen=True)
class IOSIconSpec:
    # Point size (can be fractional; e.g. 83.5 for iPad Pro).
    size: float
    scale: int
    idiom: str

    @property
    def px(self) -> int:
        # Apple requires exact pixels for some sizes (e.g. 83.5pt@2x -> 167px).
        return int(round(self.size * self.scale))

    @property
    def size_str(self) -> str:
        # Keep Xcode-friendly formatting (e.g. "83.5x83.5").
        if float(self.size).is_integer():
            s = str(int(self.size))
        else:
            s = str(self.size).rstrip("0").rstrip(".")
        return f"{s}x{s}"

    @property
    def scale_str(self) -> str:
        return f"{self.scale}x"

    @property
    def filename(self) -> str:
        # Keep filenames deterministic and human-readable.
        size_token = self.size_str
        if self.scale == 1:
            return f"AppIcon-{size_token}.png"
        return f"AppIcon-{size_token}@{self.scale}x.png"


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
    IOSIconSpec(83.5, 2, "ipad"),  # iPad Pro (167x167)
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
    with Image.open(img_path) as source:
        img = ImageOps.exif_transpose(source).convert("RGBA")

    alpha = img.getchannel("A")
    if alpha.getextrema() != (255, 255):
        raise ValueError(
            f"Source icon must be fully opaque; transparent pixels found in {img_path}"
        )

    w, h = img.size
    if w != h:
        # Center-crop to square while keeping the subject centered.
        side = min(w, h)
        left = (w - side) // 2
        top = (h - side) // 2
        img = img.crop((left, top, left + side, top + side))

    if min(img.size) < 1024:
        raise ValueError(f"Source icon must be at least 1024x1024; got {img.size[0]}x{img.size[1]}")

    # Apple rejects app icons that contain an alpha channel, even if every pixel
    # is opaque. RGB also keeps the Android launcher files deterministic.
    return img.convert("RGB")


def _save_png(img: Image.Image, out_path: Path, px: int) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    resized = img.resize((px, px), Image.Resampling.LANCZOS)
    resized.convert("RGB").save(out_path, format="PNG", optimize=True)


def generate_ios_appicon(src: Image.Image, out_appiconset: Path) -> None:
    out_appiconset.mkdir(parents=True, exist_ok=True)

    images: list[dict[str, str]] = []
    seen: set[tuple[str, str, str]] = set()
    for spec in IOS_SPECS:
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


def _assert_png(path: Path, expected_px: int) -> None:
    if not path.is_file():
        raise ValueError(f"Missing generated icon: {path}")
    with Image.open(path) as image:
        image.load()
        if image.size != (expected_px, expected_px):
            raise ValueError(
                f"Wrong icon size for {path}: expected {expected_px}x{expected_px}, got {image.size}"
            )
        if "A" in image.getbands():
            raise ValueError(f"Generated icon must not contain an alpha channel: {path}")


def verify_outputs(source: Image.Image, ios_out: Path, android_out: Path) -> None:
    for spec in IOS_SPECS:
        _assert_png(ios_out / spec.filename, spec.px)

    contents_path = ios_out / "Contents.json"
    contents = json.loads(contents_path.read_text(encoding="utf-8"))
    catalog_entries = {
        (entry["idiom"], entry["size"], entry["scale"], entry["filename"])
        for entry in contents["images"]
    }
    expected_entries = {
        (spec.idiom, spec.size_str, spec.scale_str, spec.filename) for spec in IOS_SPECS
    }
    if catalog_entries != expected_entries:
        raise ValueError(f"iOS AppIcon metadata does not match the generated files: {contents_path}")

    for folder, px in ANDROID_DENSITIES.items():
        _assert_png(android_out / folder / "ic_launcher.png", px)
    _assert_png(android_out / "play-store" / "ic_launcher_512.png", 512)

    # The full-size iOS icon must be an exact RGB copy of the source.
    with Image.open(ios_out / "AppIcon-1024x1024.png") as generated:
        if ImageChops.difference(source, generated.convert("RGB")).getbbox() is not None:
            raise ValueError("Generated 1024px iOS icon differs from the source icon")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate iOS/Android app icon PNGs from a single square source PNG.")
    parser.add_argument(
        "--input",
        type=Path,
        default=DEFAULT_SOURCE,
        help=f"Source PNG (default: {DEFAULT_SOURCE.relative_to(REPO_ROOT)}).",
    )
    parser.add_argument(
        "--ios-out",
        type=Path,
        default=DEFAULT_IOS_OUT,
        help="Output AppIcon.appiconset folder path.",
    )
    parser.add_argument(
        "--android-out",
        type=Path,
        default=DEFAULT_ANDROID_OUT,
        help="Output Android res/ folder path.",
    )
    parser.add_argument(
        "--verify-only",
        action="store_true",
        help="Validate existing generated icons without replacing them.",
    )
    args = parser.parse_args()

    img_path = args.input.resolve()
    if not img_path.is_file():
        raise SystemExit(f"Input file not found: {img_path}")

    try:
        src = _load_and_square(img_path)
        ios_out = args.ios_out.resolve()
        android_out = args.android_out.resolve()
        if not args.verify_only:
            generate_ios_appicon(src, ios_out)
            generate_android_icons(src, android_out)
        verify_outputs(src, ios_out, android_out)
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as error:
        raise SystemExit(f"App icon generation failed: {error}") from error

    action = "Verified" if args.verify_only else "Generated and verified"
    print(f"{action} iOS and Android app icons from {img_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
