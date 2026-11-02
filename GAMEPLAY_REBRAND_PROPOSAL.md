# Gameplay rebrand proposal

## Safe scope for this release

Change only the visual background layers, the platform pixels inside their existing
frames, and store-facing art. Keep the player, monsters, coins, collision points,
frame rectangles, source sizes, and atlas metadata unchanged. The scene order and
layer contract are documented in `art/rebrand-v2/SCENE_LAYOUT.md`.

## Why the constraint matters

- `PZGPlatfrom::initPhysics()` derives the platform fixture from the configured platform width and collision points.
- `PZGGamefieldJumpingPlatforms::addPlatform()` derives placement and bounds from `sprite->getContentSize()`.
- `PZGObstacle::initPhysics()` falls back to the sprite content size when collision points are absent, and otherwise offsets collision vertices from that same size.
- Runtime resources are atlas-backed by `animation_s0..s3.plist`; changing frame rectangles or source sizes can change physics indirectly.

## Recommended visual direction

1. Keep the existing sprite sheets as the gameplay contract.
2. Replace only `BackgroundLayers_*` frame pixels, preserving each plist `frame`, `sourceColorRect`, `sourceSize`, `offset`, and `rotated` value byte-for-byte.
3. Use a layered sewer-city treatment: dark navy skyline, cyan moonlight, orange tunnel lights, and toxic-green water accents. This matches the new store key art without changing gameplay readability.
4. Treat transparent tree/grass/trash frames as cutout layers, not full backgrounds. Their original alpha silhouettes must be retained so z-order and parallax remain meaningful.
5. Platform frames may be repainted only inside the original frame rectangle, with the original alpha mask and visible collision surface row preserved. The current platform is an asphalt/concrete rectangular slab in the original shallow oblique perspective, with a pale front and darker right face; water remains a separate cool-green material.

## Release gate

Before promoting runtime art into `Content/Images/{HD,SD}`:

- compare every plist frame dictionary before/after;
- assert atlas dimensions remain `2048x2048`;
- assert every non-background frame's alpha bounds and source size are unchanged;
- run the game through platform spawn, obstacle collision, coin pickup, death, and restart flows;
- capture at least one HD and one SD gameplay screenshot for visual QA.

The current atlas pass is installed in Content/Images: the background regions use the
new sewer-city art, transparent foreground layers remain transparent cutouts, and
the platform regions use an asphalt/concrete treatment built from the original platform
alpha mask. HD and SD atlases remain 2048×2048, all replaced alpha masks compare
identically to the originals, and the `.plist` metadata remains unchanged.
