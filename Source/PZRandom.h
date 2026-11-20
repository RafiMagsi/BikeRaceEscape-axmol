//
//  PZRandom.h
//  A dedicated, seedable random stream for CONTENT GENERATION (platforms, obstacle and coin placement). Cosmetic effects
//  keep using std::rand, so they can never change what the level looks like. The run director sets the seed at the start
//  of every run: a challenge / daily seed gives identical content; a normal run uses a time-based seed.
//
//  Fixed-step mode: when on, the runner simulates in 1/60 s slices regardless of frame rate, which (with the seeded
//  content) makes a run reproducible from its seed + input log (ghosts, daily challenges).
//
#pragma once

#include <cstdint>

namespace PZRand {

void setRunSeed(uint32_t seed);     // remembered; applied by applyRunSeed()
uint32_t runSeed();
void applyRunSeed();                // restart the content stream from the run seed (called when the world is (re)filled)
uint32_t drawHash();                // FNV hash of every value drawn since applyRunSeed (determinism check)
float unit();                       // [0,1)
int raw();                          // non-negative, like rand()
void setFixedStep(bool on);
bool fixedStep();
constexpr float kFixedDt = 1.0f / 60.0f;

} // namespace PZRand
