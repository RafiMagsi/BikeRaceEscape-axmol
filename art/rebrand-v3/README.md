# Dino Bike Escape — suburban canal reskin

## Direction

A weathered suburban stormwater district at blue hour. Slate-blue distance,
amber practical lights, gray asphalt/concrete, muted vegetation and petrol-teal
water. The rider and collectibles retain stronger contrast than the scenery.
Promotional artwork uses the same rider/dinosaur/canal story.

## Runtime layer contract

The order below is the actual BackgroundLayers array order, not the numeric
suffix in a sprite name. Existing transforms, speeds and source sizes are intact.

| z | Frame suffix | Role | Transparency |
|---:|---:|---|---|
| 0 | 0 | Sky and cloud atmosphere only | opaque |
| 10 | 1 | Distant suburban roofs and windows | open sky above roofs |
| 20 | 3 | Willow trees and shrubs | gaps between plants |
| 30 | 2 | Retaining wall, lamps and drains | opaque wall plane |
| 40 | 8 | Midground canal reeds | sparse clumps, open center |
| 50 | 5 | Water surface and broken reflections | opaque water plane |
| 60 | 6 | Scattered bank debris | isolated piles |
| 74/75 | — | Platforms / player | exact original silhouettes |
| 80 | 7 | Smaller near reeds | sparse clumps, open center |
| 90 | 4 | Closest broad leaves and ferns | dark edge clusters |

The original road slab revision 5 remains the platform's material and perspective
reference. Its alpha mask and geometry are retained across all three variants.
All rider, creature, obstacle and coin animation poses retain their original
pixels' positions and alpha. These sprites receive consistent color, contrast and
detail refinement, rather than independently generated poses that could drift.
Menu, character-select and coin-shop backgrounds share the new environment.
Play, pause, back, restart and unlock controls use the generated steel/slate kit.

## Artwork sources and prompt set

All sources were made with the built-in image-generation tool. Each scenery prompt
requested one isolated layer and explicitly excluded the contents of other layers.

- `sky.png`: seamless dusk sky only; low contrast slate blue clouds, no objects.
- `city.png`: transparent distant low-rise brick skyline, water tanks, amber windows;
  no wall, water, roads or sky.
- `wall.png`: continuous weathered concrete/brick drainage wall, circular grilles,
  rusty pipes and amber lamps; no sky, city, water or ground.
- `water.png`: shallow-oblique petrol-teal water only, fine broken reflections;
  no shore, horizon, objects or cartoon arcs.
- `trees.png`: transparent willow and deciduous shrub strip with gaps, cool dusk.
- `reeds.png`: transparent sparse canal reed clumps with seed heads, open center.
- `foreground.png`: transparent dark broad-leaf plants and ferns at lower edges.
- `debris.png`: transparent isolated pallet, tire, can, stone and cloth piles.
- `menu.png`: quiet canal environment, clear upper-center space for the title and
  controls; no baked-in characters or UI.
- `logo.png`: exact three-line DINO BIKE / ESCAPE / SEWER RUN wordmark, ivory,
  amber and steel, transparent background.
- `controls.png`: edit of the existing six-button contact sheet, preserving each
  symbol: pause/play/unlock/back/back/restart; steel rims, navy faces, ivory symbols.
- `ramp.png`: material-only edit of the wooden ramp, preserving wedge, corners and
  perspective; weathered boards, grain, nails and moss.
- `dirt.png`: material-only edit of the dirt ramp, preserving its irregular slope;
  compacted gravel, tire grooves and small stones.
- `hero.png`: promotional illustration of red motocross rider escaping a green
  dinosaur across the suburban canal, quiet left area for typography.
- `icon.png`: close rider/dinosaur portrait with red helmet, amber visor and sewer
  arches, full opaque square, no words or border.

Generated transparency is used for decorative scenery. UI controls use the original
alpha after alignment. No generated transparency is trusted for physics sprites.

## Tooling and recovery

`tools/reskin_game.mjs` uses ImageMagick and macOS plutil, with no new runtime
dependencies. `prepare` saves the current atlas baseline and extracts frames;
`assemble` renders candidates; `tools/validate_reskin.py` checks them;
`install` verifies candidate hashes against the report before copying them.

The local baseline, extracted frames and candidate atlas directories are ignored
to avoid duplicating large files in version control. They remain on disk for review
and recovery. Sources, manifest and validation report are retained.

`tools/build_reskin_store.mjs` exports the existing iOS and Android sizes. It uses
native AppKit text rendering through `tools/render_store_text.swift` because the
installed ImageMagick has no FreeType delegate. Captures retain aspect ratio and
are never stretched into a different device ratio.

See `validation.json` for exact candidate hashes and checks, and `captures/` for
runtime evidence. Store headers/icons are promotional artwork; screenshot panels
come from the actual running simulator, not generated gameplay.
