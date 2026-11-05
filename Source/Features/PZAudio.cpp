#include "PZAudio.h"
#include "PZGameConfig.h"
#include "PZLegacyCompat.h"

#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <vector>

namespace PZAudio {

namespace {

using Clock = std::chrono::steady_clock;

std::string pathFor(const std::string& name) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    return "Sounds/" + name + ".mp3";
#else
    return name + ".mp3";     // iOS/Mac: the app adds Content/Sounds to the search paths
#endif
}

bool exists(const std::string& path) {
    static std::unordered_map<std::string, bool> cache;
    auto it = cache.find(path);
    if (it != cache.end()) return it->second;
    const bool ok = ax::FileUtils::getInstance()->isFileExist(path);
    cache.emplace(path, ok);
    return ok;
}

std::unordered_map<std::string, Clock::time_point>& lastPlayed() {
    static std::unordered_map<std::string, Clock::time_point> m;
    return m;
}

int g_voiceId = -1;       // ax::AudioEngine id of the current voice line
int g_voicePriority = 0;

struct Binding { std::string prefix; const char* voice; };

// Label text (format strings are cut at the first '%') -> voice line. Longest prefix wins.
const std::vector<Binding>& bindings() {
    static std::vector<Binding> b;
    if (!b.empty()) return b;
    auto add = [](const char* text, const char* voice) {
        std::string p = text ? text : "";
        const auto cut = p.find('%');
        if (cut != std::string::npos) p.erase(cut);
        while (!p.empty() && p.back() == ' ') p.pop_back();
        if (!p.empty()) b.push_back({p, voice});
    };
    add(PZCfg::kStrGo, "vo_go");
    add(PZCfg::kStrSaved, "vo_second_chance");
    add(PZCfg::kStrGetReady, "vo_get_ready");
    add(PZCfg::kStrShieldBreak, "vo_shield");
    add(PZCfg::kStrBlast, "vo_blast");
    add(PZCfg::kStrJetpack, "vo_jetpack");
    add(PZCfg::kStrRocket, "vo_rocket");
    add(PZCfg::kStrGun, "vo_gun");
    add(PZCfg::kStrLuckyHelmet, "vo_helmet");
    add(PZCfg::kStrNewBest, "vo_new_best");
    add(PZCfg::kStrStage, "vo_stage");
    add(PZCfg::kStrBigAir, "vo_big_air");
    add(PZCfg::kStrCloseCall, "vo_close_call");
    add(PZCfg::kStrSmash, "vo_smash");
    add(PZCfg::kStrMissionToast, "vo_mission");
    add(PZCfg::kStrComebackTitle, "vo_welcome");
    std::sort(b.begin(), b.end(), [](const Binding& x, const Binding& y) { return x.prefix.size() > y.prefix.size(); });
    return b;
}

} // namespace

void play(const std::string& name, float volume, float minGapSeconds) {
    if (!PZCfg::kEnableSfx || name.empty()) return;
    const std::string path = pathFor(name);
    if (!exists(path)) return;
    const auto now = Clock::now();
    auto& last = lastPlayed()[name];
    if (std::chrono::duration<float>(now - last).count() < minGapSeconds) return;
    last = now;
    const int id = ax::AudioEngine::play2d(path, false, std::max(0.f, std::min(1.f, volume * PZCfg::kSfxVolume)));
}

void say(const std::string& name, int priority) {
    if (!PZCfg::kEnableVoice || name.empty()) return;
    const std::string path = pathFor(name);
    if (!exists(path)) return;
    if (g_voiceId >= 0 && ax::AudioEngine::getState(g_voiceId) == ax::AudioEngine::AudioState::PLAYING) {
        if (priority < g_voicePriority) return;
        ax::AudioEngine::stop(g_voiceId);
    }
    g_voiceId = ax::AudioEngine::play2d(path, false, std::max(0.f, std::min(1.f, PZCfg::kVoiceVolume)));
    g_voicePriority = priority;
}

void sayLabel(const std::string& text) {
    if (text.empty()) return;
    for (const auto& b : bindings()) {
        if (text.compare(0, b.prefix.size(), b.prefix) == 0) {
            say(b.voice, 1);
            return;
        }
    }
}

void preload() {
    static const char* names[] = {"fx_jump", "fx_land", "fx_powerup", "fx_explosion", "fx_smash", "fx_tick", "fx_open",
                                  "fx_buy", "fx_blast", "fx_crash", "vo_go", "vo_second_chance", "vo_get_ready"};
    for (const char* n : names) {
        const std::string p = pathFor(n);
        if (exists(p)) ax::AudioEngine::preload(p);
    }
}

} // namespace PZAudio
