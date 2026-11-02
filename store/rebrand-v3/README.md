# Current store kit

The current reskin is in `art/rebrand-v3`. Use this kit for review; v2 is historical.

- `header-1920x1080.png` and `header-3840x1646.png`: illustrated promotional headers.
- `feature-graphic-1024x500.png`: promotional banner.
- `icon-1024.png` and `icon-512.png`: new rider/dinosaur icon, also exported into
  existing app icon sets and platform store directories.
- `../ios/search_results/`: iOS search-results artwork in the supported
  1920x1280, 3840x2560 and 5244x2950 sizes, with PNG masters and upload-friendly
  JPEG copies. These use the exact game logo over a crop-aware sewer chase scene.
- `listing/`: five captioned layouts using real updated simulator captures.

The same listing designs are exported to the existing iOS 6.9-inch, 6.3-inch and
iPad 13-inch directories, and Android phone, 7-inch and 10-inch tablet directories.
PNG and JPEG variants are supplied. Existing dimensions are preserved.

The first image presents the menu and setting; the next three present jumps,
coins and bike selection; the last presents the sewer hazards. Images use short
headings, generous readable gameplay panels and a quiet slate background.

Build: `node tools/build_reskin_store.mjs`. Capture selection is explicit in
`art/rebrand-v3/captures/selected.json`. See `art/rebrand-v3/QA.md` for provenance
and runtime verification limits. These files have not been submitted or published.
