# Sewer-city scene layout

This is the runtime scene contract for the rebrand. `PZGGameFieldScene` adds
`BackgroundLayers` in this order; each entry receives a z value of `array index × 10`.
The player is inserted at index 200 and therefore renders above the water/debris
layers and below the foreground grass layers.

| Array index | Layer | Atlas frame | Runtime role | Speed | Scale | Transparency contract |
|---:|---|---|---|---:|---:|---|
| 0 | sky | `BackgroundLayers_0_0` | deep sewer-city skyline / ambient light | 0.02 | 0.90 | original full-frame alpha |
| 1 | city buildings | `BackgroundLayers_1_0` | distant elevated city and pipe silhouettes | 0.17 | 0.878 | original sparse layer alpha |
| 3 | Untitled | `BackgroundLayers_3_0` | tunnel-wall silhouette / structural depth | 0.34 | 0.861 | original sparse layer alpha |
| 2 | Graffiti wall | `BackgroundLayers_2_0` | sewer wall and drainage architecture | 0.58 | 0.80 | original frame alpha |
| 8 | Midground Grass | `BackgroundLayers_4_0` | water-edge vegetation | 0.58 | 0.52 | original cutout alpha |
| 5 | Water | `BackgroundLayers_5_0` | toxic canal / water bridge context | 0.85 | 0.89 | original full-frame alpha |
| 6 | Trash | `BackgroundLayers_6_0` | floating debris and foreground clutter | 0.84 | 0.475 | original scattered cutout alpha |
| 200 | Player | runtime node | gameplay layer | — | — | unchanged |
| 7 | Small Grass | `BackgroundLayers_7_0` | near water-edge grass | 1.22 | 0.656 | original cutout alpha |
| 4 | Big Grass Foreground | `BackgroundLayers_8_0` | closest foreground occlusion | 1.25 | 1.29 | original cutout alpha |

## Art rules used

- Backgrounds are not treated as opaque rectangles. Transparent decorative layers
  remain sparse cutouts, so pipes, wall lights, water, trash, and grass compose as
  separate parallax planes.
- Platform replacements are rendered inside the original 1374×350 visible frame
  and use the original platform alpha mask exactly. The current platform reads as
  an asphalt/concrete slab: a pale front face and shaded right face contrast against the cool
  green-blue water, while the collision surface stays at the original visible row.
- No frame dictionary, source size, source color rect, offset, or rotated value was
  edited. The atlas pass changes pixels only.

## Design intent

The player reads as travelling through a suburban sewer beneath a dense city:
the skyline establishes the outside world, the drainage wall establishes the
play-space, toxic water creates the color anchor, and debris/grass provide the
foreground depth cues. This preserves the original gameplay composition while
giving the game a coherent new setting.
