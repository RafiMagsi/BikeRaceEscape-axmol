#include "V2/V2Replay.h"
#include "V2/V2Daily.h"
#include "PZLegacyCompat.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sstream>

USING_NS_AX;

namespace V2 {
namespace Replay {

namespace {
State g_state = State::Idle;
uint32_t g_tick = 0;
Data g_rec;                    // the recording in progress / last finished recording
Data g_play;                   // the file being played
Report g_report;
bool g_pending = false;
size_t g_inIdx = 0, g_cpIdx = 0;
bool g_finished = true;
bool g_live = false;
std::function<void(const Report&)> g_handler;

uint32_t bitsOf(float f) { uint32_t u; std::memcpy(&u, &f, 4); return u; }
float floatOf(uint32_t u) { float f; std::memcpy(&f, &u, 4); return f; }

void fillBits(Checkpoint& c, const Snapshot& s) {
    c.bits[0] = bitsOf(s.x); c.bits[1] = bitsOf(s.y); c.bits[2] = bitsOf(s.vx);
    c.bits[3] = bitsOf(s.vy); c.bits[4] = bitsOf(s.distance); c.bits[5] = bitsOf(s.speed);
}

void conclude() {
    if (g_state != State::Playing || g_report.finished) return;
    g_report.finished = true;
    g_report.exact = g_report.firstMismatchTick < 0 && g_report.endMatched && g_report.checked > 0;
    char buf[200];
    std::snprintf(buf, sizeof(buf), "checked=%d exact=%d firstMismatchTick=%d maxDrift=%g endMatched=%d",
                  g_report.checked, g_report.exact ? 1 : 0, g_report.firstMismatchTick, g_report.maxDrift,
                  g_report.endMatched ? 1 : 0);
    g_report.text = buf;
    AXLOGI("[V2:replay] RESULT {}", g_report.text);
    g_finished = true;
    if (g_handler) g_handler(g_report);
}
} // namespace

State state() { return g_state; }
uint32_t tick() { return g_tick; }
const Data& lastRecording() { return g_rec; }
void setResultHandler(std::function<void(const Report&)> h) { g_handler = std::move(h); }
const Report& lastReport() { return g_report; }
bool playbackPending() { return g_pending; }
uint32_t pendingSeed() { return g_play.seed; }

void armPlayback(const Data& d) {
    g_play = d;
    g_pending = true;
}

bool live() { return g_live; }
void goLive() { if (g_state != State::Idle) { g_live = true; g_tick = 0; } }

void beginRun(uint32_t seed, bool fixedStep, bool liveNow) {
    g_tick = 0;
    g_live = false;
    g_inIdx = g_cpIdx = 0;
    g_report = Report();
    if (g_pending) {
        g_pending = false;
        g_state = State::Playing;
        g_finished = false;
        AXLOGI("[V2:replay] playing back seed={} inputs={} checkpoints={}", g_play.seed, g_play.inputs.size(), g_play.checkpoints.size());
        return;
    }
    if (fixedStep) {
        g_state = State::Recording;
        g_finished = false;
        g_rec = Data();
        g_rec.seed = seed;
        g_rec.rulesVersion = Daily::kRulesVersion;
    } else {
        g_state = State::Idle;
    }
    if (liveNow) goLive();
}

bool onLiveInput(bool press) {
    if (g_state == State::Playing) return false;               // a replay is the only source of input
    if (g_state == State::Recording && g_live && !g_finished) g_rec.inputs.push_back({g_tick, press});
    return true;
}

void feedDueInputs(const std::function<void(bool)>& apply) {
    if (g_state != State::Playing || g_finished || !g_live) return;
    while (g_inIdx < g_play.inputs.size() && g_play.inputs[g_inIdx].tick <= g_tick) {
        apply(g_play.inputs[g_inIdx].press);
        ++g_inIdx;
    }
}

void afterSlice(const Snapshot& s, uint32_t contentHash) {
    if (g_state == State::Idle || g_finished || !g_live) return;
    ++g_tick;
    if (g_tick % kCheckpointEvery != 0) return;
    Checkpoint c;
    c.tick = g_tick;
    fillBits(c, s);
    c.contentHash = contentHash;
    if (g_state == State::Recording) {
        g_rec.checkpoints.push_back(c);
        return;
    }
    // playing: compare against the recorded checkpoint with the same tick
    while (g_cpIdx < g_play.checkpoints.size() && g_play.checkpoints[g_cpIdx].tick < c.tick) ++g_cpIdx;
    if (g_cpIdx >= g_play.checkpoints.size() || g_play.checkpoints[g_cpIdx].tick != c.tick) return;
    const Checkpoint& e = g_play.checkpoints[g_cpIdx++];
    ++g_report.checked;
    bool same = e.contentHash == c.contentHash;
    for (int i = 0; i < 6; ++i) {
        if (e.bits[i] != c.bits[i]) {
            same = false;
            g_report.maxDrift = std::max(g_report.maxDrift, std::fabs(floatOf(e.bits[i]) - floatOf(c.bits[i])));
        }
    }
    if (!same && g_report.firstMismatchTick < 0) {
        g_report.firstMismatchTick = static_cast<int>(c.tick);
        AXLOGW("[V2:replay] first mismatch at tick {}: x {}/{} y {}/{} vx {}/{} vy {}/{} dist {}/{} spd {}/{} hash {}/{} (expected/got)", c.tick,
               floatOf(e.bits[0]), floatOf(c.bits[0]), floatOf(e.bits[1]), floatOf(c.bits[1]), floatOf(e.bits[2]), floatOf(c.bits[2]),
               floatOf(e.bits[3]), floatOf(c.bits[3]), floatOf(e.bits[4]), floatOf(c.bits[4]), floatOf(e.bits[5]), floatOf(c.bits[5]),
               e.contentHash, c.contentHash);
    }
    if (g_play.endTick > 0 && !g_play.ended && g_tick >= g_play.endTick) conclude();   // recorder stopped here: nothing more to compare
}

// The death of the rider is detected inside a physics step (contact callback), but its handling is deferred to a later frame
// in real time, so the number of extra slices that run in between is NOT deterministic. The end of a run is therefore the
// tick of the first lethal signal, not the frame in which game over is processed.
void noteDeath() {
    if (g_state == State::Idle || g_finished || !g_live) return;
    if (g_state == State::Recording) {
        g_rec.ended = true;
        g_rec.endTick = g_tick;
        g_finished = true;
        AXLOGI("[V2:replay] recorded seed={} ticks={} inputs={} checkpoints={}", g_rec.seed, g_tick, g_rec.inputs.size(), g_rec.checkpoints.size());
        return;
    }
    g_report.endMatched = g_play.ended && g_tick == g_play.endTick;     // the replay must die on exactly the recorded tick
    conclude();
}

void frame(bool gameOver) {
    if (g_state != State::Playing || g_finished) return;
    // safety net: the recording died but this replay is still alive well after that tick
    if (g_play.ended && g_tick > g_play.endTick + 30) { g_report.endMatched = false; conclude(); }
    else if (gameOver && !g_play.ended) { g_report.endMatched = false; conclude(); }
}

void stopRecording() {
    if (g_state != State::Recording || g_finished) return;
    g_rec.ended = false;
    g_rec.endTick = g_tick;
    g_finished = true;
    AXLOGI("[V2:replay] recording stopped seed={} ticks={} inputs={} checkpoints={}", g_rec.seed, g_tick, g_rec.inputs.size(), g_rec.checkpoints.size());
}

// ---- files ----------------------------------------------------------------------------------------------------------------

std::string serialize(const Data& d) {
    std::ostringstream o;
    o << "V2R " << d.format << " " << d.rulesVersion << " " << d.seed << " " << d.endTick << " " << (d.ended ? 1 : 0) << "\n";
    for (const auto& i : d.inputs) o << "I " << i.tick << " " << (i.press ? 1 : 0) << "\n";
    char buf[160];
    for (const auto& c : d.checkpoints) {
        std::snprintf(buf, sizeof(buf), "C %u %08x %08x %08x %08x %08x %08x %08x\n", c.tick, c.bits[0], c.bits[1], c.bits[2],
                      c.bits[3], c.bits[4], c.bits[5], c.contentHash);
        o << buf;
    }
    return o.str();
}

bool parse(const std::string& text, Data& out) {
    out = Data();
    std::istringstream in(text);
    std::string tag;
    bool head = false;
    while (in >> tag) {
        if (tag == "V2R") {
            int ended = 0;
            in >> out.format >> out.rulesVersion >> out.seed >> out.endTick >> ended;
            out.ended = ended != 0;
            head = out.format == kFormat;
        } else if (tag == "I") {
            Input i; int p = 0;
            in >> i.tick >> p;
            i.press = p != 0;
            out.inputs.push_back(i);
        } else if (tag == "C") {
            Checkpoint c;
            in >> c.tick;
            for (int k = 0; k < 6; ++k) in >> std::hex >> c.bits[k];
            in >> std::hex >> c.contentHash >> std::dec;
            out.checkpoints.push_back(c);
        } else {
            return false;
        }
    }
    return head && !out.checkpoints.empty();
}

bool save(const Data& d, const std::string& path) {
    return FileUtils::getInstance()->writeStringToFile(serialize(d), path);
}

bool load(const std::string& path, Data& out) {
    const std::string text(FileUtils::getInstance()->getStringFromFile(path));
    return !text.empty() && parse(text, out);
}

} // namespace Replay
} // namespace V2
