"""Read-only image checks; only the JSON validation report is written.

Checks every non-scenery frame's alpha, metadata, canvas size, and atlas padding.
"""
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw

ROOT = Path('art/rebrand-v3')
frames = json.loads((ROOT / 'manifest.json').read_text())
allowed_alpha = {'InterfaceMM_0_0', 'InterfaceCS_0_0', 'InterfaceCoinShop_0_0', 'InterfaceMM_1_0'}
report = {'physics_frames': 0, 'preserved_alpha_frames': 0, 'atlases': {}, 'metadata_unchanged': True}
for tier in ('HD', 'SD'):
    for sheet in range(4):
        stem = f'animation_s{sheet}'
        baseline = Image.open(ROOT / f'baseline/{tier}/{stem}.png').convert('RGBA')
        candidate_path = ROOT / f'atlas/{tier}/{stem}.png'
        candidate = Image.open(candidate_path).convert('RGBA')
        assert candidate.size == baseline.size == (2048, 2048)
        diff = ImageChops.difference(candidate, baseline)
        for frame in (f for f in frames if f['tier'] == tier and f['sheet'] == sheet):
            x, y, w, h = [frame[k] for k in ('x', 'y', 'w', 'h')]
            box = x, y, x+w, y+h
            name = frame['name']
            if not name.startswith('BackgroundLayers') and name not in allowed_alpha:
                a, b = baseline.crop(box).getchannel('A'), candidate.crop(box).getchannel('A')
                assert ImageChops.difference(a, b).getbbox() is None, (tier, name, 'alpha')
                report['preserved_alpha_frames'] += 1
                if name.startswith(('Characters', 'Obstacles', 'Coins', 'PZGameplay')):
                    report['physics_frames'] += 1
            ImageDraw.Draw(diff).rectangle((x, y, x+w-1, y+h-1), fill=(0,0,0,0))
        assert all(channel.getbbox() is None for channel in diff.split()), (tier, sheet, 'atlas padding')
        assert (ROOT / f'baseline/{tier}/{stem}.plist').read_bytes() == Path(f'Content/Images/{tier}/{stem}.plist').read_bytes()
        report['atlases'][f'{tier}/{stem}'] = hashlib.sha256(candidate_path.read_bytes()).hexdigest()
assert (ROOT / 'baseline/CompleteGameInfo.plist').read_bytes() == Path('Content/CompleteGameInfo.plist').read_bytes()
(ROOT / 'validation.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
