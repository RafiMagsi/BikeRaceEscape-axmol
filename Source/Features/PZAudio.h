//
//  PZAudio.h
//  Sound effects and voice lines for the action layer. Files live in Content/Sounds as
//  fx_<name>.mp3 (effects) and vo_<name>.mp3 (voice). A missing file is silently skipped, so a reskin can replace,
//  remove or add sounds without touching code. Volumes and the on/off switches live in PZGameConfig.h
//  (features/EnableSfx, EnableVoice, tuning/SfxVolume, VoiceVolume).
//
#pragma once

#include <string>

namespace PZAudio {

// Plays Content/Sounds/<name>.mp3 once. Effects with the same name are rate limited (default 60 ms).
void play(const std::string& name, float volume = 1.f, float minGapSeconds = 0.06f);

// Voice line (vo_<name>): one voice at a time; a new line only interrupts when `priority` is not lower.
void say(const std::string& name, int priority = 1);

// Plays the voice line bound to an on-screen label ("GO!", "SECOND CHANCE!", "STAGE 3", ...). Called by PZUI::banner so
// every label announces itself without per-call-site code. Unknown labels are silent.
void sayLabel(const std::string& text);

// Loads (decodes) the common sounds so the first use is instant.
void preload();

} // namespace PZAudio
