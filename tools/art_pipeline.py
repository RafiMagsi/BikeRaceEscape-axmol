#!/usr/bin/env python3
"""Deterministic helpers for collision-safe 2D art replacement.

OpenCV performs contour extraction and convex simplification; Pillow handles
atlas IO and review images. Commands are read-only unless `trace` is given both
plist targeting arguments and `--apply`.

The legacy game expects obstacle collision points in sprite pixel coordinates
with a bottom-left origin. PZGObstacle converts them to body-local coordinates;
do not feed it center-relative points.
"""

from __future__ import annotations

import argparse
import json
import math
import plistlib
import re
import subprocess
import sys
from pathlib import Path
from typing import Iterable, Sequence

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageChops

Point = tuple[float, float]
RECT_RE = re.compile(
    r"\{\{\s*(-?[\d.]+)\s*,\s*(-?[\d.]+)\s*\},\s*"
    r"\{\s*([\d.]+)\s*,\s*([\d.]+)\s*\}\}"
)
SIZE_RE = re.compile(r"\{\s*([\d.]+)\s*,\s*([\d.]+)\s*\}")


def _rgba(path: Path) -> Image.Image:
    image = Image.open(path).convert("RGBA")
    if "A" not in image.getbands():
        raise ValueError(f"image has no alpha channel: {path}")
    return image


def _alpha_mask(image: Image.Image, threshold: int) -> Image.Image:
    if not 0 <= threshold <= 255:
        raise ValueError("alpha threshold must be between 0 and 255")
    return image.getchannel("A").point(lambda value: 255 if value > threshold else 0)


def _opencv_contour(mask: Image.Image, vertices: int) -> list[Point]:
    """Trace the largest alpha island and simplify its convex outer boundary.

    OpenCV contours use a top-left pixel-centre coordinate system. We expand
    every contour sample to its four pixel edges before building the hull, then
    convert to the game's bottom-left sprite coordinate system. This preserves
    the full visible alpha extent instead of shrinking every side by one pixel.
    """
    array = np.asarray(mask, dtype=np.uint8)
    contours, _ = cv2.findContours(array, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        raise ValueError("alpha mask contains no visible contours")
    main = max(contours, key=cv2.contourArea)
    if cv2.contourArea(main) <= 0:
        raise ValueError("largest alpha contour has no usable area")

    edge_points = []
    for point in main.reshape(-1, 2):
        x, y = int(point[0]), int(point[1])
        edge_points.extend(((x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1)))
    hull = cv2.convexHull(np.asarray(edge_points, dtype=np.float32).reshape(-1, 1, 2), clockwise=False)

    perimeter = cv2.arcLength(hull, True)
    epsilon = 0.0025 * perimeter
    approximation = cv2.approxPolyDP(hull, epsilon, True)
    while len(approximation) > vertices and epsilon < perimeter:
        epsilon += 0.0025 * perimeter
        approximation = cv2.approxPolyDP(hull, epsilon, True)

    height = mask.height
    bottom_left = [(float(x), float(height - y)) for x, y in approximation.reshape(-1, 2)]
    return reduce_convex(convex_hull(bottom_left), vertices)


def convex_hull(points: Iterable[Point]) -> list[Point]:
    pts = sorted(set(points))
    if len(pts) < 3:
        raise ValueError("at least three distinct alpha-boundary points are required")

    def cross(origin: Point, a: Point, b: Point) -> float:
        return (a[0] - origin[0]) * (b[1] - origin[1]) - (a[1] - origin[1]) * (b[0] - origin[0])

    lower: list[Point] = []
    for point in pts:
        while len(lower) >= 2 and cross(lower[-2], lower[-1], point) <= 0:
            lower.pop()
        lower.append(point)
    upper: list[Point] = []
    for point in reversed(pts):
        while len(upper) >= 2 and cross(upper[-2], upper[-1], point) <= 0:
            upper.pop()
        upper.append(point)
    return lower[:-1] + upper[:-1]


def polygon_area(points: Sequence[Point]) -> float:
    return 0.5 * sum(
        a[0] * b[1] - b[0] * a[1]
        for a, b in zip(points, points[1:] + points[:1])
    )


def reduce_convex(points: Sequence[Point], count: int) -> list[Point]:
    if count < 3:
        raise ValueError("collision polygons need at least three vertices")
    reduced = list(points)
    while len(reduced) > count:
        # Removing the vertex with the smallest local triangle discards the
        # least silhouette area while retaining convex winding.
        costs = []
        for i, current in enumerate(reduced):
            before = reduced[(i - 1) % len(reduced)]
            after = reduced[(i + 1) % len(reduced)]
            cost = abs(
                (current[0] - before[0]) * (after[1] - before[1])
                - (current[1] - before[1]) * (after[0] - before[0])
            )
            costs.append((cost, i))
        reduced.pop(min(costs)[1])

    # PZGObstacle currently always passes five input vertices to Box2D. If a
    # rectangular mask produces four hull corners, split the longest edge with
    # a non-duplicate collinear midpoint; Box2D can safely canonicalize it.
    while len(reduced) < count:
        edges = [
            (math.dist(a, b), i, a, b)
            for i, (a, b) in enumerate(zip(reduced, reduced[1:] + reduced[:1]))
        ]
        _, i, a, b = max(edges)
        reduced.insert(i + 1, ((a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0))
    if polygon_area(reduced) < 0:
        reduced.reverse()
    return reduced


def trace_collision(image: Image.Image, threshold: int = 15, vertices: int = 5, inset: float = 0.0) -> dict:
    mask = _alpha_mask(image, threshold)
    bbox = mask.getbbox()
    if bbox is None:
        raise ValueError("alpha mask contains no visible pixels")
    points = _opencv_contour(mask, vertices)
    if inset:
        cx = sum(point[0] for point in points) / len(points)
        cy = sum(point[1] for point in points) / len(points)
        adjusted = []
        for x, y in points:
            distance = math.hypot(x - cx, y - cy)
            factor = max(0.0, (distance - inset) / distance) if distance else 1.0
            adjusted.append((cx + (x - cx) * factor, cy + (y - cy) * factor))
        points = adjusted
    rounded = [[round(x, 2), round(y, 2)] for x, y in points]
    width, height = image.size
    left, top, right, bottom = bbox
    return {
        "canvas": [width, height],
        "trace_engine": f"OpenCV {cv2.__version__}",
        "coordinate_system": "sprite pixels, origin bottom-left",
        "alpha_threshold": threshold,
        "alpha_bounds_top_left": [left, top, right, bottom],
        "alpha_bounds_bottom_left": [left, height - bottom, right, height - top],
        "inset_pixels": inset,
        "vertices_ccw": rounded,
        "plist_strings": [f"{{{x:g}, {y:g}}}" for x, y in rounded],
        "polygon_area": round(abs(polygon_area([(p[0], p[1]) for p in rounded])), 2),
    }


def collision_overlay(image: Image.Image, points: Sequence[Sequence[float]], output: Path) -> None:
    overlay = image.copy()
    draw = ImageDraw.Draw(overlay)
    height = image.height
    screen_points = [(float(x), height - float(y)) for x, y in points]
    draw.line(screen_points + screen_points[:1], fill=(255, 40, 40, 255), width=max(2, image.width // 100))
    radius = max(3, image.width // 80)
    for i, (x, y) in enumerate(screen_points):
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=(255, 220, 40, 255), outline=(40, 20, 10, 255))
        draw.text((x + radius + 1, y - radius), str(i), fill=(255, 255, 255, 255), stroke_width=1, stroke_fill=(0, 0, 0, 255))
    output.parent.mkdir(parents=True, exist_ok=True)
    overlay.save(output)


def _find_obstacle_entry(plist_path: Path, obstacle_index: int) -> tuple[int, dict]:
    data = plistlib.loads(plist_path.read_bytes())
    for position, item in enumerate(data.get("completeInfo", [])):
        if item.get("className") == "PZArtObstacle" and int(item.get("index", -1)) == obstacle_index:
            return position, item
    raise ValueError(f"obstacle index {obstacle_index} not found in {plist_path}")


def _apply_collision(plist_path: Path, position: int, values: Sequence[str]) -> None:
    command = [
        "plutil", "-replace", f"completeInfo.{position}.collision",
        "-json", json.dumps(list(values)), str(plist_path),
    ]
    result = subprocess.run(command, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(result.stderr.strip() or "plutil collision update failed")


def _parse_rect(value: str) -> tuple[int, int, int, int]:
    match = RECT_RE.fullmatch(value.replace(" ", "")) or RECT_RE.search(value)
    if not match:
        raise ValueError(f"unsupported atlas rectangle: {value}")
    return tuple(round(float(item)) for item in match.groups())  # type: ignore[return-value]


def _parse_size(value: str) -> tuple[int, int]:
    match = SIZE_RE.fullmatch(value.replace(" ", "")) or SIZE_RE.search(value)
    if not match:
        raise ValueError(f"unsupported size: {value}")
    return round(float(match.group(1))), round(float(match.group(2)))


def atlas_records(plist_path: Path, prefix: str = "") -> list[dict]:
    data = plistlib.loads(plist_path.read_bytes())
    texture_name = data.get("metadata", {}).get("textureFileName") or data.get("metadata", {}).get("realTextureFileName")
    if not texture_name:
        raise ValueError(f"atlas metadata has no texture filename: {plist_path}")
    texture_path = plist_path.parent / texture_name
    texture = _rgba(texture_path)
    records = []
    for name, frame in sorted(data.get("frames", {}).items()):
        if prefix and not name.startswith(prefix):
            continue
        x, y, width, height = _parse_rect(frame["frame"])
        rotated = bool(frame.get("rotated", False))
        crop = texture.crop((x, y, x + width, y + height))
        alpha = crop.getchannel("A")
        alpha_bbox = alpha.getbbox()
        histogram = alpha.histogram()
        records.append({
            "name": name,
            "atlas": texture_name,
            "frame": [x, y, width, height],
            "rotated": rotated,
            "source_size": list(_parse_size(frame.get("sourceSize", f"{{{width},{height}}}"))),
            "offset": frame.get("offset", "{0,0}"),
            "alpha_bounds_in_frame": list(alpha_bbox) if alpha_bbox else None,
            "alpha_pixels": sum(histogram[1:]),
        })
    return records


def compare_images(baseline: Image.Image, candidate: Image.Image, threshold: int) -> dict:
    base_mask = _alpha_mask(baseline, threshold)
    candidate_mask = _alpha_mask(candidate, threshold)
    same_size = baseline.size == candidate.size
    if same_size:
        intersection = ImageChops.multiply(base_mask, candidate_mask)
        union = ImageChops.lighter(base_mask, candidate_mask)
        union_pixels = sum(union.histogram()[1:])
        intersection_pixels = sum(intersection.histogram()[1:])
        exact = ImageChops.difference(base_mask, candidate_mask).getbbox() is None
        iou = intersection_pixels / union_pixels if union_pixels else 1.0
    else:
        exact, iou = False, 0.0
    return {
        "same_canvas": same_size,
        "baseline_canvas": list(baseline.size),
        "candidate_canvas": list(candidate.size),
        "baseline_alpha_bounds": list(base_mask.getbbox()) if base_mask.getbbox() else None,
        "candidate_alpha_bounds": list(candidate_mask.getbbox()) if candidate_mask.getbbox() else None,
        "exact_alpha_mask": exact,
        "alpha_iou": round(iou, 6),
    }


def command_trace(args: argparse.Namespace) -> int:
    image = _rgba(args.image)
    report = trace_collision(image, args.threshold, args.vertices, args.inset)
    if args.overlay:
        collision_overlay(image, report["vertices_ccw"], args.overlay)
        report["overlay"] = str(args.overlay)
    if args.plist is not None:
        position, obstacle = _find_obstacle_entry(args.plist, args.obstacle_index)
        report["target"] = {
            "plist": str(args.plist), "complete_info_position": position,
            "obstacle_index": args.obstacle_index, "obstacle_name": obstacle.get("name"),
            "old_collision": obstacle.get("collision", []),
        }
        if args.apply:
            _apply_collision(args.plist, position, report["plist_strings"])
            report["applied"] = True
    elif args.apply:
        raise ValueError("--apply requires --plist and --obstacle-index")
    print(json.dumps(report, indent=2))
    return 0


def command_compare(args: argparse.Namespace) -> int:
    report = compare_images(_rgba(args.baseline), _rgba(args.candidate), args.threshold)
    print(json.dumps(report, indent=2))
    failed = (
        (args.require_same_canvas and not report["same_canvas"])
        or (args.require_same_bounds and report["baseline_alpha_bounds"] != report["candidate_alpha_bounds"])
        or (args.require_exact_alpha and not report["exact_alpha_mask"])
    )
    return 1 if failed else 0


def command_atlas(args: argparse.Namespace) -> int:
    records = atlas_records(args.plist, args.prefix)
    report = {"plist": str(args.plist), "frame_count": len(records), "frames": records}
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))
    return 0


def command_brief(args: argparse.Namespace) -> int:
    records = atlas_records(args.plist, args.frame)
    matches = [record for record in records if record["name"] == args.frame]
    if not matches:
        raise ValueError(f"frame {args.frame!r} not found in {args.plist}")
    item = matches[0]
    print(f"""# Art replacement brief: `{args.frame}`

- Atlas: `{item['atlas']}`
- Frame rectangle: `{item['frame']}`
- Logical source size: `{item['source_size']}`
- Offset: `{item['offset']}`
- Alpha bounds inside packed frame: `{item['alpha_bounds_in_frame']}`
- Preserve frame name on first integration: yes
- Preserve source canvas/anchor before collision validation: yes

Style: cinematic painted arcade realism matching the existing bike/rider,
platforms/ramps, sewer parallax backgrounds, gold coins, and navy/orange HUD.
Do not follow the flat-vector style of `v2_events.png`. Use a transparent
background, strong side-view silhouette, grounded material detail, cool sewer
fill light, and controlled warm upper-left key light. No text, logos, thick
cartoon outline, baked background, bloom, or unrelated props.

Required validation: alpha comparison, collision overlay, atlas audit, gameplay
capture at normal/max speed, deterministic replay, and low-end device frame time.
""")
    return 0


def parser() -> argparse.ArgumentParser:
    root = argparse.ArgumentParser(description=__doc__)
    commands = root.add_subparsers(dest="command", required=True)

    trace = commands.add_parser("trace", help="trace alpha into the game's bottom-left collision format")
    trace.add_argument("image", type=Path)
    trace.add_argument("--threshold", type=int, default=15)
    trace.add_argument("--vertices", type=int, default=5, choices=range(3, 9))
    trace.add_argument("--inset", type=float, default=0.0, help="move hull inward by this many pixels")
    trace.add_argument("--overlay", type=Path)
    trace.add_argument("--plist", type=Path)
    trace.add_argument("--obstacle-index", type=int)
    trace.add_argument("--apply", action="store_true", help="replace the selected plist collision array")
    trace.set_defaults(run=command_trace)

    compare = commands.add_parser("compare", help="compare replacement canvas and alpha geometry")
    compare.add_argument("baseline", type=Path)
    compare.add_argument("candidate", type=Path)
    compare.add_argument("--threshold", type=int, default=15)
    compare.add_argument("--require-same-canvas", action="store_true")
    compare.add_argument("--require-same-bounds", action="store_true")
    compare.add_argument("--require-exact-alpha", action="store_true")
    compare.set_defaults(run=command_compare)

    atlas = commands.add_parser("atlas", help="emit machine-readable atlas/frame geometry")
    atlas.add_argument("plist", type=Path)
    atlas.add_argument("--prefix", default="")
    atlas.add_argument("--output", type=Path)
    atlas.set_defaults(run=command_atlas)

    brief = commands.add_parser("brief", help="emit a compact reusable art-generation brief")
    brief.add_argument("plist", type=Path)
    brief.add_argument("frame")
    brief.set_defaults(run=command_brief)
    return root


def main() -> int:
    args = parser().parse_args()
    if args.command == "trace" and (args.plist is None) != (args.obstacle_index is None):
        raise ValueError("--plist and --obstacle-index must be supplied together")
    return int(args.run(args))


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, RuntimeError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(2)
