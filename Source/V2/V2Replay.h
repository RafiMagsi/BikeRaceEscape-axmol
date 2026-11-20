//
//  V2Replay.h
//  Deterministic replay: records {seed, input events by simulation tick, state checkpoints} for fixed-step runs (daily /
//  challenge) and can play the inputs back into a fresh run while comparing the state at every checkpoint.
//
//  What it proves: the same seed + the same inputs give the same run (bit-exact checkpoints). Replaying a file recorded on
//  another platform measures cross-platform float drift. It is the foundation for ghosts and for server-side score checks.
//
//  Time base: ticks = fixed 1/60 s simulation slices actually simulated (intro, pause and hit-stop do not advance it).
//
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace V2 {
namespace Replay {

constexpr int kFormat = 1;
constexpr uint32_t kCheckpointEvery = 30;      // ticks (0.5 s)

struct Input { uint32_t tick = 0; bool press = false; };

struct Snapshot {                              // what a checkpoint captures (rider body + run counters + content stream)
    float x = 0, y = 0, vx = 0, vy = 0, distance = 0, speed = 0;
};

struct Checkpoint {
    uint32_t tick = 0;
    uint32_t bits[6] = {0, 0, 0, 0, 0, 0};     // float bit patterns of Snapshot
    uint32_t contentHash = 0;                  // PZRand::drawHash()
};

struct Data {
    int format = kFormat;
    int rulesVersion = 0;
    uint32_t seed = 0;
    uint32_t endTick = 0;
    bool ended = false;                        // true = the run ended (death) at endTick; false = stopped by the recorder
    std::vector<Input> inputs;
    std::vector<Checkpoint> checkpoints;
};

struct Report {
    bool finished = false;
    bool exact = false;                        // every checkpoint bit-identical and the end matched
    int checked = 0;
    int firstMismatchTick = -1;
    float maxDrift = 0.f;                      // largest |difference| of any captured float
    bool endMatched = true;
    std::string text;
};

enum class State { Idle, Recording, Playing };

// Lifecycle ---------------------------------------------------------------------------------------------------------------
void beginRun(uint32_t seed, bool fixedStep, bool liveNow);  // every run start; ticks only count once goLive() is called
void goLive();                                 // GO: the world is released, tick 0 starts here
bool live();
State state();
uint32_t tick();

// Called by the runner (fixed-step loop) ---------------------------------------------------------------------------------
bool onLiveInput(bool press);                  // returns true when the input should be applied now (always, unless playing)
void feedDueInputs(const std::function<void(bool)>& apply);   // start of a slice: playing -> apply inputs due at this tick
void afterSlice(const Snapshot& s, uint32_t contentHash);     // end of a slice: tick++, record or compare a checkpoint
void frame(bool gameOver);                     // once per rendered frame (kept for the stuck-run safety net)
void noteDeath();                              // the run's FIRST lethal signal (a physics event, so it has a deterministic tick)

// Control -----------------------------------------------------------------------------------------------------------------
void stopRecording();                          // manual end of a recording (keeps what was recorded)
const Data& lastRecording();
void armPlayback(const Data& d);               // the NEXT run start plays this back
bool playbackPending();
uint32_t pendingSeed();
void setResultHandler(std::function<void(const Report&)> h);
const Report& lastReport();

// Files -------------------------------------------------------------------------------------------------------------------
std::string serialize(const Data& d);
bool parse(const std::string& text, Data& out);
bool save(const Data& d, const std::string& path);
bool load(const std::string& path, Data& out);

} // namespace Replay
} // namespace V2
