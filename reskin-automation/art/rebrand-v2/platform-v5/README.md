# Platform revision 5

Generated using the built-in image-generation editor with the original platform
as the edit target. Art direction: retain the long rectangular slab in shallow
oblique 2.5D perspective; charcoal asphalt top, thin worn concrete lip, pale warm
gray concrete front with pores and subtle joints, dark right face, gray left
drainage pipe. Preserve perspective and silhouette. Remove stripes, arcs, upright
panels, rust-colored caps, railings, and decorative road markings.

The generated source is cropped to 2152x558+0+80 and aligned to 1374x350 with
ImageMagick. The original platform alpha is copied as the final opacity channel.
The result replaces only platform rectangles in animation_s2 and animation_s3,
in both HD and SD. The generated source remains here for reproducibility.

Verification: original alpha compared independently against each of the six
platform crops from git HEAD; all match. Atlas dimensions remain 2048x2048.
Every pixel outside the six edited rectangles matches the pre-edit snapshot.
All Content/Images plist files match git HEAD byte for byte.

Visual review used the aligned sprite on a contrasting background. An in-game
capture of this revision has not been reviewed; runtime visual QA remains open.
