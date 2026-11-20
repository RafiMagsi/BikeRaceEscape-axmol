# V2 art replacement helpers

Use `tools/art_pipeline.py` for new obstacle and V2 art work. It replaces the
unsupported experiments in `reskin-automation/automation/`, which require an
uninstalled OpenCV package and emit center-relative collision points that do not
match `PZGObstacle::initPhysics`.

The supported tool uses OpenCV for contour extraction/convex simplification and
Pillow for atlas IO and review images. Both are installed into the ignored local
art environment; they are not game runtime dependencies.

Set up or refresh the environment:

```sh
sh tools/setup_art_env.sh
```

Use `.venv-art/bin/python` for every command below. The tool is read-only unless
`trace` receives a plist target and explicit `--apply`.

## Common commands

Generate a five-point collision polygon and a review overlay:

```sh
.venv-art/bin/python tools/art_pipeline.py trace work/barrel.png \
  --threshold 15 --inset 2 --overlay work/barrel-collision.png
```

Preview a collision remap for obstacle index 4 without modifying the plist:

```sh
.venv-art/bin/python tools/art_pipeline.py trace work/barrel.png \
  --overlay work/barrel-collision.png \
  --plist Content/CompleteGameInfo.plist --obstacle-index 4
```

After visual review, apply that exact remap:

```sh
.venv-art/bin/python tools/art_pipeline.py trace work/barrel.png \
  --overlay work/barrel-collision.png \
  --plist Content/CompleteGameInfo.plist --obstacle-index 4 --apply
```

Compare a candidate with its baseline. Use the strict flags for a repaint that
must not change gameplay geometry:

```sh
.venv-art/bin/python tools/art_pipeline.py compare work/old.png work/new.png \
  --require-same-canvas --require-same-bounds
```

Generate a machine-readable atlas report or a compact reusable art brief:

```sh
.venv-art/bin/python tools/art_pipeline.py atlas Content/Images/HD/animation_s3.plist \
  --prefix Obstacles_ --output work/obstacles.json

.venv-art/bin/python tools/art_pipeline.py brief \
  Content/Images/HD/v2_events.plist rider_cool_0
```

Run the helper tests:

```sh
.venv-art/bin/python tools/test_art_pipeline.py
```

## Safety contract

- Collision output uses absolute sprite pixels with a bottom-left origin.
- The tool always emits the five values consumed by the current obstacle code.
- Alpha tracing gives a starting hull, not automatic design approval. Review the
  overlay; visible decoration, smoke, antennas, and tiny fragments should not
  necessarily be collidable.
- Use a small inset for fair hazards and compare the result with existing replay
  and collision-debug evidence.
- Never apply collision remapping to cosmetic V2 actors or background dinosaurs.
- Keep a gameplay-art change and a physics change as separate commits whenever
  practical so a collision regression can be isolated and reverted.
