#!/usr/bin/env python3
from __future__ import annotations

import argparse
import plistlib
from pathlib import Path


def load_ids(path: Path) -> list[str]:
    ids: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        ids.append(s)
    # Preserve order but unique.
    seen: set[str] = set()
    out: list[str] = []
    for s in ids:
        if s in seen:
            continue
        seen.add(s)
        out.append(s)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--info-plist", required=True, type=Path)
    ap.add_argument("--ids-file", required=True, type=Path)
    args = ap.parse_args()

    info_plist: Path = args.info_plist
    ids_file: Path = args.ids_file

    data = plistlib.loads(info_plist.read_bytes())
    items = data.get("SKAdNetworkItems")
    if not isinstance(items, list):
        items = []
        data["SKAdNetworkItems"] = items

    existing: set[str] = set()
    for it in items:
        if isinstance(it, dict):
            v = it.get("SKAdNetworkIdentifier")
            if isinstance(v, str) and v:
                existing.add(v)

    required = load_ids(ids_file)
    added = 0
    for sk_id in required:
        if sk_id in existing:
            continue
        items.append({"SKAdNetworkIdentifier": sk_id})
        existing.add(sk_id)
        added += 1

    if added:
        info_plist.write_bytes(plistlib.dumps(data, sort_keys=False))
        print(f"[skadnetwork] added {added} ids to {info_plist}")
    else:
        print(f"[skadnetwork] no changes needed for {info_plist}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

