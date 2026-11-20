# V2 production art sources

These transparent source sheets were generated with the built-in image-generation
tool and are repacked deterministically by `tools/build_v2_production_art.py`.
Runtime atlases retain the frame names and rectangles already registered through
`Content/CompleteGameInfo.plist`; source sheets are never loaded by the game.

Style target: cinematic painted arcade realism matching the shipped bike/rider,
platforms, sewer parallax backgrounds, gold coins, and navy/orange HUD. The old
flat `v2_events`, portrait, and scenic sheets are layout/identity references only.

Source roles:

- `events/events-painted-source.png`: event dinosaurs, riders and props.
- `workers/workers-painted-source.png`: STOP/GO construction worker poses omitted
  by the first event-sheet generation.
- `portraits/portraits-painted-source.png`: four expressions each for player and
  Cool Kid.
- `scenic/scenic-painted-source.png`: two-frame distant walker and wall-peek T-Rex.
- `obstacles/barrel/barrel-painted-source.png`: first collision-sensitive legacy
  hurdle vertical slice; repacked into the original 200x200 logical canvas and
  original `Obstacles_4_0` atlas frame before OpenCV collision review.
- `obstacles/all/creatures-painted-source.png`: eight-frame alligator, eight-frame
  lizard and five-frame rat cycles. Their 21 existing frame ids remain unchanged.
- `obstacles/all/hurdles-painted-source.png`: light barrier off/on, launch ramp,
  fence, grind rail and dirt ramp. The seventh generated delivery-crate concept
  is deliberately unused because it has no matching registered collision contract.
- `obstacles/all/logical/`: first-frame logical canvases and reviewed OpenCV
  collision overlays for every obstacle family except the separately documented barrel.
- `ui/screens-painted-source.png`: coherent sewer-metal results, pause, shop,
  upgrade, kids-mode and tutorial elements. Existing readable circular controls,
  bikes, platforms, parallax backgrounds, coins and HUD art are intentionally preserved.

The generated source canvas is intentionally not the runtime packing contract.
The builder segments approved subjects, fits them into the existing registered
frames with transparent gutters, and preserves the atlas plists unchanged.

## Built-in ImageGen prompts

The production sources were generated in built-in tool mode with transparent
backgrounds. The final prompt families were:

1. **Creatures:** exactly 21 isolated left-facing sprites—8 mutant sewer
   alligator run frames, 8 distinct monitor-lizard run frames and 5 scrappy rat
   run frames—in cinematic painted arcade realism, cool sewer rim light, strong
   phone-scale silhouettes, no gore, text, scenery, overlap or cropped anatomy.
2. **Hurdles:** exactly 7 isolated side-view concepts—barrier off/on, steel
   launch ramp, bent municipal fence, grind rail, rubble ramp and delivery
   crates—matching the bike/platform material language with restrained orange
   safety accents and collision-readable silhouettes.
3. **Screen UI:** exactly 10 isolated navy/steel/orange plates with the verbatim
   labels GAME OVER, PAUSED, COIN SHOP, UPGRADE BIKE, KIDS MODE, RIDE, RESTORE,
   REMOVE ADS, TAP TO JUMP and HOLD TO FLIP; premium arcade construction, high
   contrast, transparent background and no extra branding.
