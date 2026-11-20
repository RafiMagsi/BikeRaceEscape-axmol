#include "V2/V2Scenic.h"
#include "V2/V2Content.h"
#include "V2/V2Presentation.h"
#include "PZGGameFieldScene.h"

#include <algorithm>
#include <chrono>
#include <cmath>

USING_NS_AX;

namespace V2 {

ScenicDirector* ScenicDirector::create(PZGGameFieldScene* scene) {
    auto* d = new ScenicDirector();
    if (d->init(scene)) {
        d->autorelease();
        return d;
    }
    delete d;
    return nullptr;
}

bool ScenicDirector::init(PZGGameFieldScene* scene) {
    if (!Node::init()) return false;
    _scene = scene;
    _rng.reseed(static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count()) | 1u);   // cosmetic: not the run seed
    return true;
}

void ScenicDirector::reset() {
    clear();
    _runClock = 0.f;
    _recent.clear();
    _lastId.clear();
    const ScenicSettings& s = Content::shared().scenic();
    _cooldown = _rng.range(s.firstEligibleMin, std::max(s.firstEligibleMin, s.firstEligibleMax));
}

void ScenicDirector::clear() {
    if (_clip) _clip->removeFromParent();
    else if (_node) _node->removeFromParent();
    _clip = nullptr;
    _node = nullptr;
    _spr = nullptr;
    _beat = -1;
}

int ScenicDirector::pickBeat() {
    const auto& beats = Content::shared().scenicBeats();
    std::vector<int> ok;
    for (int i = 0; i < static_cast<int>(beats.size()); ++i) {
        if (beats[i].id == _lastId && beats.size() > 1) continue;               // never the same beat twice in a row
        if (_runClock < beats[i].minRunSeconds) continue;
        ok.push_back(i);
    }
    if (ok.empty()) return -1;
    return ok[_rng.index(static_cast<int>(ok.size()))];
}

void ScenicDirector::update(float dt, bool safe) {
    if (!_scene) return;
    const Presentation::Spectacle spec = Presentation::spectacle();
    if (spec == Presentation::Spectacle::Off) {
        if (_node) clear();
        return;
    }
    if (!safe) {                                    // decoration yields instantly: fade a live beat out, never start one
        if (_node && dt > 0.f) _t = std::max(_t, _duration - 0.35f);
        return;
    }
    _runClock += dt;
    if (!_node) {
        _cooldown -= dt;
        if (_cooldown > 0.f) return;
        // at most N beats in any five minutes
        _recent.erase(std::remove_if(_recent.begin(), _recent.end(), [this](float t) { return _runClock - t > 300.f; }), _recent.end());
        if (static_cast<int>(_recent.size()) >= Content::shared().scenic().maxPerFiveMinutes) { _cooldown = 20.f; return; }
        if (!Content::shared().ensureGroup(Content::shared().scenic().preloadGroup)) { _cooldown = 600.f; return; }   // optional art missing: stay quiet
        const int i = pickBeat();
        if (i < 0) { _cooldown = 10.f; return; }
        _reduced = spec == Presentation::Spectacle::Reduced || Presentation::reducedMotion();
        startBeat(i);
        return;
    }
    _t += dt;
    const float p = std::min(1.f, _t / std::max(0.1f, _duration));
    const auto& def = Content::shared().scenicBeats()[_beat];
    if (_reduced) {                                 // still image, fade in / hold / fade out
        const float a = p < 0.15f ? p / 0.15f : (p > 0.85f ? (1.f - p) / 0.15f : 1.f);
        _spr->setOpacity(static_cast<uint8_t>(def.opacity * 0.8f * std::max(0.f, a)));
    } else {
        const float e = p * p * (3.f - 2.f * p);    // smoothstep
        _node->setPosition(Vec2(_x0 + (_x1 - _x0) * (def.motion == "walk" ? p : e), _y0 + (_y1 - _y0) * (def.motion == "walk" ? 0.f : (p < 0.5f ? e * 2.f : (1.f - e) * 2.f))));
        const float a = p < 0.1f ? p / 0.1f : (p > 0.9f ? (1.f - p) / 0.1f : 1.f);
        _spr->setOpacity(static_cast<uint8_t>(def.opacity * std::max(0.f, a)));
        if (def.motion == "walk") {
            const AnimDef* an = Content::shared().animation(def.animationId);
            if (an && !an->frames.empty()) _spr->setSpriteFrame(an->frames[static_cast<size_t>(_t * an->fps) % an->frames.size()]);
        }
    }
    if (_t >= _duration) finishBeat();
}

void ScenicDirector::startBeat(int index) {
    const auto& def = Content::shared().scenicBeats()[index];
    const std::string& f0 = Content::shared().frame(def.animationId, 0);
    _spr = f0.empty() ? nullptr : Sprite::createWithSpriteFrameName(f0);
    if (!_spr) { _cooldown = 120.f; return; }
    const Size win = Director::getInstance()->getWinSize();
    const float target = win.height * def.heightFrac;
    _spr->setScale(target / std::max(1.f, _spr->getContentSize().height));
    _spr->setColor(Color3B(def.tint[0], def.tint[1], def.tint[2]));
    _spr->setOpacity(0);
    _spr->setAnchorPoint(Vec2(0.5f, 0.f));
    _node = Node::create();
    _node->addChild(_spr);
    if (def.clipYFrac > 0.f) {                      // the beat lives in the open sky band: parts below the skyline are not drawn
        auto* clip = ClippingRectangleNode::create(Rect(0.f, win.height * def.clipYFrac, win.width, win.height * (1.f - def.clipYFrac) + 4.f));
        _scene->addChild(clip, std::max(1, _scene->character_depth_z - 2));
        _clip = clip;
    }
    _beat = index;
    _t = 0.f;
    _duration = _reduced ? std::min(def.duration, 6.f) : def.duration;
    const float w = _spr->getContentSize().width * _spr->getScale();
    const float baseY = win.height * def.yFrac;
    if (def.motion == "walk") {                     // crosses the far plane right to left
        _x0 = win.width + w * 0.5f;
        _x1 = -w * 0.5f;
        _y0 = _y1 = baseY;
    } else {                                        // peek: rises, watches, lowers (a still fade when reduced)
        _x0 = _x1 = win.width * def.xFrac;
        _y0 = baseY - target * 0.97f;
        _y1 = baseY - target * 0.45f;
    }
    if (_reduced) { _x0 = _x1 = win.width * (def.motion == "walk" ? 0.6f : def.xFrac); _y0 = _y1 = baseY - (def.motion == "peek" ? target * 0.30f : 0.f); }
    _node->setPosition(Vec2(_x0, _y0));
    // behind the gameplay platforms (z = depth - 1) and in front of the parallax: never over the road or the rider
    if (_clip) _clip->addChild(_node);
    else _scene->addChild(_node, std::max(1, _scene->character_depth_z - 2));
    _lastId = def.id;
    _recent.push_back(_runClock);
    AXLOGI("[V2:scenic] start {} reduced={}", def.id, _reduced ? 1 : 0);
    Analytics::shared().log("scenic_beat", {{"beat", def.id}, {"reduced", _reduced ? "1" : "0"}, {"run_s", std::to_string(static_cast<int>(_runClock))}});
}

void ScenicDirector::finishBeat() {
    const auto& def = Content::shared().scenicBeats()[_beat];
    clear();
    _cooldown = _rng.range(def.cooldownMin, std::max(def.cooldownMin, def.cooldownMax));
}

} // namespace V2
