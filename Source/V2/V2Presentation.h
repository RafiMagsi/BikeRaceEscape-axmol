//
//  V2Presentation.h
//  Player-facing presentation preferences (PRODUCT_PLAN 21.12 / 21.15): Spectacle Full / Reduced / Off (decoration only) and
//  Motion Full / Reduced. They change decoration and camera motion, never difficulty, score, scoring rules or replays.
//
#pragma once

namespace V2 {
namespace Presentation {

enum class Spectacle { Full = 0, Reduced = 1, Off = 2 };

Spectacle spectacle();
void setSpectacle(Spectacle s);
Spectacle nextSpectacle(Spectacle s);
const char* spectacleName(Spectacle s);

bool reducedMotion();                      // the player's setting OR the OS Reduce Motion accessibility flag
bool systemReduceMotion();                 // OS flag only (iOS UIAccessibility; false elsewhere for now)
void setReducedMotion(bool on);

} // namespace Presentation
} // namespace V2
