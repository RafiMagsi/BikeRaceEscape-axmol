#include "V2/V2Dialogue.h"
#include "V2/V2Content.h"
#include "V2/V2Core.h"
#include "V2/V2Presentation.h"
#include "V2/V2Cinematic.h"
#include "Features/PZUI.h"
#include "Features/PZLayout.h"

#include <set>

USING_NS_AX;

namespace V2 {

namespace {
std::set<std::string> g_sessionShown;

std::string seenKey(const DialogueSeq& q, int dateKey) {
    if (q.policy == "daily") return "PZP_V2_dlg_" + q.id + "_d" + std::to_string(dateKey);
    return "PZP_V2_dlg_" + q.id + "_v" + std::to_string(q.version);
}

bool alreadySeen(const DialogueSeq& q, int dateKey) {
    if (q.policy == "always") return false;
    if (q.policy == "session") return g_sessionShown.count(q.id) > 0;
    return UserDefault::getInstance()->getBoolForKey(seenKey(q, dateKey).c_str(), false);
}

void markSeen(const DialogueSeq& q, int dateKey) {
    if (q.policy == "session") { g_sessionShown.insert(q.id); return; }
    if (q.policy == "always") return;
    UserDefault::getInstance()->setBoolForKey(seenKey(q, dateKey).c_str(), true);
    UserDefault::getInstance()->flush();
}
}

DialogueDirector* DialogueDirector::create() {
    auto* d = new DialogueDirector();
    if (d->init()) { d->autorelease(); return d; }
    delete d;
    return nullptr;
}

std::string DialogueDirector::pick(const DialogueContext& ctx) {
    if (ctx.replay) return "";
    Content::shared().load();
    auto* ud = UserDefault::getInstance();
    const bool firstRun = !ud->getBoolForKey("PZP_V2_first_run_done", false);
    ud->setBoolForKey("PZP_V2_first_run_done", true);
    const int last = ud->getIntegerForKey("PZP_V2_rv_cool_last", 0);          // 1 = the player won the last race, -1 = lost
    const bool trexSeen = ud->getBoolForKey("PZP_V2_trex_seen", false);
    const DialogueSeq* best = nullptr;
    for (const auto& kv : Content::shared().sequences()) {
        const DialogueSeq& q = kv.second;
        bool eligible = false;
        if (q.trigger == "first_run") eligible = firstRun;
        else if (q.trigger == "rival_last_loss") eligible = last < 0;
        else if (q.trigger == "rival_last_win") eligible = last > 0;
        else if (q.trigger == "trex_seen") eligible = trexSeen;
        else if (q.trigger == "daily_first_of_day") eligible = ctx.dailyRun;
        if (!eligible || alreadySeen(q, ctx.dateKey)) continue;
        if (!best || q.priority > best->priority) best = &q;
    }
    if (!best) return "";
    markSeen(*best, ctx.dateKey);                  // counted when chosen: leaving mid-way never repeats it endlessly
    return best->id;
}

bool DialogueDirector::show(const std::string& id, Node* host, std::function<void()> onDone) {
    cancel();
    auto it = Content::shared().sequences().find(id);
    if (!host || it == Content::shared().sequences().end() || it->second.lines.empty()) return false;
    if (!Content::shared().ensureGroup(Content::shared().dialogueSettings().preloadGroup)) return false;   // art missing: skip silently
    if (!Cinematic::shared().request(Cinematic::Owner::PreRunDialogue)) return false;
    _id = id;
    _line = 0;
    _done = std::move(onDone);
    _reduced = Presentation::reducedMotion();
    const Size win = Director::getInstance()->getWinSize();
    _root = Node::create();
    host->addChild(_root, 9000);

    auto* dim = LayerColor::create(Color4B(4, 6, 14, 190), win.width, win.height);
    _root->addChild(dim);
    const float panelH = win.height * 0.26f, panelW = std::min(win.width - 120.f, 820.f);
    const Rect safe = PZLayout::safeRect();
    const float cx = safe.getMidX();
    auto* panel = PZUI::roundedRect(panelW, panelH, 22.f, Color3B(14, 20, 40), 0.96f, Color3B(255, 188, 74), 1.f, 2.5f);
    panel->setPosition(Vec2(cx, panelH * 0.5f + 18.f));
    _root->addChild(panel, 5);
    _name = PZUI::heading("", 12.f, Color3B(255, 205, 80));
    _name->setAnchorPoint(Vec2(0.f, 0.5f));
    _name->setPosition(Vec2(cx - panelW * 0.5f + 34.f, panelH + 18.f - 30.f));
    _root->addChild(_name, 6);
    _text = PZUI::label("", 15.f, Color3B(245, 245, 250));
    _text->setAnchorPoint(Vec2(0.f, 0.5f));
    _text->setPosition(Vec2(cx - panelW * 0.5f + 34.f, panelH * 0.5f + 12.f));
    _root->addChild(_text, 6);
    _hint = PZUI::label("TAP TO CONTINUE", 7.f, Color3B(150, 160, 190));
    _hint->setAnchorPoint(Vec2(1.f, 0.5f));
    _hint->setPosition(Vec2(cx + panelW * 0.5f - 30.f, 34.f));
    _root->addChild(_hint, 6);
    // SKIP is always visible, in the top corner away from the thumb zone
    auto* skipBg = PZUI::roundedRect(150.f, 44.f, 14.f, Color3B(40, 48, 72), 0.92f, Color3B(255, 188, 74), 1.f, 2.f);
    const Vec2 sp(safe.getMaxX() - 100.f, win.height - 52.f);
    skipBg->setPosition(sp);
    _root->addChild(skipBg, 6);
    auto* skipL = PZUI::heading("SKIP", 11.f, Color3B(255, 255, 255));
    skipL->setPosition(sp);
    _root->addChild(skipL, 7);
    _skipX0 = sp.x - 85.f; _skipX1 = sp.x + 85.f; _skipY0 = sp.y - 30.f; _skipY1 = sp.y + 30.f;

    const auto& q = it->second;
    const float portraitH = win.height * 0.58f;
    for (int i = 0; i < 2 && i < static_cast<int>(q.speakers.size()); ++i) {
        const auto& sd = q.speakers[i];
        const std::string& anim = Content::shared().portrait(sd.portraitSet, "neutral");
        auto* s = Sprite::createWithSpriteFrameName(Content::shared().frame(anim, 0));
        if (!s) continue;
        s->setAnchorPoint(Vec2(0.5f, 0.f));
        const float sc = portraitH / std::max(1.f, s->getContentSize().height);
        s->setScale(sc);
        s->setFlippedX(sd.side == "right");                    // both face inward
        s->setPosition(Vec2(sd.side == "right" ? cx + panelW * 0.30f : cx - panelW * 0.30f, panelH + 28.f));
        s->setOpacity(0);
        _root->addChild(s, 3);
        _spr[i] = s;
    }
    auto* l = EventListenerTouchOneByOne::create();
    l->setSwallowTouches(true);
    l->onTouchBegan = [](Touch*, Event*) { return true; };
    l->onTouchEnded = [this](Touch* t, Event*) {
        const Vec2 p = t->getLocation();
        if (p.x >= _skipX0 && p.x <= _skipX1 && p.y >= _skipY0 && p.y <= _skipY1) finish(true);
        else advance();
    };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(l, _root);
    _root->setCascadeOpacityEnabled(true);
    _root->setOpacity(0);
    _root->runAction(FadeIn::create(0.18f));
    showLine();
    return true;
}

void DialogueDirector::showLine() {
    if (!_root) return;
    const auto& q = Content::shared().sequences().at(_id);
    if (_line >= static_cast<int>(q.lines.size())) { finish(false); return; }
    const DialogueLine& ln = q.lines[_line];
    const DialogueSpeaker* sp = nullptr;
    int idx = 0;
    for (int i = 0; i < static_cast<int>(q.speakers.size()); ++i) if (q.speakers[i].id == ln.speaker) { sp = &q.speakers[i]; idx = i; }
    if (!sp) { ++_line; showLine(); return; }
    _name->setString(sp->name);
    _name->setTextColor(Color4B(sp->color[0], sp->color[1], sp->color[2], 255));
    _text->setString(ln.text);
    _text->setOpacity(0);
    _text->runAction(FadeIn::create(0.12f));
    for (int i = 0; i < 2; ++i) {
        if (!_spr[i]) continue;
        const bool speaking = i == idx;
        const std::string& anim = speaking ? Content::shared().portrait(sp->portraitSet, ln.expression)
                                           : Content::shared().portrait(q.speakers[i].portraitSet, "neutral");
        const std::string& fr = Content::shared().frame(anim, 0);
        if (!fr.empty()) _spr[i]->setSpriteFrame(fr);
        _spr[i]->stopAllActions();
        const float baseScale = _spr[i]->getScale();
        _spr[i]->runAction(FadeTo::create(0.14f, speaking ? 255 : 120));
        _spr[i]->setColor(speaking ? Color3B::WHITE : Color3B(150, 156, 176));
        if (speaking && !_reduced) {                        // slow 2-3% push-in on the current speaker, Full motion only
            const float k = 1.f + Content::shared().dialogueSettings().pushIn * 0.01f;
            _spr[i]->runAction(ScaleTo::create(std::max(1.f, Content::shared().dialogueSettings().autoAdvance), baseScale * k));
        }
    }
    _root->stopActionByTag(77);
    auto* auto_ = Sequence::create(DelayTime::create(Content::shared().dialogueSettings().autoAdvance), CallFunc::create([this]() { advance(); }), nullptr);
    auto_->setTag(77);
    _root->runAction(auto_);
}

void DialogueDirector::advance() {
    if (!_root) return;
    ++_line;
    showLine();
}

void DialogueDirector::finish(bool skipped) {
    if (!_root) return;
    (void)skipped;
    Cinematic::shared().release(Cinematic::Owner::PreRunDialogue);
    Node* root = _root;
    _root = nullptr;
    _spr[0] = _spr[1] = nullptr;
    _name = _text = _hint = nullptr;
    Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(root);
    root->stopAllActions();
    root->runAction(Sequence::create(FadeOut::create(0.14f), RemoveSelf::create(), nullptr));
    auto done = std::move(_done);
    _done = nullptr;
    if (done) done();
}

void DialogueDirector::cancel() {
    if (!_root) return;
    Cinematic::shared().release(Cinematic::Owner::PreRunDialogue);
    Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(_root);
    _root->removeFromParent();
    _root = nullptr;
    _spr[0] = _spr[1] = nullptr;
    _name = _text = _hint = nullptr;
    _done = nullptr;
}

void DialogueDirector::onExit() {
    cancel();
    Node::onExit();
}

} // namespace V2
