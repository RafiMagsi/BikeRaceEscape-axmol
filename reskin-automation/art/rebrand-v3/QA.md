# Visual and physics verification

- 8 atlas candidates remain 2048x2048.
- 146 physics-related frames (73 per tier: characters, obstacles, coins and
  platforms) retain every original alpha pixel, not just bounding boxes.
- 210 non-scenery frame masks are preserved, including the updated controls.
- Every atlas plist and CompleteGameInfo.plist is byte-identical to the saved
  baseline. Frame/source rectangles, offsets, rotation, animation lengths,
  collision vertices, speeds and layer transforms are untouched.
- Atlas padding and all pixels outside the declared frame rectangles are intact.
- Installed candidates are hash-checked against validation.json.

## Runtime review

Captured the actual iPhone 16 Pro and iPad Pro 13-inch simulator render buffers.
The iPhone run exercised main menu, bike selection, jumps, ramps, coin pickup,
and game over. The iPad review exercised menus, initial gameplay and game over;
the unattended jump timing did not clear its first gap.

The temporary simulator-only capture hook invoked the existing menu and jump
callbacks. It did not change physics or award currency. It was removed from
source after capture. Concurrent landing-page diagnostics, ads and audio changes
belong to the other workspace work and were preserved.

The initial Xcode simulator build succeeded. A later regeneration exposed missing
libpng NEON objects in the local build archive. The existing upstream ARM sources
were compiled into the local archive and the generated link command was rerun
successfully for the review binary. No engine source or dependency version was
changed. A clean release build is not certified by this art pass.

## Store image provenance

Headers, icon and feature graphic are promotional illustrations. Listing panels
use unaltered rendered game captures with added captions and margins. The supplied
phone captures are fitted, without stretching, into the existing phone and tablet
marketing dimensions. These are store compositions, not claims of separate native
Android or tablet captures. Raw iPad captures are retained for comparison.

No assets were uploaded to a store and no release was published.
