//
//  PZLayout.h
//  One place that knows where UI may live: inside the device safe area (notch / dynamic island / home
//  indicator) and ABOVE the AdMob banner strip. Both the plist-driven interfaces (PZGBaseMenuScene) and the
//  code-drawn feature UI read from here, so nothing ever ends up under the ad or the camera cut-out.
//
#pragma once

#include "axmol.h"

namespace PZLayout {

// Design units reserved along the bottom edge for the banner ad (0 once ads are removed).
float bannerReserve();

// Usable rectangle in design coordinates: safe area minus the banner strip.
ax::Rect safeRect();

// Compresses a design-space Y so the full authored layout fits above the banner:
// 0..H  ->  bannerReserve..H. Leaves the top of the screen essentially untouched.
float mapY(float y);

// Slides `node` (using its bounding box in its parent's space) horizontally inside the safe rect and
// vertically above the banner. Only ever moves it when it would otherwise be clipped or covered.
void keepInside(ax::Node* node);

// Centre of the area above the banner (where popups are centred).
ax::Vec2 popupCentre();

// Maximum height a popup panel may use.
float popupMaxHeight();

} // namespace PZLayout
