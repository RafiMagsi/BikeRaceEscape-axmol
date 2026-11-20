//
//  V2Content.h
//  Typed registry for everything V2 presents: atlases, animations and dialogue lines. The single source of truth is the
//  `v2.presentation` dictionary of Content/CompleteGameInfo.plist (PRODUCT_PLAN section 23). The atlas .plist next to a texture
//  is storage metadata only (frame rectangles); names, frame sequences, timing and authored text live in the registry.
//
//  Feature code asks for semantic ids ("rider_cool", "rival_intro") and never for atlas files or raw frame names.
//
#pragma once

#include <map>
#include <string>
#include <vector>

namespace V2 {

struct AtlasDef {
    std::string id, plist, texture, preloadGroup;
    bool required = true;
};

struct AnimDef {
    std::string id, atlasId, role;
    std::vector<std::string> frames;
    float fps = 8.f;
    bool loop = true;
};

struct ScenicBeatDef {
    std::string id, animationId, layer, motion;      // motion: "walk" (crosses the far plane) or "peek" (rises behind a wall)
    float cooldownMin = 60.f, cooldownMax = 120.f, duration = 8.f, minRunSeconds = 25.f;
    float heightFrac = 0.4f, yFrac = 0.35f, xFrac = 0.5f;   // fractions of the screen
    float clipYFrac = 0.f;                          // > 0: only the part of the beat above this screen height is drawn (the open sky band)
    int opacity = 150;
    int tint[3] = {135, 155, 185};
};

struct DialogueSpeaker { std::string id, name, portraitSet, side; int color[3] = {255, 255, 255}; };
struct DialogueLine { std::string speaker, text, expression; };
struct DialogueSeq {
    std::string id, trigger, policy;                 // policy: once | session | daily | always
    int version = 1, priority = 0;
    std::vector<DialogueSpeaker> speakers;
    std::vector<DialogueLine> lines;
};
struct DialogueSettings { std::string preloadGroup = "dialogue_v2"; float autoAdvance = 3.2f, pushIn = 2.5f; };

struct ScenicSettings {
    float firstEligibleMin = 25.f, firstEligibleMax = 40.f;
    int maxPerFiveMinutes = 2;
    std::string preloadGroup = "scenic_v2";
};

class Content {
public:
    static Content& shared();

    void load();                                             // idempotent; reads CompleteGameInfo.plist once (reload() forces)
    void reload();
    bool ensureGroup(const std::string& group);              // loads every atlas of a preload group into the SpriteFrameCache
    const AnimDef* animation(const std::string& id) const;   // nullptr when unregistered
    const std::string& frame(const std::string& animId, int index) const;   // frame name (wraps); "" when unregistered
    const std::vector<std::string>* dialogue(const std::string& id) const;  // all lines of a dialogue id
    std::string line(const std::string& id) const;                          // first line, "" when unregistered
    const std::vector<ScenicBeatDef>& scenicBeats() const { return _beats; }
    const ScenicSettings& scenic() const { return _scenic; }
    const std::map<std::string, DialogueSeq>& sequences() const { return _seqs; }
    const DialogueSettings& dialogueSettings() const { return _dlgSettings; }
    const std::string& portrait(const std::string& set, const std::string& expression) const;   // animation id, "" when unknown
    int contentVersion() const { return _contentVersion; }
    int schemaVersion() const { return _schemaVersion; }
    const std::map<std::string, std::vector<std::string>>& dialogues() const { return _dialogues; }

    // One complete, actionable report of every problem (empty string = clean). errors = number of problems found.
    std::string validate(int* errors = nullptr);

private:
    bool _loaded = false;
    int _contentVersion = 0, _schemaVersion = 0;
    std::vector<AtlasDef> _atlases;
    std::map<std::string, AnimDef> _anims;
    std::map<std::string, std::vector<std::string>> _dialogues;
    std::map<std::string, bool> _groupLoaded;
    std::vector<ScenicBeatDef> _beats;
    ScenicSettings _scenic;
    std::map<std::string, DialogueSeq> _seqs;
    DialogueSettings _dlgSettings;
    std::map<std::string, std::map<std::string, std::string>> _portraits;
};

} // namespace V2
