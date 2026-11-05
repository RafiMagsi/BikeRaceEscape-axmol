//
//  PZUI.cpp
//
#include "PZUI.h"
#include "PZHaptics.h"
#include "SharedData/PZGSharedData.h"
#include "SharedData/PZGArtCoins.h"
#include <cmath>
#include <unordered_map>
#include <vector>

USING_NS_AX;

namespace PZUI {

namespace {
constexpr int kShakeTag = 7731;

void roundedPoints(std::vector<Vec2>& pts, float w, float h, float r) {
    r = std::min(r, std::min(w, h) * 0.5f);
    const float hw = w * 0.5f, hh = h * 0.5f;
    const int seg = 6;
    // Corner centres, counter-clockwise starting bottom-right.
    const Vec2 centres[4] = {Vec2(hw - r, -hh + r), Vec2(hw - r, hh - r), Vec2(-hw + r, hh - r), Vec2(-hw + r, -hh + r)};
    const float starts[4] = {-90.f, 0.f, 90.f, 180.f};
    for (int c = 0; c < 4; ++c) {
        for (int i = 0; i <= seg; ++i) {
            const float a = CC_DEGREES_TO_RADIANS(starts[c] + 90.f * i / seg);
            pts.push_back(centres[c] + Vec2(std::cos(a), std::sin(a)) * r);
        }
    }
}
} // namespace

// Optional-art lookups are cached: the engine logs a warning for every missing file, and orbs are created constantly.
bool fileExists(const std::string& name) {
    static std::unordered_map<std::string, bool> cache;
    auto it = cache.find(name);
    if (it != cache.end()) return it->second;
    const bool ok = FileUtils::getInstance()->isFileExist(name);
    cache.emplace(name, ok);
    return ok;
}

Label* label(const std::string& text, float size, const Color3B& color) {
    Label* l = Label::createWithTTF(text, PZCfg::kFontPath, std::max(size * PZCfg::kFontScale, PZCfg::kMinFontPx));
    if (!l || l->getContentSize().width <= 0) {
        l = Label::createWithSystemFont(text, "Courier", size * 1.6f);
    }
    if (l) l->setColor(color);
    return l;
}

Label* heading(const std::string& text, float size, const Color3B& color) {
    Label* l = label(text, size, color);
    if (l) l->enableOutline(Color4B(40, 16, 0, 255), std::max(2, static_cast<int>(size / 7.f)));
    return l;
}

Node* gadgetIcon(const std::string& id, float size, const std::string& fallbackLetter, bool dim) {
    auto* root = Node::create();
    auto* d = DrawNode::create();
    root->addChild(d);
    const float u = size * 0.5f;   // radius
    const float a = dim ? 0.45f : 1.f;
    auto C = [a](int r, int g, int b) { return Color4F(r / 255.f, g / 255.f, b / 255.f, a); };
    auto P = [u](float x, float y) { return Vec2(x * u, y * u); };
    auto poly = [&](std::vector<Vec2> pts, const Color4F& fill, const Color4F& line, float w = 1.5f) {
        d->drawSolidPoly(pts.data(), static_cast<unsigned int>(pts.size()), fill, w, line, true);
    };
    const Color4F outline = C(30, 22, 18);

    if (id == "magnet") {
        // Horseshoe: red arch with silver pole tips.
        const int segs = 14;
        for (int i = 0; i < segs; ++i) {
            const float a0 = CC_DEGREES_TO_RADIANS(180.f - 180.f * i / segs);
            const float a1 = CC_DEGREES_TO_RADIANS(180.f - 180.f * (i + 1) / segs);
            d->drawSegment(P(0.55f * std::cos(a0), 0.15f + 0.55f * std::sin(a0)), P(0.55f * std::cos(a1), 0.15f + 0.55f * std::sin(a1)),
                           u * 0.30f, C(214, 48, 52));
        }
        poly({P(-0.85f, 0.15f), P(-0.25f, 0.15f), P(-0.25f, -0.55f), P(-0.85f, -0.55f)}, C(214, 48, 52), outline);
        poly({P(0.25f, 0.15f), P(0.85f, 0.15f), P(0.85f, -0.55f), P(0.25f, -0.55f)}, C(214, 48, 52), outline);
        poly({P(-0.85f, -0.30f), P(-0.25f, -0.30f), P(-0.25f, -0.55f), P(-0.85f, -0.55f)}, C(226, 230, 238), outline);
        poly({P(0.25f, -0.30f), P(0.85f, -0.30f), P(0.85f, -0.55f), P(0.25f, -0.55f)}, C(226, 230, 238), outline);
    } else if (id == "turbo") {
        poly({P(0.15f, 0.95f), P(-0.55f, 0.00f), P(-0.05f, 0.00f), P(-0.25f, -0.95f), P(0.60f, 0.12f), P(0.08f, 0.12f)},
             C(255, 214, 40), outline);
        poly({P(0.10f, 0.62f), P(-0.25f, 0.10f), P(0.12f, 0.10f)}, C(255, 244, 170), C(255, 244, 170), 0.f);
    } else if (id == "shield") {
        std::vector<Vec2> body = {P(0.0f, 0.95f), P(0.78f, 0.65f), P(0.72f, -0.10f), P(0.0f, -0.95f), P(-0.72f, -0.10f), P(-0.78f, 0.65f)};
        poly(body, C(54, 138, 238), outline, 2.f);
        poly({P(0.0f, 0.78f), P(0.60f, 0.54f), P(0.56f, -0.05f), P(0.0f, -0.74f)}, C(110, 186, 255), C(110, 186, 255), 0.f);
        poly({P(0.0f, 0.78f), P(-0.60f, 0.54f), P(-0.56f, -0.05f), P(0.0f, -0.74f)}, C(38, 102, 200), C(38, 102, 200), 0.f);
        d->drawSolidCircle(P(0.f, 0.12f), u * 0.20f, 0.f, 16, C(255, 255, 255));
    } else if (id == "jetpack") {
        poly({P(-0.62f, 0.75f), P(-0.10f, 0.75f), P(-0.10f, -0.35f), P(-0.62f, -0.35f)}, C(170, 178, 192), outline);
        poly({P(0.10f, 0.75f), P(0.62f, 0.75f), P(0.62f, -0.35f), P(0.10f, -0.35f)}, C(170, 178, 192), outline);
        poly({P(-0.62f, 0.45f), P(-0.10f, 0.45f), P(-0.10f, 0.25f), P(-0.62f, 0.25f)}, C(222, 64, 52), C(222, 64, 52), 0.f);
        poly({P(0.10f, 0.45f), P(0.62f, 0.45f), P(0.62f, 0.25f), P(0.10f, 0.25f)}, C(222, 64, 52), C(222, 64, 52), 0.f);
        poly({P(-0.56f, -0.35f), P(-0.16f, -0.35f), P(-0.36f, -0.95f)}, C(255, 148, 30), outline);
        poly({P(0.16f, -0.35f), P(0.56f, -0.35f), P(0.36f, -0.95f)}, C(255, 148, 30), outline);
        poly({P(-0.45f, -0.35f), P(-0.27f, -0.35f), P(-0.36f, -0.70f)}, C(255, 236, 120), C(255, 236, 120), 0.f);
        poly({P(0.27f, -0.35f), P(0.45f, -0.35f), P(0.36f, -0.70f)}, C(255, 236, 120), C(255, 236, 120), 0.f);
    } else if (id == "headstart") {
        // Two forward chevrons: "go ahead".
        for (int k = 0; k < 2; ++k) {
            const float ox = -0.62f + k * 0.62f;
            poly({P(ox, 0.80f), P(ox + 0.55f, 0.0f), P(ox, -0.80f), P(ox + 0.30f, -0.80f), P(ox + 0.85f, 0.0f), P(ox + 0.30f, 0.80f)},
                 k == 0 ? C(96, 214, 96) : C(150, 238, 120), outline);
        }
    } else if (id == "rocket") {
        poly({P(-0.20f, -0.45f), P(-0.20f, 0.50f), P(0.0f, 0.95f), P(0.20f, 0.50f), P(0.20f, -0.45f)}, C(236, 238, 244), outline, 2.f);
        poly({P(-0.20f, 0.50f), P(0.0f, 0.95f), P(0.20f, 0.50f)}, C(224, 56, 48), outline, 2.f);
        poly({P(-0.20f, -0.10f), P(-0.62f, -0.62f), P(-0.20f, -0.45f)}, C(224, 56, 48), outline);
        poly({P(0.20f, -0.10f), P(0.62f, -0.62f), P(0.20f, -0.45f)}, C(224, 56, 48), outline);
        d->drawSolidCircle(P(0.f, 0.22f), u * 0.14f, 0.f, 14, C(70, 160, 240));
        poly({P(-0.14f, -0.45f), P(0.14f, -0.45f), P(0.0f, -0.98f)}, C(255, 160, 30), C(255, 160, 30), 0.f);
    } else if (id == "helmet") {
        const int segs = 14;
        std::vector<Vec2> dome;
        for (int i = 0; i <= segs; ++i) {
            const float t = CC_DEGREES_TO_RADIANS(180.f * i / segs);
            dome.push_back(P(0.85f * std::cos(t), -0.05f + 0.85f * std::sin(t)));
        }
        poly(dome, C(250, 190, 40), outline, 2.f);
        poly({P(-0.85f, -0.05f), P(0.85f, -0.05f), P(0.85f, -0.30f), P(-0.85f, -0.30f)}, C(214, 146, 26), outline);
        poly({P(-0.10f, 0.08f), P(0.80f, 0.08f), P(0.70f, 0.50f), P(0.10f, 0.52f)}, C(70, 160, 240), outline);
        d->drawSolidCircle(P(-0.32f, 0.5f), u * 0.10f, 0.f, 10, C(255, 244, 190));
    } else if (id == "gun") {
        poly({P(-0.80f, -0.15f), P(-0.80f, -0.65f), P(-0.15f, -0.65f), P(-0.15f, -0.15f)}, C(86, 92, 106), outline);   // body
        poly({P(-0.20f, 0.05f), P(0.62f, 0.05f), P(0.62f, -0.40f), P(-0.20f, -0.40f)}, C(150, 158, 172), outline);     // barrel
        poly({P(0.62f, 0.14f), P(0.80f, 0.14f), P(0.80f, -0.49f), P(0.62f, -0.49f)}, C(96, 102, 116), outline);        // muzzle
        poly({P(0.82f, -0.12f), P(1.0f, 0.12f), P(0.96f, -0.18f), P(1.0f, -0.46f), P(0.82f, -0.26f)}, C(255, 200, 40), C(255, 200, 40), 0.f);
        d->drawSolidCircle(P(-0.48f, -0.40f), u * 0.18f, 0.f, 12, C(40, 44, 54));
    } else {
        d->drawSolidCircle(Vec2::ZERO, u * 0.9f, 0.f, 24, C(70, 76, 96));
        auto* l = heading(fallbackLetter, size * 0.55f, dim ? PZCfg::kColDim : PZCfg::kColAccent);
        root->addChild(l);
    }
    root->setContentSize(Size(size, size));
    return root;
}

Node* coinIcon(float size) {
    auto* gsd = PZGSharedData::sharedInstanse();
    if (gsd && gsd->artResource) {
        if (auto* coins = static_cast<__Array*>(gsd->artResource->objectForKey("Coins")); coins && coins->count() > 0) {
            if (auto* art = dynamic_cast<PZGArtCoins*>(coins->objectAtIndex(0))) {
                if (Sprite* spr = art->getResource()) {
                    const float base = std::max(spr->getContentSize().width, spr->getContentSize().height);
                    if (base > 1.f) {
                        spr->setScale(size / base);
                        spr->setAnchorPoint(Vec2(0.5f, 0.5f));
                        auto* holder = Node::create();
                        holder->addChild(spr);
                        return holder;
                    }
                }
            }
        }
    }
    auto* d = DrawNode::create();
    d->drawSolidCircle(Vec2::ZERO, size * 0.5f, 0.f, 20, c4f(PZCfg::kColAccent));
    d->drawCircle(Vec2::ZERO, size * 0.5f, 0.f, 20, false, c4f(Color3B(140, 90, 10)), 2.f);
    d->drawCircle(Vec2::ZERO, size * 0.28f, 0.f, 16, false, c4f(Color3B(190, 130, 20)), 1.5f);
    return d;
}

Node* panel(float w, float h, float radius) {
    auto* root = Node::create();
    auto* shadow = roundedRect(w + 6.f, h + 6.f, radius + 3.f, Color3B(0, 0, 0), 0.40f, Color3B(0, 0, 0), 0.f, 0.f);
    shadow->setPosition(Vec2(0, -7));
    root->addChild(shadow);
    root->addChild(roundedRect(w, h, radius, PZCfg::kColPanel, 0.97f, PZCfg::kColPanelEdge, 1.f, 5.f));
    root->addChild(roundedRect(w - 16.f, h - 16.f, std::max(4.f, radius - 7.f), PZCfg::kColPanel, 0.f, PZCfg::kColPanelEdge2, 0.55f, 1.5f));
    return root;
}

DrawNode* roundedRect(float w, float h, float radius, const Color3B& fill, float fillAlpha, const Color3B& border,
                      float borderAlpha, float borderWidth) {
    auto* d = DrawNode::create();
    std::vector<Vec2> pts;
    roundedPoints(pts, w, h, radius);
    d->drawSolidPoly(pts.data(), static_cast<unsigned>(pts.size()), c4f(fill, fillAlpha), borderWidth,
                     c4f(border, borderAlpha), true);
    return d;
}

LayerColor* modalBase(float dim) {
    auto* base = LayerColor::create(Color4B(0, 0, 0, static_cast<uint8_t>(dim * 255)));
    auto* l = EventListenerTouchOneByOne::create();
    l->setSwallowTouches(true);
    l->onTouchBegan = [](Touch*, Event*) { return true; };
    base->getEventDispatcher()->addEventListenerWithSceneGraphPriority(l, base);
    return base;
}

// ---- Button ----------------------------------------------------------------------------------

Button* Button::create(const std::string& text, float w, float h, const Color3B& color, std::function<void()> onTap,
                       float fontSize) {
    auto* b = new Button();
    if (b->initButton(text, w, h, color, std::move(onTap), fontSize)) {
        b->autorelease();
        return b;
    }
    delete b;
    return nullptr;
}

bool Button::initButton(const std::string& text, float w, float h, const Color3B& color,
                        std::function<void()> onTap, float fontSize) {
    if (!Node::init()) return false;
    _text = text; _w = w; _h = h; _color = color; _onTap = std::move(onTap); _fontSize = fontSize;
    setContentSize(Size(w, h));
    setAnchorPoint(Vec2(0.5f, 0.5f));
    rebuild();

    auto* l = EventListenerTouchOneByOne::create();
    l->setSwallowTouches(true);
    l->onTouchBegan = [this](Touch* t, Event*) {
        if (!_active || !isVisible()) return false;
        for (Node* n = this; n; n = n->getParent()) {
            if (!n->isVisible()) return false;
        }
        const Vec2 p = convertToNodeSpace(t->getLocation());
        if (std::abs(p.x - _w * 0.5f) > _w * 0.5f || std::abs(p.y - _h * 0.5f) > _h * 0.5f) return false;
        _pressed = true;
        stopActionByTag(41);
        auto* a = ScaleTo::create(0.06f, 0.93f);
        a->setTag(41);
        runAction(a);
        return true;
    };
    l->onTouchEnded = [this](Touch* t, Event*) {
        if (!_pressed) return;
        _pressed = false;
        stopActionByTag(41);
        auto* a = EaseBackOut::create(ScaleTo::create(0.14f, 1.f));
        a->setTag(41);
        runAction(a);
        const Vec2 p = convertToNodeSpace(t->getLocation());
        const bool inside = std::abs(p.x - _w * 0.5f) <= _w * 0.5f && std::abs(p.y - _h * 0.5f) <= _h * 0.5f;
        if (inside && _onTap) {
            PZHaptics::play(PZHaptics::Kind::Selection);
            auto cb = _onTap;   // the callback may remove this button
            cb();
        }
    };
    l->onTouchCancelled = [this](Touch*, Event*) {
        _pressed = false;
        stopActionByTag(41);
        setScale(1.f);
    };
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(l, this);
    return true;
}

void Button::rebuild() {
    if (_bg) _bg->removeFromParent();
    if (_label) _label->removeFromParent();
    // Orange-pill look from the original UI: drop shadow, white outline, glossy two-tone body.
    const Color3B base = _active ? _color : Color3B(96, 98, 108);
    const Color3B dark(static_cast<uint8_t>(base.r * 0.58f), static_cast<uint8_t>(base.g * 0.58f), static_cast<uint8_t>(base.b * 0.58f));
    const Color3B light(static_cast<uint8_t>(base.r + (255 - base.r) * 0.32f), static_cast<uint8_t>(base.g + (255 - base.g) * 0.32f),
                        static_cast<uint8_t>(base.b + (255 - base.b) * 0.32f));
    const float r = _h * 0.5f;
    _bg = DrawNode::create();
    auto add = [this](DrawNode* d, const Vec2& off) { d->setPosition(off); _bg->addChild(d); };
    add(roundedRect(_w + 8.f, _h + 8.f, r + 4.f, Color3B(0, 0, 0), 0.38f, Color3B(0, 0, 0), 0.f, 0.f), Vec2(0, -5.f));
    add(roundedRect(_w + 6.f, _h + 6.f, r + 3.f, Color3B(255, 255, 255), _active ? 1.f : 0.7f, Color3B(255, 255, 255), 0.f, 0.f), Vec2(0, 0));
    add(roundedRect(_w, _h, r, dark, 1.f, dark, 0.f, 0.f), Vec2(0, 0));
    add(roundedRect(_w - 6.f, _h * 0.66f, r * 0.8f, base, 1.f, base, 0.f, 0.f), Vec2(0, _h * 0.08f));
    add(roundedRect(_w - 22.f, _h * 0.26f, r * 0.4f, light, 0.55f, light, 0.f, 0.f), Vec2(0, _h * 0.22f));
    _bg->setPosition(Vec2(_w * 0.5f, _h * 0.5f));
    addChild(_bg, 0);
    _label = heading(_text, _fontSize, _active ? PZCfg::kColText : PZCfg::kColDim);
    _label->setPosition(Vec2(_w * 0.5f, _h * 0.5f + 1.f));
    // Shrink long captions to fit.
    const float maxW = _w - 28.f;
    if (_label->getContentSize().width > maxW) _label->setScale(maxW / _label->getContentSize().width);
    addChild(_label, 1);
}

void Button::setText(const std::string& text) { _text = text; rebuild(); }

void Button::setActive(bool active) {
    if (_active == active) return;
    _active = active;
    rebuild();
}

void Button::setSwallow(bool) {}

void Button::setBadge(int count) {
    if (_badge) { _badge->removeFromParent(); _badge = nullptr; }
    if (count <= 0) return;
    _badge = Node::create();
    auto* dot = DrawNode::create();
    dot->drawSolidCircle(Vec2::ZERO, 13.f, 0.f, 18, c4f(PZCfg::kColBad));
    dot->drawCircle(Vec2::ZERO, 13.f, 0.f, 18, false, c4f(PZCfg::kColText), 1.5f);
    _badge->addChild(dot);
    auto* n = label(std::to_string(count), 11.f);
    _badge->addChild(n);
    _badge->setPosition(Vec2(_w - 15.f, _h - 13.f));
    addChild(_badge, 5);
    _badge->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.5f, 1.15f), ScaleTo::create(0.5f, 1.f), nullptr)));
}

// ---- effects ---------------------------------------------------------------------------------

void burst(Node* parent, const Vec2& pos, const Color3B& color, int count, float radius, float duration, int z) {
    if (!parent) return;
    for (int i = 0; i < count; ++i) {
        const float ang = (static_cast<float>(i) / count) * 6.2831853f + CCRANDOM_0_1() * 0.5f;
        const float dist = radius * (0.6f + 0.6f * CCRANDOM_0_1());
        auto* dot = DrawNode::create();
        dot->drawDot(Vec2::ZERO, 2.5f + 2.f * CCRANDOM_0_1(), c4f(color));
        dot->setPosition(pos);
        parent->addChild(dot, z);
        dot->runAction(Sequence::create(
            Spawn::create(EaseOut::create(MoveBy::create(duration, Vec2(std::cos(ang), std::sin(ang)) * dist), 2.f),
                          FadeOut::create(duration), ScaleTo::create(duration, 0.2f), nullptr),
            RemoveSelf::create(), nullptr));
    }
}

void glitter(Node* parent, const Vec2& pos, const Color3B& color, int count, float radius, float duration, int z) {
    if (!parent || !PZCfg::kEnableFx) return;
    for (int i = 0; i < count; ++i) {
        auto* star = DrawNode::create();
        const float r = 5.f + 8.f * CCRANDOM_0_1();
        Vec2 pts[8];
        for (int k = 0; k < 8; ++k) {
            const float a = CC_DEGREES_TO_RADIANS(45.f * k);
            const float rr = (k % 2 == 0) ? r : r * 0.28f;
            pts[k] = Vec2(std::cos(a), std::sin(a)) * rr;
        }
        // A warm core with a white heart reads as a glint.
        star->drawSolidPoly(pts, 8, c4f(color, 0.95f), 0.f, Color4F(0, 0, 0, 0), false);
        star->drawDot(Vec2::ZERO, r * 0.22f, Color4F(1.f, 1.f, 1.f, 1.f));
        const float ang = CCRANDOM_0_1() * 6.2831853f;
        const float dist = radius * (0.25f + 0.75f * CCRANDOM_0_1());
        star->setPosition(pos + Vec2(std::cos(ang), std::sin(ang)) * dist);
        star->setScale(0.f);
        star->setRotation(CCRANDOM_0_1() * 90.f);
        parent->addChild(star, z);
        const float life = duration * (0.7f + 0.5f * CCRANDOM_0_1());
        star->runAction(Sequence::create(
            DelayTime::create(CCRANDOM_0_1() * duration * 0.5f),
            Spawn::create(
                Sequence::create(EaseOut::create(ScaleTo::create(life * 0.35f, 1.1f), 2.f), ScaleTo::create(life * 0.65f, 0.f), nullptr),
                RotateBy::create(life, 120.f),
                MoveBy::create(life, Vec2(0.f, 14.f)),
                nullptr),
            RemoveSelf::create(), nullptr));
    }
}

void floatText(Node* parent, const std::string& text, const Vec2& pos, const Color3B& color, float size, int z) {
    if (!parent) return;
    auto* l = heading(text, size, color);
    l->setPosition(pos);
    l->setScale(0.4f);
    parent->addChild(l, z);
    l->runAction(Sequence::create(
        EaseBackOut::create(ScaleTo::create(0.18f, 1.f)),
        Spawn::create(MoveBy::create(0.7f, Vec2(0, 55)), Sequence::create(DelayTime::create(0.35f), FadeOut::create(0.35f), nullptr), nullptr),
        RemoveSelf::create(), nullptr));
}

void banner(Node* parent, const std::string& title, const std::string& sub, const Color3B& color, float y, float hold, int z) {
    if (!parent) return;
    const Size win = Director::getInstance()->getWinSize();
    auto* box = Node::create();
    box->setCascadeOpacityEnabled(true);
    box->setPosition(Vec2(win.width * 0.5f, y));
    auto* t = heading(title, 30.f, color);
    t->setPositionY(sub.empty() ? 0.f : 12.f);
    box->addChild(t, 1);
    float w = t->getContentSize().width;
    float h = 44.f;
    if (!sub.empty()) {
        auto* s = heading(sub, 13.f, PZCfg::kColText);
        s->setPositionY(-22.f);
        box->addChild(s, 1);
        w = std::max(w, s->getContentSize().width);
        h = 74.f;
    }
    // Dark plate behind the text: banners sit over busy gameplay art and the menu logo, where bare text is unreadable.
    auto* plate = roundedRect(w + 56.f, h, h * 0.5f, PZCfg::kColPanel, 0.88f, color, 0.85f, 3.f);
    plate->setPosition(Vec2(0.f, sub.empty() ? 0.f : -5.f));
    box->addChild(plate, 0);
    box->setScale(0.2f);
    box->setOpacity(0);
    parent->addChild(box, z);
    box->runAction(Sequence::create(
        Spawn::create(EaseBackOut::create(ScaleTo::create(0.32f, 1.f)), FadeIn::create(0.2f), nullptr),
        DelayTime::create(hold),
        Spawn::create(MoveBy::create(0.3f, Vec2(0, 30)), FadeOut::create(0.3f), nullptr),
        RemoveSelf::create(), nullptr));
}

void shake(Node* node, const Vec2& home, float amplitude, float duration) {
    if (!node || !PZCfg::kEnableShake) return;
    node->stopActionByTag(kShakeTag);
    const float step = 0.03f;
    const int n = std::max(2, static_cast<int>(duration / step));
    Vector<FiniteTimeAction*> steps;
    for (int i = 0; i < n; ++i) {
        const float k = 1.f - static_cast<float>(i) / n;
        steps.pushBack(MoveTo::create(step, home + Vec2(CCRANDOM_MINUS1_1(), CCRANDOM_MINUS1_1()) * amplitude * k));
    }
    steps.pushBack(MoveTo::create(step, home));
    auto* seq = Sequence::create(steps);
    seq->setTag(kShakeTag);
    node->runAction(seq);
}

void redrawBar(DrawNode* node, float w, float h, float value, const Color3B& fill) {
    node->clear();
    value = std::max(0.f, std::min(1.f, value));
    const Vec2 lo(-w * 0.5f, -h * 0.5f), hi(w * 0.5f, h * 0.5f);
    node->drawSolidRect(lo + Vec2(-3, -3), hi + Vec2(3, 3), c4f(PZCfg::kColPanelEdge));         // bronze frame
    node->drawSolidRect(lo, hi, c4f(Color3B(10, 12, 22), 0.95f));                                // trough
    if (value > 0.001f) {
        const Vec2 end(lo.x + w * value, hi.y);
        node->drawSolidRect(lo, end, c4f(fill));
        const Color3B light(static_cast<uint8_t>(fill.r + (255 - fill.r) * 0.45f), static_cast<uint8_t>(fill.g + (255 - fill.g) * 0.45f),
                            static_cast<uint8_t>(fill.b + (255 - fill.b) * 0.45f));
        node->drawSolidRect(Vec2(lo.x, hi.y - h * 0.38f), end, c4f(light, 0.55f));               // gloss strip
    }
    node->drawRect(lo + Vec2(-3, -3), hi + Vec2(3, 3), c4f(PZCfg::kColPanelEdge2, 0.8f));
}

DrawNode* bar(float w, float h, float value, const Color3B& fill) {
    auto* d = DrawNode::create();
    redrawBar(d, w, h, value, fill);
    return d;
}

} // namespace PZUI
