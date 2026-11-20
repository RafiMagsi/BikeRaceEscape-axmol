#include "PZRandom.h"
#include "V2/V2Core.h"

namespace PZRand {
namespace {
V2::Rng g_rng(1u);
uint32_t g_seed = 1u;
bool g_fixed = false;
uint32_t g_hash = 2166136261u;
uint32_t draw() {
    const uint32_t v = g_rng.next();
    g_hash = (g_hash ^ v) * 16777619u;
    return v;
}
}

void setRunSeed(uint32_t seed) { g_seed = seed ? seed : 1u; }
uint32_t runSeed() { return g_seed; }
void applyRunSeed() { g_rng.reseed(g_seed); g_hash = 2166136261u; }
uint32_t drawHash() { return g_hash; }
float unit() { return static_cast<float>(draw() & 0xFFFFFFu) / 16777216.f; }
int raw() { return static_cast<int>(draw() & 0x7FFFFFFFu); }
void setFixedStep(bool on) { g_fixed = on; }
bool fixedStep() { return g_fixed; }

} // namespace PZRand
