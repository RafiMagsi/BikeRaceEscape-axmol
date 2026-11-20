#!/usr/bin/env python3
"""Repack approved painted source sheets into the existing V2 atlas contracts.

The image generator produces reviewable contact sheets, not engine-safe atlases.
This script isolates approved subjects, fits them inside the registered frame
rectangles, preserves every frame name/plist, and leaves a transparent gutter.
It never edits CompleteGameInfo.plist or installs runtime files by itself.
"""

from __future__ import annotations

import plistlib
import re
from pathlib import Path

import cv2
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
HD = ROOT / "Content/Images/HD"
ART = ROOT / "reskin-automation/art/v2-production"
OUT = ART / "built"
RECT = re.compile(r"\{\{\s*(\d+)\s*,\s*(\d+)\s*\},\s*\{\s*(\d+)\s*,\s*(\d+)\s*\}\}")


EVENT_RECTS = {
    "dino_small_0": (7, 16, 91, 64), "dino_small_1": (107, 16, 93, 64),
    "dino_small_2": (207, 17, 88, 62), "dino_small_3": (302, 18, 84, 61),
    "dino_big_0": (390, 23, 155, 116), "dino_big_1": (548, 23, 157, 116),
    "dino_big_2": (711, 23, 157, 115), "dino_big_3": (880, 21, 162, 115),
    "rider_cool_0": (1065, 2, 154, 134), "rider_cool_1": (16, 146, 155, 141),
    "rider_cool_2": (203, 145, 155, 142), "rider_cool_3": (389, 146, 156, 141),
    "rider_delivery_0": (577, 146, 154, 141), "rider_delivery_1": (763, 146, 158, 141),
    "rider_delivery_2": (949, 145, 158, 142), "rider_delivery_3": (18, 291, 160, 138),
    "rider_ambient_a_0": (205, 292, 159, 137), "rider_ambient_a_1": (390, 292, 157, 138),
    "rider_ambient_a_2": (580, 291, 160, 139), "rider_ambient_a_3": (771, 291, 159, 141),
    "rider_ambient_b_0": (961, 291, 162, 138), "rider_ambient_b_1": (18, 435, 155, 142),
    "rider_ambient_b_2": (206, 434, 155, 142), "rider_ambient_b_3": (388, 435, 158, 143),
    "rider_grandma_0": (568, 434, 167, 143), "rider_grandma_1": (757, 434, 168, 143),
    "rider_grandma_2": (946, 434, 172, 143), "rider_grandma_3": (9, 579, 169, 142),
    "meteor_0": (198, 587, 50, 118), "meteor_1": (275, 593, 51, 111),
    "meteor_2": (353, 584, 54, 120), "meteor_3": (432, 580, 53, 123),
    "flame_0": (506, 592, 82, 41), "flame_1": (596, 591, 80, 42),
    "flame_2": (682, 591, 78, 41), "flame_3": (767, 590, 76, 42),
    "sign_work_0": (1152, 579, 90, 143),
    "cone_0": (4, 736, 52, 68), "barrel_0": (55, 733, 73, 75),
    "rock_0": (133, 734, 80, 67), "coin_gold_0": (214, 729, 40, 40),
    "sign_dino_0": (252, 730, 87, 141), "pizza_box_0": (343, 741, 68, 48),
    "impact_star_0": (417, 734, 76, 74),
}


def frames(plist_path: Path) -> dict[str, tuple[int, int, int, int]]:
    data = plistlib.loads(plist_path.read_bytes())
    result = {}
    for name, item in data["frames"].items():
        match = RECT.fullmatch(item["frame"].replace(" ", ""))
        if not match:
            raise ValueError(f"unsupported frame rectangle for {name}: {item['frame']}")
        result[name] = tuple(map(int, match.groups()))
    return result


def frame_record(plist_path: Path, name: str) -> tuple[tuple[int, int, int, int], tuple[int, int, int, int], tuple[int, int]]:
    data = plistlib.loads(plist_path.read_bytes())["frames"][name]
    packed = tuple(map(int, RECT.fullmatch(data["frame"].replace(" ", "")).groups()))
    source_rect = tuple(map(int, RECT.fullmatch(data["sourceColorRect"].replace(" ", "")).groups()))
    size_values = tuple(map(int, re.findall(r"\d+", data["sourceSize"])))
    return packed, source_rect, size_values


def clean(image: Image.Image, threshold: int = 12) -> Image.Image:
    rgba = image.convert("RGBA")
    data = np.array(rgba)
    data[data[:, :, 3] <= threshold] = 0
    return Image.fromarray(data, "RGBA")


def crop(image: Image.Image, rect: tuple[int, int, int, int], margin: int = 3) -> Image.Image:
    x, y, width, height = rect
    box = (max(0, x - margin), max(0, y - margin), min(image.width, x + width + margin), min(image.height, y + height + margin))
    return clean(image.crop(box))


def components(image: Image.Image, threshold: int = 24, min_area: int = 200) -> list[tuple[int, int, int, int]]:
    alpha = np.asarray(image.convert("RGBA").getchannel("A"), dtype=np.uint8)
    mask = (alpha > threshold).astype(np.uint8) * 255
    count, _, stats, _ = cv2.connectedComponentsWithStats(mask, 8)
    boxes = []
    for index in range(1, count):
        x, y, width, height, area = map(int, stats[index])
        if area >= min_area:
            boxes.append((x, y, width, height))
    return boxes


def row_order(boxes: list[tuple[int, int, int, int]], tolerance: int = 80) -> list[tuple[int, int, int, int]]:
    remaining = sorted(boxes, key=lambda b: (b[1] + b[3] / 2, b[0]))
    ordered = []
    while remaining:
        first = remaining.pop(0)
        centre = first[1] + first[3] / 2
        row = [first]
        keep = []
        for box in remaining:
            if abs((box[1] + box[3] / 2) - centre) <= tolerance:
                row.append(box)
            else:
                keep.append(box)
        ordered.extend(sorted(row, key=lambda b: b[0]))
        remaining = keep
    return ordered


def fit(subject: Image.Image, width: int, height: int, pad: int = 3, bottom_align: bool = True) -> Image.Image:
    alpha_box = subject.getchannel("A").getbbox()
    if not alpha_box:
        raise ValueError("subject contains no visible alpha")
    subject = subject.crop(alpha_box)
    scale = min((width - 2 * pad) / subject.width, (height - 2 * pad) / subject.height)
    size = (max(1, round(subject.width * scale)), max(1, round(subject.height * scale)))
    subject = subject.resize(size, Image.Resampling.LANCZOS)
    cell = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    x = (width - subject.width) // 2
    y = height - pad - subject.height if bottom_align else (height - subject.height) // 2
    cell.alpha_composite(subject, (x, y))
    return clean(cell, 4)


def paste_frame(atlas: Image.Image, layout: dict[str, tuple[int, int, int, int]], name: str, subject: Image.Image, pad: int = 3, bottom: bool = True) -> None:
    x, y, width, height = layout[name]
    atlas.alpha_composite(fit(subject, width, height, pad, bottom), (x, y))


def build_events() -> None:
    layout = frames(HD / "v2_events.plist")
    old = Image.open(HD / "v2_events.png").convert("RGBA")
    source = Image.open(ART / "events/events-painted-source.png").convert("RGBA")
    workers = Image.open(ART / "workers/workers-painted-source.png").convert("RGBA")
    worker_boxes = row_order(components(workers, min_area=1000), tolerance=250)
    if len(worker_boxes) != 2:
        raise ValueError(f"expected two worker components, found {len(worker_boxes)}: {worker_boxes}")
    atlas = Image.new("RGBA", old.size, (0, 0, 0, 0))
    for name in layout:
        if name in EVENT_RECTS:
            paste_frame(atlas, layout, name, crop(source, EVENT_RECTS[name]), 2)
        elif name == "worker_0":
            paste_frame(atlas, layout, name, crop(workers, worker_boxes[0]), 2)
        elif name == "worker_1":
            paste_frame(atlas, layout, name, crop(workers, worker_boxes[1]), 2)
        elif name == "shadow_0":
            x, y, width, height = layout[name]
            # Preserve this approved legacy frame byte-for-byte inside its atlas
            # rectangle. Refitting it on every run progressively shrinks it.
            atlas.alpha_composite(old.crop((x, y, x + width, y + height)), (x, y))
        else:
            raise ValueError(f"no approved painted source for event frame {name}")
    atlas.save(OUT / "v2_events.png", optimize=True)


def build_portraits() -> None:
    layout = frames(HD / "v2_portraits.plist")
    source = Image.open(ART / "portraits/portraits-painted-source.png").convert("RGBA")
    boxes = row_order(components(source, min_area=5000), tolerance=170)
    names = [
        "portrait_player_neutral_0", "portrait_player_confident_0", "portrait_player_surprised_0",
        "portrait_player_frustrated_0", "portrait_cool_kid_neutral_0", "portrait_cool_kid_confident_0",
        "portrait_cool_kid_surprised_0", "portrait_cool_kid_frustrated_0",
    ]
    if len(boxes) != len(names):
        raise ValueError(f"expected eight portrait components, found {len(boxes)}: {boxes}")
    atlas = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    for name, box in zip(names, boxes):
        paste_frame(atlas, layout, name, crop(source, box), 4)
    atlas.save(OUT / "v2_portraits.png", optimize=True)


def build_scenic() -> None:
    layout = frames(HD / "v2_scenic.plist")
    source = Image.open(ART / "scenic/scenic-painted-source.png").convert("RGBA")
    boxes = row_order(components(source, min_area=5000), tolerance=170)
    names = ["scenic_walker_0", "scenic_walker_1", "scenic_peek_0"]
    if len(boxes) != len(names):
        raise ValueError(f"expected three scenic components, found {len(boxes)}: {boxes}")
    atlas = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    for name, box in zip(names, boxes):
        paste_frame(atlas, layout, name, crop(source, box), 5)
    atlas.save(OUT / "v2_scenic.png", optimize=True)


def build_barrel() -> None:
    plist = HD / "animation_s0.plist"
    packed, source_rect, source_size = frame_record(plist, "Obstacles_4_0")
    generated = Image.open(ART / "obstacles/barrel/barrel-painted-source.png").convert("RGBA")
    visible = generated.getchannel("A").getbbox()
    if not visible:
        raise ValueError("painted barrel source has no visible alpha")
    source = clean(generated.crop(visible))
    rx, ry, rw, rh = source_rect
    logical = Image.new("RGBA", source_size, (0, 0, 0, 0))
    logical.alpha_composite(fit(source, rw, rh, 1), (rx, ry))
    barrel_dir = ART / "obstacles/barrel"
    logical.save(barrel_dir / "barrel-painted-200.png", optimize=True)

    atlas = Image.open(HD / "animation_s0.png").convert("RGBA")
    x, y, width, height = packed
    atlas.paste((0, 0, 0, 0), (x, y, x + width, y + height))
    atlas.alpha_composite(logical.crop((rx, ry, rx + rw, ry + rh)), (x, y))
    atlas.save(OUT / "animation_s0.png", optimize=True)


def logical_frame(plist_path: Path, name: str, subject: Image.Image) -> Image.Image:
    _, source_rect, source_size = frame_record(plist_path, name)
    rx, ry, rw, rh = source_rect
    logical = Image.new("RGBA", source_size, (0, 0, 0, 0))
    logical.alpha_composite(fit(subject, rw, rh, 1), (rx, ry))
    return logical


def replace_registered_frame(
    atlases: dict[str, Image.Image], atlas_name: str, name: str, subject: Image.Image
) -> Image.Image:
    plist_path = HD / f"{atlas_name}.plist"
    packed, source_rect, _ = frame_record(plist_path, name)
    logical = logical_frame(plist_path, name, subject)
    x, y, width, height = packed
    rx, ry, rw, rh = source_rect
    if (width, height) != (rw, rh):
        raise ValueError(f"rotated or mismatched frame is not supported: {atlas_name}/{name}")
    atlas = atlases[atlas_name]
    atlas.paste((0, 0, 0, 0), (x, y, x + width, y + height))
    atlas.alpha_composite(logical.crop((rx, ry, rx + rw, ry + rh)), (x, y))
    return logical


def build_legacy_obstacles_and_ui() -> None:
    """Replace weak legacy families while preserving every registered frame."""
    atlas_names = ["animation_s0", "animation_s1", "animation_s2", "animation_s3"]
    atlases = {
        name: Image.open(OUT / f"{name}.png" if name == "animation_s0" else HD / f"{name}.png").convert("RGBA")
        for name in atlas_names
    }

    creature_source = Image.open(ART / "obstacles/all/creatures-painted-source.png").convert("RGBA")
    creature_boxes = row_order(components(creature_source, min_area=5000), tolerance=90)
    if len(creature_boxes) != 21:
        raise ValueError(f"expected 21 creature frames, found {len(creature_boxes)}: {creature_boxes}")
    creature_subjects = [crop(creature_source, box, 2) for box in creature_boxes]
    creature_frames = [
        [("animation_s3", f"Obstacles_0_{i}") for i in range(8)],
        [
            ("animation_s3", "Obstacles_1_0"), ("animation_s0", "Obstacles_1_1"),
            ("animation_s3", "Obstacles_1_2"), ("animation_s3", "Obstacles_1_3"),
            ("animation_s1", "Obstacles_1_4"), ("animation_s1", "Obstacles_1_5"),
            ("animation_s1", "Obstacles_1_6"), ("animation_s1", "Obstacles_1_7"),
        ],
        [("animation_s3", f"Obstacles_2_{i}") for i in range(5)],
    ]

    obstacle_dir = ART / "obstacles/all/logical"
    obstacle_dir.mkdir(parents=True, exist_ok=True)
    source_cursor = 0
    for obstacle_index, (family, slug) in enumerate(zip(creature_frames, ["alligator", "lizard", "rat"])):
        logical_frames = []
        for atlas_name, frame_name in family:
            logical_frames.append(replace_registered_frame(
                atlases, atlas_name, frame_name, creature_subjects[source_cursor]
            ))
            source_cursor += 1
        # The runtime uses one polygon for every animation frame even though
        # their logical canvases vary by a few pixels. Trace the registered
        # first frame, matching the legacy collision authoring contract.
        logical_frames[0].save(
            obstacle_dir / f"{obstacle_index}-{slug}-collision-source.png", optimize=True
        )

    hurdle_source = Image.open(ART / "obstacles/all/hurdles-painted-source.png").convert("RGBA")
    hurdle_boxes = row_order(components(hurdle_source, min_area=5000), tolerance=150)
    if len(hurdle_boxes) != 7:
        raise ValueError(f"expected seven hurdle concepts, found {len(hurdle_boxes)}: {hurdle_boxes}")
    hurdle_subjects = [crop(hurdle_source, box, 2) for box in hurdle_boxes]
    hurdle_families = [
        (3, "light-barrier", [("animation_s0", "Obstacles_3_0", 0), ("animation_s0", "Obstacles_3_1", 1)]),
        (5, "launch-ramp", [("animation_s1", "Obstacles_5_0", 2)]),
        (6, "fence", [("animation_s0", "Obstacles_6_0", 3)]),
        (7, "grind-rail", [("animation_s3", "Obstacles_7_0", 4)]),
        (8, "dirt-ramp", [("animation_s1", "Obstacles_8_0", 5)]),
    ]
    for obstacle_index, slug, family in hurdle_families:
        logical_frames = [
            replace_registered_frame(atlases, atlas_name, frame_name, hurdle_subjects[source_index])
            for atlas_name, frame_name, source_index in family
        ]
        union = Image.new("RGBA", logical_frames[0].size, (0, 0, 0, 0))
        for logical in logical_frames:
            union = Image.alpha_composite(union, logical)
        union.save(obstacle_dir / f"{obstacle_index}-{slug}-collision-source.png", optimize=True)

    ui_source = Image.open(ART / "ui/screens-painted-source.png").convert("RGBA")
    ui_boxes = row_order(components(ui_source, min_area=10000), tolerance=100)
    if len(ui_boxes) != 10:
        raise ValueError(f"expected ten UI components, found {len(ui_boxes)}: {ui_boxes}")
    ui_subjects = [crop(ui_source, box, 2) for box in ui_boxes]
    ui_map = [
        ("animation_s3", "InterfaceGO_0_0", 0),
        ("animation_s2", "InterfacePM_0_0", 1),
        ("animation_s3", "InterfaceCoinShop_1_0", 2),
        ("animation_s3", "InterfaceMM_4_0", 6),
        ("animation_s3", "InterfaceMM_5_0", 3),
        ("animation_s3", "InterfaceMM_6_0", 2),
        ("animation_s3", "InterfaceMM_7_0", 4),
        ("animation_s3", "InterfaceMM_8_0", 4),
        ("animation_s3", "InterfaceGO_4_0", 4),
        ("animation_s3", "InterfaceGO_5_0", 2),
        ("animation_s3", "InterfaceGO_6_0", 3),
        ("animation_s3", "InterfaceCS_12_0", 2),
        ("animation_s1", "InterfaceGF_4_0", 8),
    ]
    for atlas_name, frame_name, source_index in ui_map:
        replace_registered_frame(atlases, atlas_name, frame_name, ui_subjects[source_index])

    for atlas_name, atlas in atlases.items():
        atlas.save(OUT / f"{atlas_name}.png", optimize=True)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    build_events()
    build_portraits()
    build_scenic()
    build_barrel()
    build_legacy_obstacles_and_ui()
    print(f"built production candidates in {OUT}")


if __name__ == "__main__":
    main()
