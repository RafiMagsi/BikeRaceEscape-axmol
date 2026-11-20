#include "V2/V2Content.h"
#include "PZLegacyCompat.h"

#include <set>

USING_NS_AX;

namespace V2 {

namespace {
const std::string kEmpty;

std::string str(const ValueMap& m, const char* k) {
    auto it = m.find(k);
    return it == m.end() ? std::string() : it->second.asString();
}
}

Content& Content::shared() {
    static Content c;
    return c;
}

void Content::load() {
    if (!_loaded) reload();
}

void Content::reload() {
    _loaded = true;
    _atlases.clear();
    _anims.clear();
    _dialogues.clear();
    _groupLoaded.clear();
    _beats.clear();
    _seqs.clear();
    _portraits.clear();
    _dlgSettings = DialogueSettings();
    _scenic = ScenicSettings();
    ValueMap file = FileUtils::getInstance()->getValueMapFromFile("CompleteGameInfo.plist");
    auto v2 = file.find("v2");
    if (v2 == file.end() || v2->second.getType() != Value::Type::MAP) {
        AXLOGE("[V2:content] CompleteGameInfo.plist has no v2 section");
        return;
    }
    const ValueMap& root = v2->second.asValueMap();
    auto cv = root.find("contentVersion");
    _contentVersion = cv == root.end() ? 0 : cv->second.asInt();
    auto pr = root.find("presentation");
    if (pr == root.end() || pr->second.getType() != Value::Type::MAP) {
        AXLOGE("[V2:content] v2.presentation is missing");
        return;
    }
    const ValueMap& p = pr->second.asValueMap();
    auto sv = p.find("schemaVersion");
    _schemaVersion = sv == p.end() ? 0 : sv->second.asInt();

    auto at = p.find("atlases");
    if (at != p.end() && at->second.getType() == Value::Type::VECTOR) {
        for (const Value& v : at->second.asValueVector()) {
            if (v.getType() != Value::Type::MAP) continue;
            const ValueMap& m = v.asValueMap();
            AtlasDef a;
            a.id = str(m, "id");
            a.plist = str(m, "plist");
            a.texture = str(m, "texture");
            a.preloadGroup = str(m, "preloadGroup");
            auto rq = m.find("required");
            a.required = rq == m.end() ? true : rq->second.asBool();
            _atlases.push_back(a);
        }
    }
    auto an = p.find("animations");
    if (an != p.end() && an->second.getType() == Value::Type::MAP) {
        for (const auto& kv : an->second.asValueMap()) {
            if (kv.second.getType() != Value::Type::MAP) continue;
            const ValueMap& m = kv.second.asValueMap();
            AnimDef d;
            d.id = kv.first;
            d.atlasId = str(m, "atlasId");
            d.role = str(m, "role");
            auto fps = m.find("fps");
            d.fps = fps == m.end() ? 8.f : fps->second.asFloat();
            auto lp = m.find("loop");
            d.loop = lp == m.end() ? true : lp->second.asBool();
            auto fr = m.find("frames");
            if (fr != m.end() && fr->second.getType() == Value::Type::VECTOR) {
                for (const Value& f : fr->second.asValueVector()) d.frames.push_back(f.asString());
            }
            _anims[d.id] = d;
        }
    }
    auto dl = p.find("dialogues");
    if (dl != p.end() && dl->second.getType() == Value::Type::MAP) {
        for (const auto& kv : dl->second.asValueMap()) {
            if (kv.second.getType() != Value::Type::MAP) continue;
            const ValueMap& m = kv.second.asValueMap();
            auto ln = m.find("lines");
            if (ln == m.end() || ln->second.getType() != Value::Type::VECTOR) continue;
            std::vector<std::string> lines;
            for (const Value& l : ln->second.asValueVector()) lines.push_back(l.asString());
            _dialogues[kv.first] = lines;
        }
    }
    auto sc = p.find("scenic");
    if (sc != p.end() && sc->second.getType() == Value::Type::MAP) {
        const ValueMap& m = sc->second.asValueMap();
        auto f = [&](const char* k, float d) { auto it = m.find(k); return it == m.end() ? d : it->second.asFloat(); };
        _scenic.firstEligibleMin = f("firstEligibleMin", 25.f);
        _scenic.firstEligibleMax = f("firstEligibleMax", 40.f);
        auto mx = m.find("maxPerFiveMinutes");
        if (mx != m.end()) _scenic.maxPerFiveMinutes = mx->second.asInt();
        if (!str(m, "preloadGroup").empty()) _scenic.preloadGroup = str(m, "preloadGroup");
    }
    auto sb = p.find("scenicBeats");
    if (sb != p.end() && sb->second.getType() == Value::Type::MAP) {
        for (const auto& kv : sb->second.asValueMap()) {
            if (kv.second.getType() != Value::Type::MAP) continue;
            const ValueMap& m = kv.second.asValueMap();
            auto f = [&](const char* k, float d) { auto it = m.find(k); return it == m.end() ? d : it->second.asFloat(); };
            ScenicBeatDef b;
            b.id = kv.first;
            b.animationId = str(m, "animationId");
            b.layer = str(m, "layer");
            b.motion = str(m, "motion");
            b.cooldownMin = f("cooldownMin", 60.f);
            b.cooldownMax = f("cooldownMax", 120.f);
            b.duration = f("duration", 8.f);
            b.minRunSeconds = f("minRunSeconds", 25.f);
            b.heightFrac = f("heightFrac", 0.4f);
            b.yFrac = f("yFrac", 0.35f);
            b.xFrac = f("xFrac", 0.5f);
            b.clipYFrac = f("clipYFrac", 0.f);
            b.opacity = static_cast<int>(f("opacity", 150.f));
            auto tn = m.find("tint");
            if (tn != m.end() && tn->second.getType() == Value::Type::VECTOR && tn->second.asValueVector().size() >= 3) {
                for (int i = 0; i < 3; ++i) b.tint[i] = tn->second.asValueVector()[i].asInt();
            }
            if (b.animationId.empty() || b.duration <= 0.f) continue;
            _beats.push_back(b);
        }
    }
    auto ps = p.find("portraitSets");
    if (ps != p.end() && ps->second.getType() == Value::Type::MAP) {
        for (const auto& kv : ps->second.asValueMap()) {
            if (kv.second.getType() != Value::Type::MAP) continue;
            for (const auto& e : kv.second.asValueMap()) _portraits[kv.first][e.first] = e.second.asString();
        }
    }
    auto ds = p.find("dialogueSettings");
    if (ds != p.end() && ds->second.getType() == Value::Type::MAP) {
        const ValueMap& m = ds->second.asValueMap();
        auto f = [&](const char* k, float d) { auto it = m.find(k); return it == m.end() ? d : it->second.asFloat(); };
        if (!str(m, "preloadGroup").empty()) _dlgSettings.preloadGroup = str(m, "preloadGroup");
        _dlgSettings.autoAdvance = f("autoAdvanceSeconds", 3.2f);
        _dlgSettings.pushIn = f("pushInPercent", 2.5f);
    }
    auto sq = p.find("dialogueSequences");
    if (sq != p.end() && sq->second.getType() == Value::Type::MAP) {
        for (const auto& kv : sq->second.asValueMap()) {
            if (kv.second.getType() != Value::Type::MAP) continue;
            const ValueMap& m = kv.second.asValueMap();
            DialogueSeq q;
            q.id = kv.first;
            q.trigger = str(m, "trigger");
            q.policy = str(m, "oncePolicy");
            auto vr = m.find("version");
            if (vr != m.end()) q.version = vr->second.asInt();
            auto pr2 = m.find("priority");
            if (pr2 != m.end()) q.priority = pr2->second.asInt();
            auto sp = m.find("speakers");
            if (sp != m.end() && sp->second.getType() == Value::Type::VECTOR) {
                for (const Value& v : sp->second.asValueVector()) {
                    if (v.getType() != Value::Type::MAP) continue;
                    const ValueMap& s = v.asValueMap();
                    DialogueSpeaker d;
                    d.id = str(s, "id"); d.name = str(s, "name"); d.portraitSet = str(s, "portraitSet"); d.side = str(s, "side");
                    auto cl = s.find("color");
                    if (cl != s.end() && cl->second.getType() == Value::Type::VECTOR && cl->second.asValueVector().size() >= 3) {
                        for (int i = 0; i < 3; ++i) d.color[i] = cl->second.asValueVector()[i].asInt();
                    }
                    q.speakers.push_back(d);
                }
            }
            auto ln = m.find("lines");
            if (ln != m.end() && ln->second.getType() == Value::Type::VECTOR) {
                for (const Value& v : ln->second.asValueVector()) {
                    if (v.getType() != Value::Type::MAP) continue;
                    const ValueMap& s = v.asValueMap();
                    q.lines.push_back({str(s, "speaker"), str(s, "text"), str(s, "expression")});
                }
            }
            if (!q.lines.empty()) _seqs[q.id] = q;
        }
    }
    AXLOGI("[V2:content] loaded contentVersion={} atlases={} animations={} dialogues={}", _contentVersion, _atlases.size(), _anims.size(), _dialogues.size());
}

bool Content::ensureGroup(const std::string& group) {
    load();
    if (_groupLoaded[group]) return true;
    bool ok = true;
    for (const AtlasDef& a : _atlases) {
        if (a.preloadGroup != group) continue;
        if (!FileUtils::getInstance()->isFileExist(a.plist)) {
            AXLOGE("[V2:content] atlas '{}' plist missing: {}", a.id, a.plist);
            if (a.required) ok = false;
            continue;
        }
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile(a.plist);
    }
    _groupLoaded[group] = ok;
    return ok;
}

const AnimDef* Content::animation(const std::string& id) const {
    auto it = _anims.find(id);
    return it == _anims.end() ? nullptr : &it->second;
}

const std::string& Content::frame(const std::string& animId, int index) const {
    const AnimDef* a = animation(animId);
    if (!a || a->frames.empty()) return kEmpty;
    const int n = static_cast<int>(a->frames.size());
    return a->frames[((index % n) + n) % n];
}

const std::string& Content::portrait(const std::string& set, const std::string& expression) const {
    auto s = _portraits.find(set);
    if (s == _portraits.end()) return kEmpty;
    auto e = s->second.find(expression);
    return e == s->second.end() ? kEmpty : e->second;
}

const std::vector<std::string>* Content::dialogue(const std::string& id) const {
    auto it = _dialogues.find(id);
    return it == _dialogues.end() ? nullptr : &it->second;
}

std::string Content::line(const std::string& id) const {
    const auto* d = dialogue(id);
    return d && !d->empty() ? d->front() : std::string();
}

// Debug / tooling report. Reads atlas plists as data (no texture upload): every atlas must exist, name the registered texture,
// own each frame exclusively; every animation must point at a known atlas and existing frames with a safe fps; every dialogue
// needs at least one line of at most 40 characters.
std::string Content::validate(int* errorsOut) {
    load();
    std::string r;
    int errors = 0;
    auto err = [&](const std::string& s) { r += "ERROR " + s + "\n"; ++errors; };
    if (_contentVersion < 1) err("v2.contentVersion must be >= 1");
    if (_schemaVersion < 1) err("v2.presentation.schemaVersion must be >= 1");
    std::map<std::string, std::set<std::string>> framesOf;
    std::set<std::string> owned;
    std::set<std::string> ids;
    for (const AtlasDef& a : _atlases) {
        if (a.id.empty()) { err("an atlas has no id"); continue; }
        if (!ids.insert(a.id).second) err("duplicate atlas id '" + a.id + "'");
        if (!FileUtils::getInstance()->isFileExist(a.plist)) { err("atlas '" + a.id + "': plist not found: " + a.plist); continue; }
        if (!FileUtils::getInstance()->isFileExist(a.texture)) err("atlas '" + a.id + "': texture not found: " + a.texture);
        ValueMap pl = FileUtils::getInstance()->getValueMapFromFile(a.plist);
        auto md = pl.find("metadata");
        if (md != pl.end() && md->second.getType() == Value::Type::MAP) {
            const std::string tex = str(md->second.asValueMap(), "textureFileName");
            if (!tex.empty() && tex != a.texture) err("atlas '" + a.id + "': plist names texture '" + tex + "' but the registry says '" + a.texture + "'");
        }
        auto fr = pl.find("frames");
        if (fr == pl.end() || fr->second.getType() != Value::Type::MAP) { err("atlas '" + a.id + "': plist has no frames"); continue; }
        for (const auto& kv : fr->second.asValueMap()) {
            framesOf[a.id].insert(kv.first);
            if (a.required && !owned.insert(kv.first).second) err("frame '" + kv.first + "' is defined by two required atlases");
        }
    }
    for (const auto& kv : _anims) {
        const AnimDef& d = kv.second;
        if (!ids.count(d.atlasId)) { err("animation '" + d.id + "': unknown atlasId '" + d.atlasId + "'"); continue; }
        if (d.frames.empty()) err("animation '" + d.id + "' has no frames");
        if (d.fps < 1.f || d.fps > 60.f) err("animation '" + d.id + "': unsafe fps " + std::to_string(d.fps));
        for (const auto& f : d.frames) {
            if (!framesOf[d.atlasId].count(f)) err("animation '" + d.id + "': frame '" + f + "' is not in atlas '" + d.atlasId + "'");
        }
    }
    for (const ScenicBeatDef& b : _beats) {
        if (!animation(b.animationId)) err("scenic beat '" + b.id + "': unknown animation '" + b.animationId + "'");
        if (b.layer.rfind("Background", 0) != 0) err("scenic beat '" + b.id + "': layer '" + b.layer + "' is not a background layer");
    }
    for (const auto& kv : _seqs) {
        std::set<std::string> speakers;
        for (const auto& s : kv.second.speakers) {
            speakers.insert(s.id);
            if (portrait(s.portraitSet, "neutral").empty()) err("sequence '" + kv.first + "': speaker '" + s.id + "' has no portrait set '" + s.portraitSet + "'");
        }
        for (const auto& l : kv.second.lines) {
            if (!speakers.count(l.speaker)) err("sequence '" + kv.first + "': unknown speaker '" + l.speaker + "'");
            if (l.text.empty() || l.text.size() > 40) err("sequence '" + kv.first + "': line empty or over 40 characters: " + l.text);
        }
    }
    for (const auto& kv : _dialogues) {
        if (kv.second.empty()) err("dialogue '" + kv.first + "' has no lines");
        for (const auto& l : kv.second) {
            if (l.empty()) err("dialogue '" + kv.first + "' has an empty line");
            if (l.size() > 40) err("dialogue '" + kv.first + "': line longer than 40 characters: " + l);
        }
    }
    if (errorsOut) *errorsOut = errors;
    return r;
}

} // namespace V2
