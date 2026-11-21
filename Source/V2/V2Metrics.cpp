#include "V2/V2Metrics.h"
#include "V2/V2Core.h"
#include "V2/V2Config.h"
#include "V2/V2Content.h"

#include <algorithm>
#include <chrono>

namespace V2 {
namespace Metrics {

namespace {
using Clock = std::chrono::steady_clock;
Clock::time_point g_launch = Clock::now(), g_runStart, g_control, g_lastEnd, g_death;
bool g_hasDeath = false;
bool g_firstEventLogged = false;
bool g_live = false, g_hadControl = false, g_launchLogged = false, g_hasEnd = false;
int g_runIndex = 0, g_frames = 0;
int g_b[5] = {0, 0, 0, 0, 0};            // <=17 ms, <=25, <=34, <=50, >50
float g_worst = 0.f;
std::string g_mode = "normal";

float since(Clock::time_point t) { return std::chrono::duration<float>(Clock::now() - t).count(); }
std::string f1(float v) { char b[24]; std::snprintf(b, sizeof(b), "%.1f", v); return b; }
}

void deathNoted() { g_death = Clock::now(); g_hasDeath = true; }

void markLaunch() { g_launch = Clock::now(); g_launchLogged = false; }

void runStarted(const std::string& mode) {
    if (g_live) runEnded(0, 0, 0, "abandoned");        // the previous run never reached the results screen (left the scene / restarted)
    ++g_runIndex;
    g_firstEventLogged = false;
    g_mode = mode;
    g_runStart = Clock::now();
    g_live = true;
    g_hadControl = false;
    g_frames = 0;
    g_worst = 0.f;
    for (int& b : g_b) b = 0;
    Analytics::shared().log("run_start", {{"n", std::to_string(g_runIndex)}, {"mode", mode},
                                          {"retry_gap_s", g_hasEnd ? f1(since(g_lastEnd)) : std::string("-")},
                                          {"rules", std::to_string(cfg().rulesVersion)}, {"content", std::to_string(Content::shared().contentVersion())}});
}

void controlGranted() {
    if (!g_live || g_hadControl) return;
    g_hadControl = true;
    g_control = Clock::now();
    Analytics::shared().log("run_control", {{"n", std::to_string(g_runIndex)}, {"intro_s", f1(since(g_runStart))},
                                            {"death_to_control_s", (g_runIndex > 1 && g_hasDeath) ? f1(std::chrono::duration<float>(Clock::now() - g_death).count()) : std::string("-")},
                                            {"launch_to_control_s", g_launchLogged ? std::string("-") : f1(since(g_launch))}});
    g_launchLogged = true;
}

void firstEvent() {
    if (!g_live || !g_hadControl || g_firstEventLogged) return;
    g_firstEventLogged = true;
    Analytics::shared().log("first_event", {{"n", std::to_string(g_runIndex)}, {"after_s", f1(since(g_control))}});
}

void frame(float realDt) {
    if (!g_live || !g_hadControl || realDt <= 0.f) return;
    const float ms = realDt * 1000.f;
    ++g_frames;
    g_worst = std::max(g_worst, ms);
    ++g_b[ms <= 17.f ? 0 : (ms <= 25.f ? 1 : (ms <= 34.f ? 2 : (ms <= 50.f ? 3 : 4)))];
}

void runEnded(int distance, int eventsCleared, int coins, const std::string& cause) {
    if (!g_live) return;
    g_live = false;
    g_hasEnd = true;
    g_lastEnd = Clock::now();
    Analytics::shared().log("run_end", {{"n", std::to_string(g_runIndex)}, {"mode", g_mode}, {"cause", cause},
                                        {"death_to_results_s", g_hasDeath ? f1(std::chrono::duration<float>(g_lastEnd - g_death).count()) : std::string("-")},
                                        {"distance", std::to_string(distance)}, {"events", std::to_string(eventsCleared)},
                                        {"coins", std::to_string(coins)},
                                        {"duration_s", g_hadControl ? f1(std::chrono::duration<float>(g_lastEnd - g_control).count()) : std::string("0")},
                                        {"frames", std::to_string(g_frames)},
                                        {"ft_le17", std::to_string(g_b[0])}, {"ft_le25", std::to_string(g_b[1])}, {"ft_le34", std::to_string(g_b[2])},
                                        {"ft_le50", std::to_string(g_b[3])}, {"ft_gt50", std::to_string(g_b[4])}, {"ft_worst_ms", f1(g_worst)}});
}

} // namespace Metrics
} // namespace V2
