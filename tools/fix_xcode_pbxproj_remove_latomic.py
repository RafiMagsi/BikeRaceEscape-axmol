#!/usr/bin/env python3
"""
Remove '-latomic' from generated Xcode pbxproj files.

Why:
- openal-soft sometimes adds '-latomic' on non-Apple toolchains.
- iOS/iOS-simulator SDKs do not ship a standalone libatomic, so the link fails:
    "Library 'atomic' not found"

This script is safe/idempotent and only removes explicit '-latomic' entries.
"""

from __future__ import annotations

import sys
from pathlib import Path


def _remove_latomic(text: str) -> tuple[str, int]:
    # Remove common pbxproj encodings of -latomic.
    patterns = [
        '"-latomic",\n',
        '"-latomic",\r\n',
        '"-latomic",',
        '-latomic",\n',
        '-latomic",\r\n',
        '-latomic",',
        ' -latomic ',
    ]
    removed = 0
    for p in patterns:
        if p in text:
            count = text.count(p)
            removed += count
            text = text.replace(p, "" if p.endswith(",") else "")
    # Also handle the case where it's the only token on a line: \t\t\t\t\t\t"-latomic",
    lines = text.splitlines(True)
    new_lines = []
    for line in lines:
        if '"-latomic"' in line:
            removed += 1
            continue
        new_lines.append(line)
    return ("".join(new_lines), removed)


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: fix_xcode_pbxproj_remove_latomic.py <path/to/project.pbxproj>", file=sys.stderr)
        return 2

    pbxproj = Path(argv[1])
    if not pbxproj.exists():
        print(f"error: not found: {pbxproj}", file=sys.stderr)
        return 2

    original = pbxproj.read_text(encoding="utf-8", errors="ignore")
    updated, removed = _remove_latomic(original)

    if removed == 0:
        print(f"✓ no -latomic found in {pbxproj}")
        return 0

    pbxproj.write_text(updated, encoding="utf-8")
    print(f"✓ removed {removed} occurrences of -latomic from {pbxproj}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))

