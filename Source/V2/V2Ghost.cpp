#include "V2/V2Ghost.h"
#include "Features/PZUI.h"

#include <algorithm>
#include <cstring>

USING_NS_AX;

namespace V2 {

namespace {
float floatOf(uint32_t b) { float f; std::memcpy(&f, &b, sizeof(f)); return f; }
}

GhostRider* GhostRider::create() {
    auto* g = new GhostRider();
    if (g->init()) { g->autorelease(); return g; }
    delete g;
    return nullptr;
}

bool GhostRider::compatible(const Replay::Data& d, uint32_t seed, int rules, int modifier) {
    return d.seed == seed && d.rulesVersion == rules && d.modifier == modifier && d.checkpoints.size() >= 3;
}

void GhostRider::arm(const Replay::Data& d) {
    clear();
    for (const auto& c : d.checkpoints) _pts.push_back({c.tick, floatOf(c.bits[4]), floatOf(c.bits[1])});
}

void GhostRider::clear() {
    _pts.clear();
    if (_spr) { _spr->removeFromParent(); _spr = nullptr; }
    if (_tag) { _tag->removeFromParent(); _tag = nullptr; }
}

void GhostRider::update(uint32_t tick, float myDistance, float myBodyY, const Vec2& riderScreen, float metersPerPixel,
                        SpriteFrame* frame, float spriteScale, Node* parent, int z) {
    if (_pts.size() < 2 || !frame || !parent) return;
    if (tick < _pts.front().tick || tick > _pts.back().tick) {          // before the first checkpoint / the ghost's run is over
        if (_spr) _spr->setVisible(false);
        if (_tag) _tag->setVisible(false);
        return;
    }
    size_t i = 0;
    while (i + 1 < _pts.size() && _pts[i + 1].tick < tick) ++i;
    const Pt& a = _pts[i];
    const Pt& b = _pts[std::min(i + 1, _pts.size() - 1)];
    const float t = b.tick > a.tick ? static_cast<float>(tick - a.tick) / static_cast<float>(b.tick - a.tick) : 0.f;
    const float dist = a.dist + (b.dist - a.dist) * t, y = a.y + (b.y - a.y) * t;
    const float dx = dist - myDistance;                                   // distance units are pixels (platform speed is px / s)
    const float dy = (y - myBodyY) / std::max(0.0001f, metersPerPixel);
    if (!_spr) {
        _spr = Sprite::createWithSpriteFrame(frame);
        _spr->setColor(Color3B(150, 200, 255));
        _spr->setOpacity(110);
        parent->addChild(_spr, z);
        _tag = PZUI::label("BEST", 7.f, Color3B(170, 215, 255));
        parent->addChild(_tag, z);
    }
    _spr->setSpriteFrame(frame);
    _spr->setScale(spriteScale);
    _spr->setVisible(true);
    _tag->setVisible(true);
    const Vec2 p(riderScreen.x + dx, riderScreen.y + dy);
    _spr->setPosition(p);
    _tag->setPosition(p + Vec2(0.f, 70.f));
}

} // namespace V2
