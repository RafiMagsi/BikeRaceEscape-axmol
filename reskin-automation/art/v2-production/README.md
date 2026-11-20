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

The generated source canvas is intentionally not the runtime packing contract.
The builder segments approved subjects, fits them into the existing registered
frames with transparent gutters, and preserves the atlas plists unchanged.
