#!/usr/bin/env python3
import tempfile
import unittest
from pathlib import Path

from PIL import Image, ImageDraw

import art_pipeline


class ArtPipelineTests(unittest.TestCase):
    def test_trace_uses_bottom_left_absolute_coordinates(self):
        image = Image.new("RGBA", (100, 80), (0, 0, 0, 0))
        ImageDraw.Draw(image).rectangle((10, 20, 69, 59), fill=(255, 255, 255, 255))
        report = art_pipeline.trace_collision(image, threshold=15, vertices=5)
        xs = [point[0] for point in report["vertices_ccw"]]
        ys = [point[1] for point in report["vertices_ccw"]]
        self.assertEqual((min(xs), max(xs)), (10.0, 70.0))
        self.assertEqual((min(ys), max(ys)), (20.0, 60.0))
        self.assertGreater(report["polygon_area"], 0)
        self.assertEqual(len(report["plist_strings"]), 5)

    def test_compare_detects_canvas_and_alpha_changes(self):
        baseline = Image.new("RGBA", (20, 20), (0, 0, 0, 0))
        candidate = baseline.copy()
        baseline.putpixel((5, 5), (255, 255, 255, 255))
        candidate.putpixel((6, 5), (255, 255, 255, 255))
        report = art_pipeline.compare_images(baseline, candidate, threshold=15)
        self.assertTrue(report["same_canvas"])
        self.assertFalse(report["exact_alpha_mask"])
        self.assertEqual(report["alpha_iou"], 0.0)

    def test_atlas_audit_reads_format_two(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            image = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
            ImageDraw.Draw(image).rectangle((3, 4, 12, 15), fill=(255, 255, 255, 255))
            image.save(root / "sheet.png")
            plist = {
                "frames": {
                    "thing_0": {
                        "frame": "{{0,0},{20,20}}", "offset": "{0,0}",
                        "rotated": False, "sourceSize": "{20,20}",
                        "sourceColorRect": "{{0,0},{20,20}}",
                    }
                },
                "metadata": {"format": 2, "textureFileName": "sheet.png", "size": "{32,32}"},
            }
            path = root / "sheet.plist"
            path.write_bytes(__import__("plistlib").dumps(plist))
            records = art_pipeline.atlas_records(path)
            self.assertEqual(records[0]["name"], "thing_0")
            self.assertEqual(records[0]["alpha_bounds_in_frame"], [3, 4, 13, 16])


if __name__ == "__main__":
    unittest.main()
