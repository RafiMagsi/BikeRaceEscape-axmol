//
//  PZUI.cpp
//
#include "PZUI.h"
#include <cmath>
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

bool fileExists(const std::string& name) {
    return FileUtils::getInstance()->isFileExist(name);
}

Label* label(const std::string& text, float size, const Color3B& color) {
    Label* l = Label::createWithTTF(text, PZCfg::kFontPath, size);
    if (!l || l->getContentSize().width <= 0) {
        l = Label::createWithSystemFont(text, "Courier", size * 1.6f);
    }
    if (l) l->setColor(color);
    return l;
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
    const Color3B fill = _active ? Color3B(_color.r / 4, _color.g / 4, _color.b / 4) : Color3B(40, 44, 52);
    const Color3B edge = _active ? _color : Color3B(90, 96, 108);
    _bg = roundedRect(_w, _h, std::min(_h * 0.32f, 14.f), fill, 0.92f, edge, 1.f, 2.5f);
    _bg->setPosition(Vec2(_w * 0.5f, _h * 0.5f));
    addChild(_bg, 0);
    _label = label(_text, _fontSize, _active ? PZCfg::kColText : PZCfg::kColDim);
    _label->setPosition(Vec2(_w * 0.5f, _h * 0.5f));
    // Shrink long captions to fit.
    const float maxW = _w - 16.f;
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
    _badge->setPosition(Vec2(_w - 6.f, _h - 4.f));
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

void floatText(Node* parent, const std::string& text, const Vec2& pos, const Color3B& color, float size, int z) {
    if (!parent) return;
    auto* l = label(text, size, color);
    l->enableOutline(Color4B(0, 0, 0, 220), 2);
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
    auto* t = label(title, 30.f, color);
    t->enableOutline(Color4B(0, 0, 0, 230), 3);
    t->setPositionY(sub.empty() ? 0.f : 12.f);
    box->addChild(t);
    if (!sub.empty()) {
        auto* s = label(sub, 13.f, PZCfg::kColText);
        s->enableOutline(Color4B(0, 0, 0, 230), 2);
        s->setPositionY(-22.f);
        box->addChild(s);
    }
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
    node->drawSolidRect(Vec2(-w * 0.5f, -h * 0.5f), Vec2(w * 0.5f, h * 0.5f), c4f(Color3B(10, 14, 24), 0.85f));
    if (value > 0.001f) {
        node->drawSolidRect(Vec2(-w * 0.5f, -h * 0.5f), Vec2(-w * 0.5f + w * value, h * 0.5f), c4f(fill));
    }
    node->drawRect(Vec2(-w * 0.5f, -h * 0.5f), Vec2(w * 0.5f, h * 0.5f), c4f(PZCfg::kColPanelEdge, 0.9f));
}

DrawNode* bar(float w, float h, float value, const Color3B& fill) {
    auto* d = DrawNode::create();
    redrawBar(d, w, h, value, fill);
    return d;
}

} // namespace PZUI
