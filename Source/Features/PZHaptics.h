//
//  PZHaptics.h
//  Cross-platform haptic feedback (iOS UIKit feedback generators, Android VibrationEffect).
//  Respects the player's VIBRATION setting (PZProgress) and throttles bursts of the same kind.
//
#pragma once

namespace PZHaptics {

enum class Kind {
    Tick = 0,      // tiny, for rapid events (coin pickup)
    Light = 1,     // jump, small hits
    Medium = 2,    // power-up pickup, close call, landing
    Heavy = 3,     // smash, shield break, blast
    Success = 4,   // stage up, revive, reward claimed
    Warning = 5,   // danger moments
    Error = 6,     // death
    Selection = 7  // UI taps
};

void play(Kind kind);

} // namespace PZHaptics

// Implemented per platform (PZHapticsApple.mm, PZHaptics.cpp for Android). No-op elsewhere.
void PZHaptics_native(int kind);
