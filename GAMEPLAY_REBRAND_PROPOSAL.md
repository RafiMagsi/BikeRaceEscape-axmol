# Gameplay rebrand proposal

## Safe scope for this release

Change only the visual background layers and store-facing art. Keep the player, monsters, coins, platforms, collision points, and atlas metadata unchanged.

## Why the constraint matters

- `PZGPlatfrom::initPhysics()` derives the platform fixture from the configured platform width and collision points.
- `PZGGamefieldJumpingPlatforms::addPlatform()` derives placement and bounds from `sprite->getContentSize()`.
- `PZGObstacle::initPhysics()` falls back to the sprite content size when collision points are absent, and otherwise offsets collision vertices from that same size.
- Runtime resources are atlas-backed by `animation_s0..s3.plist`; changing frame rectangles or source sizes can change physics indirectly.

## Recommended visual direction

1. Keep the existing sprite sheets as the gameplay contract.
2. Replace only `BackgroundLayers_*` frame pixels, preserving each plist `frame`, `sourceColorRect`, `sourceSize`, `offset`, and `rotated` value byte-for-byte.
3. Use a layered sewer-city treatment: dark navy skyline, cyan moonlight, orange tunnel lights, and toxic-green water accents. This matches the new store key art without changing gameplay readability.
4. Do not repaint platform frames in the same pass unless each replacement is rendered inside the original frame rectangle and the visible collision surface remains at the same pixel row.

## Release gate

Before promoting runtime art into `Content/Images/{HD,SD}`:

- compare every plist frame dictionary before/after;
- assert atlas dimensions remain `2048x2048`;
- assert every non-background frame's alpha bounds and source size are unchanged;
- run the game through platform spawn, obstacle collision, coin pickup, death, and restart flows;
- capture at least one HD and one SD gameplay screenshot for visual QA.

The current implementation stops before that promotion step because generated key art is marketing art, not a deterministic atlas replacement. That is the correct boundary: forcing it into the runtime sheet would risk changing physics.
