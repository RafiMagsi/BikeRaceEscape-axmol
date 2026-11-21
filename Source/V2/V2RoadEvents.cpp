#include "V2/V2RoadEvents.h"
#include "V2/V2Config.h"
#include "V2/V2Content.h"
#include "V2/V2Metrics.h"
#include "V2/V2Cinematic.h"
#include "PZGGameFieldScene.h"
#include "PZGGamefieldPlatforms.h"
#include "PZGObstacle.h"
#include "Features/PZUI.h"
#include "Features/PZLayout.h"
#include "Features/PZGameConfig.h"
#include "Features/PZAudio.h"
#include "Features/PZHaptics.h"
#include "constants.h"

#include <algorithm>
#include <cmath>

USING_NS_AX;

namespace V2 {

namespace {
constexpr float kGravity = 1500.f;
float clampf(float v, float a, float b) { return std::max(a, std::min(b, v)); }
float approach(float v, float target, float maxStep) { return v + clampf(target - v, -maxStep, maxStep); }
float riderVisualHeight(const Context& ctx) {
    const float k = kDeviceScale();
    return clampf(ctx.riderBox.size.height * 2.05f, 145.f * k, 180.f * k);
}
}

const char* outcomeName(Outcome o) {
    switch (o) {
    case Outcome::Success: return "success";
    case Outcome::Failed: return "failed";
    case Outcome::SkippedNoRoad: return "skipped_no_road";
    case Outcome::AbortedDeath: return "aborted_death";
    case Outcome::CancelledScene: return "cancelled_scene";
    }
    return "?";
}

const char* tierName(Tier t) {
    switch (t) {
    case Tier::Intro: return "intro";
    case Tier::Standard: return "standard";
    case Tier::Expert: return "expert";
    }
    return "?";
}

static int variantCount(EventId id) {
    switch (id) {
    case EventId::DinoCrossing: case EventId::ConstructionChaos: case EventId::MeteorShower: case EventId::PizzaPanic: return 3;
    case EventId::CoolKidRace: case EventId::GrandmaTurbo: return 2;
    default: return 1;
    }
}

const char* eventName(EventId id) {
    switch (id) {
    case EventId::DinoCrossing: return "dino_crossing";
    case EventId::PizzaPanic:   return "pizza_panic";
    case EventId::CoolKidRace:  return "cool_kid_race";
    case EventId::ConstructionChaos: return "construction_chaos";
    case EventId::MeteorShower: return "meteor_shower";
    case EventId::GrandmaTurbo: return "grandma_turbo";
    case EventId::TrexChase: return "trex_chase";
    }
    return "?";
}

RoadEventDirector* RoadEventDirector::create(PZGGameFieldScene* scene, Node* hud) {
    auto* d = new RoadEventDirector();
    if (d->init(scene, hud)) {
        d->autorelease();
        return d;
    }
    delete d;
    return nullptr;
}

bool RoadEventDirector::init(PZGGameFieldScene* scene, Node* hud) {
    if (!Node::init()) return false;
    _scene = scene;
    _hud = hud;
    Content::shared().ensureGroup("gameplay_v2");          // atlas files / frame sequences come from CompleteGameInfo.plist v2.presentation
    {
        static bool reported = false;                      // one actionable report per process (debug and release log; never aborts a run)
        if (!reported) {
            reported = true;
            int errors = 0;
            const std::string rep = Content::shared().validate(&errors);
            if (errors > 0) AXLOGE("[V2:content] {} problem(s):\n{}", errors, rep);
        }
    }
    {
        std::vector<int> all;
        for (int i = 0; i < kEventCount; ++i) all.push_back(i);
        _bag.setItems(all);
    }
    return true;
}

PZGGamefieldPlatforms* RoadEventDirector::platform() const { return _scene ? _scene->platformLayer() : nullptr; }

bool RoadEventDirector::ground(float x, float& y) const {
    auto* p = platform();
    return p && p->groundYAt(x, y);
}

// ------------------------------------------------------------------------------------------------ run lifecycle

void RoadEventDirector::reset(uint32_t seed) {
    clear();
    _seed = seed;
    _rng.reseed(seed);
    _fxRng.reseed(seed ^ 0xA5A5A5A5u);
    _barks.resetHistory();
    {
        std::vector<int> all;
        for (int i = 0; i < kEventCount; ++i) {
            all.push_back(i);
            if (_pizzaBoost && i == static_cast<int>(EventId::PizzaPanic)) { all.push_back(i); all.push_back(i); }
        }
        _bag.setItems(all);
        for (int i = 0; i < kEventTotal; ++i) {
            std::vector<int> v;
            for (int k = 0; k < variantCount(static_cast<EventId>(i)); ++k) v.push_back(k);
            _variantBag[i].setItems(v);
        }
    }
    _rs = RunStats();
    _chain = 0;
    _chainT = 0.f;
    _chapter = -1;
    _nextClimaxM = cfg().climaxAtM;
    if (_chapterTitle) { _chapterTitle->removeFromParent(); _chapterTitle = nullptr; }
    _streak = 0;
    _lastLucky = false;
    _gagState = 0;
    _micro = Micro();
    _micro.cooldown = cfg().microEvery;
    _microNext = 0;
    _clock = 0.f;
    _successes = 0;
    _cool = cfg().firstEventAt * _breatherScale;
    _ambientCool = 14.f + _rng.range(0.f, 8.f);
    Analytics::shared().enableFileSink(FileUtils::getInstance()->getWritablePath() + "v2_analytics.jsonl", 262144);
    _speed = 300.f;
    Analytics::shared().log("v2_run_seed", {{"seed", std::to_string(seed)}, {"rules", std::to_string(cfg().rulesVersion)}, {"content", std::to_string(Content::shared().contentVersion())}});
}

void RoadEventDirector::clear(Outcome reason) {
    if (_active) {                                       // an unfinished event: log WHY it stopped (not a player failure)
        Analytics::shared().log("road_event_end", {{"event", eventName(_cur)}, {"variant", std::to_string(_variant)}, {"tier", tierName(_tier)},
                                                   {"outcome", outcomeName(reason)}, {"duration", std::to_string(static_cast<int>(_t))}});
    }
    if (_objPlate) { _objPlate->removeFromParent(); _objPlate = nullptr; _objLabel = nullptr; }
    if (_microPlate) { _microPlate->removeFromParent(); _microPlate = nullptr; _microLabel = nullptr; }
    _objText.clear();
    _micro.active = false;
    Cinematic::shared().release(Cinematic::Owner::Climax);
    for (auto& a : _actors) {
        if (a->root) a->root->removeFromParent();
    }
    _actors.clear();
    for (auto& b : _bubbles) {
        if (b.node) b.node->removeFromParent();
    }
    _bubbles.clear();
    if (_marker) {
        _marker->removeFromParent();
        _marker = nullptr;
        _markerBar = nullptr;
        _markerRival = nullptr;
        _markerPlayer = nullptr;
    }
    _active = false;
    _rivalId = 0;
}

void RoadEventDirector::shiftWorld(float dx) {
    for (auto& a : _actors) {
        if (a->kind == Kind::Ambient || a->kind == Kind::Rider) continue;   // screen-locked actors follow the rider, not the road
        a->x -= dx;
        a->gapX0 -= dx;
        a->gapX1 -= dx;
        if (a->root) a->root->setPositionX(a->x);
    }
}

void RoadEventDirector::clearNear(float behind, float ahead) {
    for (auto& a : _actors) {
        if (!a->hazard || a->launched) continue;
        launch(*a, _scene && _scene->character ? _scene->character->loc.x : 0.f);
    }
    (void)behind;
    (void)ahead;
}

void RoadEventDirector::notifyBoost(float amount) {
    if (_active && _cur == EventId::CoolKidRace && _raceState == 1) _pendingGain += amount;
}

float RoadEventDirector::hazardAhead(float x) const {
    float best = -1.f;
    for (const auto& a : _actors) {
        if (!a->root || !a->hazard || a->launched) continue;
        const float d = (a->x - a->w * 0.5f) - x;
        if (d > -a->w && (best < 0.f || d < best)) best = d;
    }
    return best;
}

bool RoadEventDirector::forceStart(const std::string& nameIn) {
    if (_active) return false;
    std::string name = nameIn;
    int variant = -1;
    const size_t colon = name.find(':');                  // "dino_crossing:1" forces variant 1 (QA)
    if (colon != std::string::npos) { variant = std::atoi(name.c_str() + colon + 1); name.resize(colon); }
    for (int i = 0; i < kEventTotal; ++i) {
        if (name == eventName(static_cast<EventId>(i))) {
            startEvent(static_cast<EventId>(i));
            if (variant >= 0 && variant < variantCount(static_cast<EventId>(i))) _variant = variant;
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------ road queries

bool RoadEventDirector::obstacleNear(float x, float range) const {
    auto* p = platform();
    if (!p) return false;
    for (int i = 0; i < p->obstacleSlots(); ++i) {
        PZGObstacle* o = p->obstacleAt(i);
        if (!o || o->dead || !o->destroyCharacter || !o->sprite) continue;
        const Rect b = o->worldBox();
        if (b.getMaxX() > x - range && b.getMinX() < x + range) return true;
    }
    return false;
}

// Flat, continuous road from x0 to x1 that is not littered with lethal obstacles.
bool RoadEventDirector::roadClear(float x0, float x1, float apex) const {
    (void)apex;
    float y0 = 0.f;
    if (!ground(x0, y0)) return false;
    for (float x = x0; x <= x1; x += 24.f) {
        float y = 0.f;
        if (!ground(x, y) || std::abs(y - y0) > 6.f) return false;
    }
    return !obstacleNear(0.5f * (x0 + x1), 0.5f * (x1 - x0) + 70.f);
}

// ------------------------------------------------------------------------------------------------ actors

RoadEventDirector::Actor* RoadEventDirector::spawn(Kind kind, const std::string& base, int frames, float x, float y,
                                                   float targetH, bool faceRight) {
    auto a = std::make_shared<Actor>();
    a->id = _nextId++;
    a->kind = kind;
    a->base = base;
    a->frames = frames;
    a->x = x;
    a->y = y;
    a->facingRight = faceRight;
    a->root = Node::create();
    const AnimDef* anim = Content::shared().animation(base);          // `base` is a semantic animation id
    if (!anim || anim->frames.empty()) {
        AXLOGW("[V2] animation '{}' is not registered", base);
        return nullptr;
    }
    a->frameNames = anim->frames;
    a->frames = static_cast<int>(anim->frames.size());
    a->fps = anim->fps;
    a->spr = Sprite::createWithSpriteFrameName(anim->frames.front());
    if (!a->spr) {
        AXLOGW("[V2] missing sprite frame {} (animation '{}')", anim->frames.front(), base);
        return nullptr;
    }
    a->spr->setAnchorPoint(Vec2(0.5f, 0.f));
    const float sc = targetH / std::max(1.f, a->spr->getContentSize().height);
    a->spr->setScale(sc);
    const bool naturalRight = kind == Kind::Rider || kind == Kind::Ambient;
    a->spr->setFlippedX(faceRight != naturalRight);
    a->root->addChild(a->spr);
    a->root->setPosition(Vec2(x, y));
    a->w = a->spr->getContentSize().width * sc * 0.58f;
    a->h = targetH * 0.86f;
    const int z = _scene->character_depth_z + (kind == Kind::Ambient || kind == Kind::Rider ? 0 : 1);
    _scene->addChild(a->root, z);
    _actors.push_back(a);
    return a.get();
}

RoadEventDirector::Actor* RoadEventDirector::find(int id) {
    for (auto& a : _actors) {
        if (a->id == id) return a.get();
    }
    return nullptr;
}

void RoadEventDirector::removeActor(Actor& a) {
    if (a.root) a.root->removeFromParent();
    a.root = nullptr;
    a.life = -1.f;      // swept by updateActors
}

void RoadEventDirector::animate(Actor& a, float dt) {
    if (a.extra) {
        const int f = static_cast<int>(_clock * 20.f) % 4;
        a.extra->setSpriteFrame(Content::shared().frame("flame", f));
    }
    if (a.frames <= 1 || !a.spr) return;
    a.t += dt;
    const int f = static_cast<int>(a.t * a.fps) % a.frames;
    a.spr->setSpriteFrame(a.frameNames[f % a.frameNames.size()]);
}

// Keeps a kinematic actor on the road; over a gap it flies a ballistic-looking hop between the two edges.
void RoadEventDirector::followRoad(Actor& a, float dt, bool hop) {
    float gy = 0.f;
    if (ground(a.x, gy)) {
        a.inGap = false;
        const float target = gy + a.laneLift;
        a.y = approach(a.y, target, (target > a.y ? 900.f : 1400.f) * dt);
        return;
    }
    if (!hop) {
        a.stopped = true;
        return;
    }
    if (!a.inGap) {
        a.inGap = true;
        a.gapX0 = a.x;
        a.gapY0 = a.y - a.laneLift;
        for (float x = a.x; x > a.x - 220.f; x -= 8.f) {          // last road behind
            float y = 0.f;
            if (ground(x, y)) { a.gapX0 = x; a.gapY0 = y; break; }
        }
        a.gapX1 = a.x + 700.f;
        a.gapY1 = a.gapY0;
        for (float x = a.x; x < a.x + 900.f; x += 12.f) {         // next road ahead
            float y = 0.f;
            if (ground(x, y)) { a.gapX1 = x; a.gapY1 = y; break; }
        }
    }
    const float span = std::max(40.f, a.gapX1 - a.gapX0);
    const float p = clampf((a.x - a.gapX0) / span, 0.f, 1.f);
    const float hopH = clampf(span * 0.28f, 46.f, 150.f);
    a.y = a.gapY0 + (a.gapY1 - a.gapY0) * p + 4.f * p * (1.f - p) * hopH + a.laneLift;
}

// Absorbed hit (shield / turbo / grace): the actor is knocked away with a comic spin instead of killing the rider.
void RoadEventDirector::launch(Actor& a, float sx) {
    (void)sx;
    if (a.launched) return;
    a.launched = true;
    a.hazard = false;
    if (!a.counted) { a.counted = true; ++_passed; }
    a.vx = 420.f + _rng.range(0.f, 160.f);
    a.vy = 620.f + _rng.range(0.f, 160.f);
    a.spin = _rng.range(360.f, 720.f);
    a.life = 1.4f;
    PZAudio::play("fx_smash", 0.8f, 0.08f);
}

void RoadEventDirector::say(const std::string& tag, int actorId) {
    const std::string line = _barks.pick(tag, _fxRng);   // cosmetic RNG: rival memory / barks never shift the gameplay sequence
    if (!line.empty()) bubble(line, actorId, 2.1f);
}

void RoadEventDirector::bubble(const std::string& text, int actorId, float life) {
    if (_bubbles.size() >= 3) {                       // one or two at a time; never a wall of text
        if (_bubbles.front().node) _bubbles.front().node->removeFromParent();
        _bubbles.erase(_bubbles.begin());
    }
    auto* node = Node::create();
    auto* label = PZUI::label(text, 11.f, Color3B(38, 24, 18));
    const float w = std::min(340.f, label->getContentSize().width + 30.f), h = label->getContentSize().height + 18.f;
    if (label->getContentSize().width + 30.f > w) label->setScale((w - 30.f) / label->getContentSize().width);
    node->addChild(PZUI::roundedRect(w, h, h * 0.45f, Color3B(255, 252, 240), 0.97f, Color3B(38, 24, 18), 1.f, 2.4f));
    auto* tail = DrawNode::create();
    Vec2 pts[3] = {Vec2(-9.f, -h * 0.5f + 1.f), Vec2(9.f, -h * 0.5f + 1.f), Vec2(0.f, -h * 0.5f - 11.f)};
    tail->drawSolidPoly(pts, 3, Color4F(1.f, 0.99f, 0.94f, 0.97f), 2.f, Color4F(0.15f, 0.09f, 0.07f, 1.f), true);
    node->addChild(tail);
    node->addChild(label, 2);
    node->setScale(0.2f);
    node->setCascadeOpacityEnabled(true);
    node->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.f)));
    _scene->addChild(node, 1400);
    Bubble b;
    b.node = node;
    b.actorId = actorId;
    b.life = life;
    b.off = Vec2(0.f, 0.f);
    _bubbles.push_back(b);
}

// ------------------------------------------------------------------------------------------------ scheduler

void RoadEventDirector::startEvent(EventId id) {
    _active = true;
    _cur = id;
    _t = 0.f;
    _step = 0;
    _wait = 0.f;
    _waited = 0.f;
    _score = _total = _passed = _target = 0;
    _riskGot = _riskNeed = _riskReward = 0;
    _riskLabel.clear();
    _evCoins = 0;
    _draft = 0.f;
    _drafted = false;
    _dodgesEv = 0;
    _lastCountedGroup = 0;
    _powerUpInEvent = false;
    _hitThisEvent = false;
    _rivalId = 0;
    _raceState = 0;
    _raceT = 0.f;
    _pendingGain = 0.f;
    _lead = -240.f;
    const Config& c = cfg();
    _tier = _lastDistance >= c.tierExpertAt ? Tier::Expert : (_lastDistance >= c.tierStandardAt ? Tier::Standard : Tier::Intro);
    // Variants: the first tier teaches the plain version; later tiers draw from a shuffle bag (never the same one twice in a row).
    _variant = (_tier == Tier::Intro) ? 0 : _variantBag[static_cast<int>(id)].next(_rng);
    // Expert-only patterns (variant 2) for Dino, Construction, Meteor and Pizza: they change the decision, not just the speed. Below the
    // expert tier the same bag draw maps to the standard variant 1, so the draw sequence (and replays) never depend on the tier.
    if (_variant == 2 && _tier != Tier::Expert) _variant = 1;
    if (_micro.active && _micro.type == 2) _micro.armedNoPowerUp = true;
    Metrics::firstEvent();
    if (id == EventId::TrexChase && Cinematic::shared().request(Cinematic::Owner::Climax) && _host.climaxReveal) _host.climaxReveal();   // owner granted: one wide shot; no punches / blast zooms during the set piece
    if (id == EventId::TrexChase) UserDefault::getInstance()->setBoolForKey("PZP_V2_trex_seen", true);   // unlocks the T-Rex dialogue next run
    Analytics::shared().log("road_event_start", {{"event", eventName(id)}, {"variant", std::to_string(_variant)}, {"tier", tierName(_tier)},
                                                 {"clock", std::to_string(static_cast<int>(_clock))}, {"dist", std::to_string(static_cast<int>(_lastDistance))}});
}

void RoadEventDirector::finishEvent(Outcome o) {
    Analytics::shared().log("road_event_end", {{"event", eventName(_cur)}, {"variant", std::to_string(_variant)}, {"tier", tierName(_tier)},
                                               {"outcome", outcomeName(o)}, {"duration", std::to_string(static_cast<int>(_t))},
                                               {"hit", _hitThisEvent ? "1" : "0"}, {"progress", std::to_string(_passed) + "/" + std::to_string(_target)},
                                               {"coins", std::to_string(_evCoins)}});
    if (o == Outcome::Success) {
        if (_cur == EventId::TrexChase) _rs.trexEscaped = true;
        {   // event mastery stamp: the first clean success on each event / tier is recorded once and paid a small one-time bonus
            const std::string key = std::string("PZP_V2_stamp_") + eventName(_cur) + "_" + tierName(_tier);
            if (!UserDefault::getInstance()->getBoolForKey(key.c_str(), false)) {
                UserDefault::getInstance()->setBoolForKey(key.c_str(), true);
                reward(10, Vec2(_lastRiderX + 40.f, 360.f), "NEW STAMP! +10");
                if (_rs.newStamp.empty()) _rs.newStamp = std::string(eventName(_cur)) + "/" + tierName(_tier);
                Analytics::shared().log("mastery_stamp", {{"event", eventName(_cur)}, {"tier", tierName(_tier)}});
            }
        }
        ++_successes;
        ++_streak;
        if (_streak >= 3) {                               // ROAD STREAK: three clean events in a row, growing payout
            const int bonus = std::min(20, 5 * (_streak - 2));
            reward(bonus, Vec2(_lastRiderX + 30.f, 400.f), "ROAD STREAK x" + std::to_string(_streak) + "! +" + std::to_string(bonus));
            PZAudio::play("fx_levelup", 0.5f, 0.1f);
        }
    } else if (o == Outcome::Failed) {
        _streak = 0;
    }
    // objective plate: a short result, then it goes away
    if (o == Outcome::Success) flashResult("CLEARED!", Color3B(110, 235, 120));
    else if (o == Outcome::Failed) flashResult("MISSED IT", Color3B(255, 150, 90));
    else hideObjective();
    // micro challenge "clear the next event without a power-up"
    if (_micro.active && _micro.type == 2 && _micro.armedNoPowerUp) {
        if (o == Outcome::Success && !_powerUpInEvent) {
            ++_rs.microDone;
            reward(cfg().rewardMicro, Vec2(_lastRiderX + 20.f, 330.f), "CHALLENGE! +" + std::to_string(cfg().rewardMicro));
            Analytics::shared().log("micro_challenge_end", {{"type", "clean_event"}, {"success", "1"}});
        } else if (o != Outcome::SkippedNoRoad) {
            Analytics::shared().log("micro_challenge_end", {{"type", "clean_event"}, {"success", "0"}});
        }
        if (o != Outcome::SkippedNoRoad) { _micro.active = false; _micro.cooldown = cfg().microEvery; }
    }
    if (o != Outcome::SkippedNoRoad || _cur == EventId::TrexChase) Cinematic::shared().release(Cinematic::Owner::Climax);
    _active = false;
    _cool = _rng.range(cfg().breatherMin, cfg().breatherMax) * _breatherScale;
    if (_marker) {
        _marker->removeFromParent();
        _marker = nullptr;
        _markerBar = nullptr;
        _markerRival = nullptr;
        _markerPlayer = nullptr;
    }
}

void RoadEventDirector::updateScheduler(const Context& ctx) {
    if (_active) return;
    _cool -= ctx.dt;
    if (_cool > 0.f || ctx.ridesActive || ctx.controlLocked) return;
    for (auto& a : _actors) {
        if (a->kind != Kind::Ambient) return;           // leftovers (launched dinos) finish first
    }
    if (_lastDistance >= _nextClimaxM) {                  // the chapter climax: first safe window after Crown Hunt, then every climaxEveryM
        _nextClimaxM = _lastDistance + cfg().climaxEveryM;
        startEvent(EventId::TrexChase);
        return;
    }
    // draw from the shuffle bag until the event belongs to the current chapter (bounded; deterministic); a chapter's first listed
    // event is the registered fallback
    const int ch = chapterIndexAt(_lastDistance);
    EventId pick = static_cast<EventId>(_bag.next(_rng));
    for (int tries = 0; tries < 14 && !chapterAllows(ch, pick); ++tries) pick = static_cast<EventId>(_bag.next(_rng));
    if (!chapterAllows(ch, pick) && ch >= 0 && !cfg().chapters[ch].events.empty()) {
        for (int i = 0; i < kEventCount; ++i) {
            if (cfg().chapters[ch].events.front() == eventName(static_cast<EventId>(i))) pick = static_cast<EventId>(i);
        }
    }
    startEvent(pick);
}

int RoadEventDirector::chapterIndexAt(float distance) const {
    const auto& cs = cfg().chapters;
    int idx = -1;
    for (int i = 0; i < static_cast<int>(cs.size()); ++i) if (distance >= cs[i].fromM) idx = i;
    return idx;
}

bool RoadEventDirector::chapterAllows(int chapter, EventId id) const {
    if (chapter < 0 || cfg().chapters[chapter].events.empty()) return true;
    for (const auto& e : cfg().chapters[chapter].events) if (e == eventName(id)) return true;
    return false;
}

// One-second wall-band title + a soft sting (PRODUCT_PLAN 24.3 #3): never takes control, never hides the road.
void RoadEventDirector::announceChapter(const std::string& title) {
    if (!_hud || title.empty()) return;
    if (_chapterTitle) _chapterTitle->removeFromParent();
    const Rect safe = PZLayout::safeRect();
    const float S = PZLayout::uiScale();
    auto* l = PZUI::heading(title, 22.f, Color3B(255, 205, 80));
    l->setScale(S * 0.6f);
    l->setOpacity(0);
    l->setPosition(Vec2(safe.getMidX(), safe.getMaxY() - safe.size.height * 0.46f));   // below the stage banner, clear of the rider column
    _hud->addChild(l, 18);
    _chapterTitle = l;
    l->runAction(Sequence::create(Spawn::create(FadeIn::create(0.18f), ScaleTo::create(0.18f, S), nullptr), DelayTime::create(1.0f),
                                  FadeOut::create(0.3f), CallFunc::create([this, l]() { if (_chapterTitle == l) _chapterTitle = nullptr; }),
                                  RemoveSelf::create(), nullptr));
    PZAudio::play("fx_stage", 0.35f, 0.1f);
}

void RoadEventDirector::update(const Context& ctxIn) {
    if (!_enabled || !_scene || !_scene->character) return;
    Context ctx = ctxIn;
    if (ctx.controlLocked) {
        ctx.dt = 0.f;
        ctx.scroll = 0.f;
    }
    if (ctx.dt > 0.f && ctx.scroll > 0.f) _speed += (ctx.scroll / ctx.dt - _speed) * 0.08f;
    if (!ctx.controlLocked) _clock += ctxIn.dt;
    _lastRiderX = ctx.rider.x;
    _lastDistance = ctx.distance;

    if (!ctx.controlLocked) {
        const int ch = chapterIndexAt(ctx.distance);
        if (ch != _chapter) {
            _chapter = ch;
            if (ch >= 0) {
                announceChapter(cfg().chapters[ch].title);
                Analytics::shared().log("chapter_start", {{"chapter", cfg().chapters[ch].id}, {"dist", std::to_string(static_cast<int>(ctx.distance))}});
            }
        }
    }
    updateScheduler(ctx);
    if (_active) {
        _t += ctx.dt;
        // Fail-safe: no event may run forever (a delivery that never finds road to drop on, a rider that cannot spawn...). Seen in a
        // full-run review: one Pizza Panic held the scheduler for over a minute and blocked every later chapter and the T-Rex.
        static const float kCap[7] = {40.f, 32.f, 40.f, 40.f, 40.f, 28.f, 70.f};
        if (_t > kCap[static_cast<int>(_cur)]) {
            Analytics::shared().log("road_event_timeout", {{"event", eventName(_cur)}, {"variant", std::to_string(_variant)}, {"progress", std::to_string(_passed) + "/" + std::to_string(_target)}});
            for (auto& a2 : _actors) {
                if (a2->kind != Kind::Ambient || a2->id == _rivalId) removeActor(*a2);
            }
            _rivalId = 0;
            finishEvent(Outcome::SkippedNoRoad);
        }
    }
    if (_active) {
        switch (_cur) {
        case EventId::DinoCrossing: updateDino(ctx); break;
        case EventId::PizzaPanic:   updatePizza(ctx); break;
        case EventId::CoolKidRace:  updateRace(ctx); break;
        case EventId::ConstructionChaos: updateConstruction(ctx); break;
        case EventId::MeteorShower: updateMeteor(ctx); break;
        case EventId::GrandmaTurbo: updateGrandma(ctx); break;
        case EventId::TrexChase:    updateTrex(ctx); break;
        }
    }
    updateAmbient(ctx);
    updateActors(ctx);
    updateMicro(ctx);
    updateObjectiveHud(ctx.dt);
    if (!ctx.controlLocked) hitTest(ctx);
    updateBubbles(ctx.dt, ctx);
}

// ------------------------------------------------------------------------------------------------ Dino Crossing

// Variant 0 "classic": sign, three small dinos, one big straggler.  Variant 1 "stampede" (standard tier+): six dinos in a
// tighter rhythm, with a HIGH ROLLER coin line hanging over the middle of the pack (optional: a high jump to grab it).
void RoadEventDirector::updateDino(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float spawnX = winW + 110.f;
    const float k = kDeviceScale();
    const bool stampede = _variant >= 1;
    const int n = stampede ? 6 : 4;
    if (_step == 0) {
        _waited += ctx.dt;
        if (roadClear(spawnX - 90.f, spawnX + 260.f, ctx.apex)) {
            float gy = 0.f;
            ground(spawnX, gy);
            if (Actor* s = spawn(Kind::Sign, "sign_dino", 1, spawnX, gy, 118.f * k, false)) say("dino", s->id);
            _step = 1;
            _target = n;
            _wait = telegraphLead(1.9f);                  // the sign passes well before the first dino arrives
            _waited = 0.f;
        } else if (_waited > 7.f) {
            finishEvent(Outcome::SkippedNoRoad);          // no clean stretch of road: skip the event
        }
        return;
    }
    setObjective(std::string(_variant == 2 ? "PACK HUNT!  " : (stampede ? "STAMPEDE!  " : "DINO CROSSING  ")) + std::to_string(std::min(_passed, _target)) + "/" + std::to_string(_target) +
                 (_riskNeed > 0 ? "\n" + _riskLabel + "  " + std::to_string(_riskGot) + "/" + std::to_string(_riskNeed) : std::string()));
    if (_step >= 1 && _step <= n) {
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        if (!roadClear(spawnX - 70.f, spawnX + 150.f, ctx.apex)) {
            if (_waited > 4.f) { ++_step; _waited = 0.f; _wait = 0.4f; --_target; }   // give up on this one, keep the script moving
            return;
        }
        const bool big = _step == n;
        float gy = 0.f;
        ground(spawnX, gy);
        const float h = big ? std::min(0.62f * ctx.apex, 118.f * k) : std::min(0.42f * ctx.apex, 78.f * k);
        if (_variant == 2 && (_step == 2 || _step == 4) && !big) {          // PACK HUNT (expert): a second dino right behind the first
            if (Actor* d2 = spawn(Kind::DinoSmall, "dino_small", 4, spawnX + 140.f, gy, h, false)) {
                d2->hazard = true; d2->vx = -54.f; d2->fps = 9.f; d2->group = _nextGroup++; ++_total; ++_target;
            }
        }
        if (Actor* d = spawn(big ? Kind::DinoBig : Kind::DinoSmall, big ? "dino_big" : "dino_small", 4, spawnX, gy, h, false)) {
            d->hazard = true;
            d->vx = big ? -34.f : -54.f;                  // dinos wander toward you a little
            d->fps = big ? 6.f : 9.f;
            d->group = _nextGroup++;
            ++_total;
        }
        if (stampede && _step == 3) {                     // the risky line: coins high over the middle of the pack
            spawnRiskLine(spawnX - 70.f, gy, ctx.apex, 5, 0.80f, 46.f, _nextGroup++);
            _riskReward = cfg().rewardHighRoller;
            _riskLabel = "HIGH ROLLER";
        }
        const float gapMin = stampede ? (_tier == Tier::Expert ? 1.05f : 1.15f) : 1.25f;
        ++_step;
        _waited = 0.f;
        _wait = big ? 0.f : _rng.range(gapMin, gapMin + 0.35f);
        return;
    }
    // all spawned: finished when every dino has gone by (or been launched away) and the coin line is resolved
    for (auto& a : _actors) {
        if (a->kind == Kind::DinoSmall || a->kind == Kind::DinoBig || a->kind == Kind::BonusCoin) return;
    }
    const bool clean = !_hitThisEvent;
    settleRisk(ctx);
    if (clean && _total > 0) {
        reward(cfg().rewardDino, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "CLEARED! +" + std::to_string(cfg().rewardDino));
        say("cleared", 0);
    }
    finishEvent(clean);
}

// ------------------------------------------------------------------------------------------------ Pizza Panic

void RoadEventDirector::updatePizza(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float k = kDeviceScale();
    Actor* r = find(_rivalId);
    if (_step == 0) {                                     // wait for road under the entry point, then roll in from the right
        _waited += ctx.dt;
        float gy = 0.f;
        if (ground(winW - 140.f, gy)) {
            r = spawn(Kind::Rider, "rider_delivery", 4, winW + 190.f, gy, riderVisualHeight(ctx), true);
            if (r) {
                r->fps = 12.f;
                r->laneLift = 20.f;                   // back lane: he never rides through the player's obstacles
                _rivalId = r->id;
                say("pizza_ok", r->id);
                _step = 1;
                _waited = 0.f;
            }
        } else if (_waited > 6.f) {
            finishEvent(Outcome::SkippedNoRoad);
        }
        return;
    }
    if (!r) { finishEvent(Outcome::Failed); return; }
    const bool dbl = _variant >= 1;
    const bool triple = _variant == 2;                    // TRIPLE ORDER (expert): three boxes per drop, 8 of 12 needed                       // variant 1 "double order": boxes drop in pairs, six of nine needed
    const int need = triple ? 8 : (dbl ? 6 : 5), maxBoxes = triple ? 12 : (dbl ? 9 : 7);
    setObjective(std::string(triple ? "TRIPLE ORDER  " : (dbl ? "DOUBLE ORDER  " : "PIZZA PANIC  ")) + std::to_string(_score) + "/" + std::to_string(need) + " CAUGHT");
    const float targetX = std::min(winW - 150.f, ctx.rider.x + 330.f);
    if (_step == 1) {                                     // delivering: ride ahead of the player, dropping boxes
        r->x = approach(r->x, targetX + std::sin(_t * 1.3f) * 26.f, 260.f * ctx.dt);
        _wait -= ctx.dt;
        _waited += ctx.dt;                                // time since the last drop: a delivery that cannot find road gives up (root cause of the 67 s stall)
        if (_wait <= 0.f && _total < maxBoxes && r->x < winW - 30.f) {
            // where would a box land? (world drift while it falls)
            const float fallT = 0.78f;
            const float landX = r->x - 24.f - _speed * fallT;
            float gy = 0.f;
            if (ground(landX, gy) && landX > ctx.rider.x - 30.f) {
                if (Actor* b = spawn(Kind::Box, "pizza_box", 1, r->x - 24.f, r->y + 52.f * k, 34.f * k, false)) {
                    b->vy = 260.f;
                    b->grounded = false;
                    b->hazard = false;
                    b->w = 40.f * k;
                    b->h = 34.f * k;
                    b->spin = _rng.range(-120.f, 120.f);
                    if (_total >= 2 && _rng.chance(0.2f)) { b->group = 9; b->spr->setColor(Color3B(255, 215, 70)); }   // GOLDEN SLICE: worth more
                    ++_total;
                    _waited = 0.f;
                    _wait = dbl ? 1.8f : 1.5f;
                    PZAudio::play("fx_powerup", 0.25f, 0.2f);
                }
                for (int extra = 1; extra < (triple ? 3 : (dbl ? 2 : 1)); ++extra) {         // the 2nd (and 3rd) box of the drop
                    float gy2 = 0.f;
                    const float off = 75.f * extra;
                    if (_total < maxBoxes && ground(landX - off, gy2) && landX - off > ctx.rider.x - 30.f) {
                        if (Actor* b2 = spawn(Kind::Box, "pizza_box", 1, r->x - 24.f - off, r->y + (52.f + 8.f * extra) * k, 34.f * k, false)) {
                            b2->vy = 260.f; b2->grounded = false; b2->hazard = false;
                            b2->w = 40.f * k; b2->h = 34.f * k; b2->spin = _rng.range(-120.f, 120.f);
                            ++_total;
                        }
                    }
                }
            } else {
                _wait = 0.25f;                            // no safe landing spot right now: try again in a moment
            }
        }
        if (_waited > 7.f && _total < maxBoxes) {
            Analytics::shared().log("pizza_gave_up", {{"dropped", std::to_string(_total)}, {"need", std::to_string(maxBoxes)}});
            _total = maxBoxes;                                // no landing spot for 7 s: wrap the delivery up instead of stalling the run
        }
        if (_total >= maxBoxes && _wait <= 0.f) { _step = 2; _wait = 2.4f; }
        return;
    }
    // delivery done: he speeds off, the payoff line comes up, boxes still lying on the road can be collected
    _wait -= ctx.dt;
    r->x += (240.f + 180.f * (2.4f - std::max(0.f, _wait))) * ctx.dt;
    if (r->x > winW + 220.f) {
        removeActor(*r);
        _rivalId = 0;
        const bool good = _score >= need;
        _rs.boxesMissed += std::max(0, _total - _score);
        if (good) reward(cfg().rewardPizza, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "PIZZA DELIVERED! +" + std::to_string(cfg().rewardPizza));
        say(good ? "pizza_ok" : "pizza_cold", 0);
        finishEvent(good);
    }
}

// ------------------------------------------------------------------------------------------------ Cool Kid race

void RoadEventDirector::buildRaceMarker() {
    if (_marker || !_hud) return;
    _marker = Node::create();
    _markerBar = DrawNode::create();
    _markerBar->drawSolidRect(Vec2(-120.f, -5.f), Vec2(120.f, 5.f), Color4F(0.f, 0.f, 0.f, 0.45f));
    _markerBar->drawRect(Vec2(-120.f, -5.f), Vec2(120.f, 5.f), Color4F(1.f, 0.8f, 0.3f, 0.9f));
    _marker->addChild(_markerBar);
    auto* name = PZUI::heading("COOL KID   " + std::to_string(rivalWins()) + " - " + std::to_string(rivalLosses()), 6.f, Color3B(120, 190, 255));
    name->setPosition(Vec2(0.f, 20.f));
    _marker->addChild(name);
    auto* player = DrawNode::create();
    player->drawSolidCircle(Vec2::ZERO, 9.f, 0.f, 20, Color4F(1.f, 0.82f, 0.2f, 1.f));
    player->drawCircle(Vec2::ZERO, 9.f, 0.f, 20, false, Color4F(0.15f, 0.09f, 0.07f, 1.f), 1.6f);
    _markerPlayer = player;
    _marker->addChild(player, 2);
    if (auto* rv = Sprite::createWithSpriteFrameName(Content::shared().frame("rider_cool", 0))) {
        rv->setScale(26.f / rv->getContentSize().height);
        _markerRival = rv;
        _marker->addChild(rv, 3);
    }
    _hud->addChild(_marker, 20);
    setObjective(_variant == 1 ? "WHEELIE DUEL\nHANG IN THE AIR TO CATCH HIM" : "RACE  OVERTAKE COOL KID");
    layoutHud();
}

void RoadEventDirector::layoutHud() {
    const Rect safe = PZLayout::safeRect();
    const float S = PZLayout::uiScale();
    // left column: coin counter, combo (-162), then the objective plate (-206) and the micro challenge plate (-256)
    if (_objPlate && _objPlate->getNumberOfRunningActions() == 0) _objPlate->setScale(S);
    // The plates sit in the wall band to the RIGHT of the rider column (rider ~ screen x/5, T-Rex left of it): never over the rider's
    // head, the camera cut-out or the left-edge actors.
    const float plateX = std::max(safe.getMinX() + 10.f * S, safe.getMinX() + safe.size.width * 0.27f);
    if (_objPlate) _objPlate->setPosition(Vec2(plateX, safe.getMaxY() - 208.f * S));
    if (_microPlate && _microPlate->getNumberOfRunningActions() == 0) _microPlate->setScale(S);
    if (_microPlate) _microPlate->setPosition(Vec2(plateX, safe.getMaxY() - (_objPlate ? 262.f : 208.f) * S));
    if (!_marker) return;
    _marker->setScale(S);
    _marker->setPosition(Vec2(safe.getMidX(), safe.getMaxY() - 90.f * S));
}

void RoadEventDirector::updateRaceMarker() {
    if (!_marker || !_markerRival) return;
    const float f = clampf(_lead / cfg().raceLoseLead, -1.f, 1.f);
    _markerRival->setPositionX(f * 110.f);
    _markerPlayer->setPositionX(0.f);
}

void RoadEventDirector::updateRace(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    Actor* r = find(_rivalId);
    if (_step == 0) {
        float gy = 0.f;
        if (!ground(ctx.rider.x, gy)) {                  // wait until the rider is on solid road
            _waited += ctx.dt;
            if (_waited > 8.f) finishEvent(Outcome::SkippedNoRoad);
            return;
        }
        r = spawn(Kind::Rider, "rider_cool", 4, ctx.rider.x - 240.f, gy, riderVisualHeight(ctx), true);
        if (!r) { finishEvent(Outcome::SkippedNoRoad); return; }
        r->fps = 13.f;
        r->laneLift = 8.f;
        _rivalId = r->id;
        _lead = -240.f;
        _raceState = 0;
        _raceT = 0.f;
        _step = 1;
        buildRaceMarker();
        return;
    }
    if (!r) { finishEvent(Outcome::Failed); return; }
    _raceT += ctx.dt;
    switch (_raceState) {
    case 0: {                                             // safe introduction: he overtakes you and takes the lead
        _lead += (cfg().raceStartLead + 240.f) / 1.9f * ctx.dt;
        if (_lead > 30.f && !_hitThisEvent) {
            const int w = rivalWins(), l = rivalLosses();
            say(w + l == 0 ? "rival_intro" : (w > l ? "rival_grudge" : "rival_taunt"), r->id);   // he remembers you
            _hitThisEvent = true;
        }
        if (_raceT >= 1.9f) { _raceState = 1; _raceT = 0.f; _hitThisEvent = false; _lead = cfg().raceStartLead; }
        break;
    }
    case 1: {                                             // racing: he drifts away, you close in with coins, near misses, air
        float gain = std::min(_pendingGain, 280.f * ctx.dt);
        _pendingGain -= gain;
        if (ctx.airborne) gain += cfg().gainAirPerSec * (_variant == 1 ? 2.4f : 1.f) * ctx.dt;      // wheelie duel: air time is the main weapon
        _lead += cfg().raceDrift * (_variant == 1 ? 1.6f : 1.f) * ctx.dt - gain;
        if (_lead <= cfg().raceWinLead) {
            _raceState = 2;
            _raceT = 0.f;
            reward(cfg().rewardOvertake, Vec2(ctx.rider.x + 30.f, ctx.rider.y + 80.f), "OVERTAKE! +" + std::to_string(cfg().rewardOvertake));
            noteRivalResult(true);
            _rs.rivalBeaten = true;
            if (rivalWins() % 3 == 0) {                       // rematch milestone: every third win over him
                reward(25, Vec2(ctx.rider.x + 30.f, ctx.rider.y + 130.f), "RIVAL x" + std::to_string(rivalWins()) + "! +25");
            }
            say("rival_win", r->id);
            PZAudio::play("fx_stage", 0.6f, 0.2f);
            PZHaptics::play(PZHaptics::Kind::Success);
            _score = 1;
        } else if (_lead >= cfg().raceLoseLead || _raceT >= cfg().raceMaxTime) {
            _raceState = 3;
            _raceT = 0.f;
            say("rival_loss", r->id);
            noteRivalResult(false);
        }
        break;
    }
    case 2:                                               // he tumbles off the back of the screen
        _lead -= 380.f * ctx.dt;
        r->root->setRotation(r->root->getRotation() + 220.f * ctx.dt);
        if (_lead < -420.f) { removeActor(*r); _rivalId = 0; finishEvent(true); return; }
        break;
    case 3:                                               // he rides away
        _lead += 520.f * ctx.dt;
        if (_lead > winW + 200.f) { removeActor(*r); _rivalId = 0; finishEvent(Outcome::Failed); return; }
        break;
    }
    r->x = ctx.rider.x + _lead;
    if (_raceState <= 1 || _raceState == 3) {
        r->root->setRotation(_lead > 0.f ? -7.f - 2.f * std::sin(_t * 8.f) : 0.f);   // wheelie while ahead
    }
    updateRaceMarker();
}

// ------------------------------------------------------------------------------------------------ Construction Chaos

// Variant 0 "classic": cones, a rolling paint drum, cones, a (harmless) worker; low SHORT HOPS coins over the last cones.
// Variant 1 "flagger": a worker with a STOP/GO paddle. The paddle flickers while he decides and LOCKS at least
// telegraphFloor seconds before you reach him: green = he steps aside (run through), red = he is a hazard (jump him).
// A TRICK LINE of coins hangs high over the first cone cluster.
static void drawLamp(DrawNode* d, bool stop) {
    d->clear();
    const Color4F c = stop ? Color4F(0.95f, 0.18f, 0.16f, 1.f) : Color4F(0.2f, 0.85f, 0.3f, 1.f);
    d->drawSolidCircle(Vec2::ZERO, 12.f, 0.f, 20, c);
    d->drawCircle(Vec2::ZERO, 12.f, 0.f, 20, false, Color4F(0.15f, 0.09f, 0.07f, 1.f), 2.f);
    if (stop) d->drawSolidRect(Vec2(-7.f, -2.f), Vec2(7.f, 2.f), Color4F(1.f, 1.f, 1.f, 1.f));
    else d->drawSolidPoly(std::vector<Vec2>{Vec2(-5.f, -6.f), Vec2(7.f, 0.f), Vec2(-5.f, 6.f)}.data(), 3, Color4F(1.f, 1.f, 1.f, 1.f));
}

void RoadEventDirector::updateConstruction(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float spawnX = winW + 110.f;
    const float k = kDeviceScale();
    const bool flagger = _variant >= 1;
    const int n = _variant == 2 ? 4 : (flagger ? 3 : 4);          // variant 2 DOUBLE FLAGGER (expert): a second paddle after the drum
    if (_step == 0) {
        _waited += ctx.dt;
        if (roadClear(spawnX - 90.f, spawnX + 300.f, ctx.apex)) {
            float gy = 0.f;
            ground(spawnX, gy);
            if (Actor* s = spawn(Kind::Sign, "sign_work", 1, spawnX, gy, 112.f * k, false)) say("construction", s->id);
            _step = 1;
            _target = n;
            _wait = telegraphLead(1.9f);
            _waited = 0.f;
        } else if (_waited > 7.f) {
            finishEvent(Outcome::SkippedNoRoad);
        }
        return;
    }
    setObjective(std::string(_variant == 2 ? "DOUBLE FLAGGER  " : (flagger ? "READ THE PADDLE  " : "ROAD WORKS  ")) + std::to_string(std::min(_passed, _target)) + "/" + std::to_string(_target) +
                 (_riskNeed > 0 ? "\n" + _riskLabel + "  " + std::to_string(_riskGot) + "/" + std::to_string(_riskNeed) : std::string()));
    if (_step >= 1 && _step <= n) {
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        if (!roadClear(spawnX - 70.f, spawnX + 240.f, ctx.apex)) {
            if (_waited > 4.f) { ++_step; _waited = 0.f; _wait = 0.4f; --_target; }
            return;
        }
        float gy = 0.f;
        ground(spawnX, gy);
        const float coneH = std::min(0.30f * ctx.apex, 46.f * k);
        const int kind = flagger ? (_step == 1 ? 0 : (_step == 2 ? 3 : (_step == 4 ? 3 : 1))) : (_step == 1 || _step == 3 ? 0 : (_step == 2 ? 1 : 3));   // 0 cones, 1 barrel, 3 worker
        if (kind == 0) {                                      // three cones close together: one jump clears all of them
            const int g = _nextGroup++;
            for (int i = 0; i < 3; ++i) {
                if (Actor* c = spawn(Kind::Cone, "cone", 1, spawnX + i * 34.f, gy, coneH, false)) {
                    c->hazard = true;
                    c->w = 20.f * k;
                    c->group = g;
                    ++_total;
                }
            }
            if (flagger && _step == 1) {                      // risky: a high trick line over the cones
                spawnRiskLine(spawnX - 20.f, gy, ctx.apex, 4, 0.82f, 40.f, _nextGroup++);
                _riskReward = cfg().rewardEdgeRun;
                _riskLabel = "TRICK LINE";
            } else if (!flagger && _step == 3) {              // safe: a low arc that rewards short hops
                spawnRiskLine(spawnX - 10.f, gy, ctx.apex, 3, 0.50f, 36.f, _nextGroup++);
                _riskReward = cfg().rewardConstruction / 2;
                _riskLabel = "SHORT HOPS";
            }
            _wait = _rng.range(1.4f, 1.8f);
        } else if (kind == 1) {                               // a paint drum rolls toward you: timing, not just distance
            if (Actor* b = spawn(Kind::Barrel, "barrel", 1, spawnX, gy, std::min(0.34f * ctx.apex, 54.f * k), false)) {
                b->hazard = true;
                b->vx = -170.f;
                b->w = b->h * 0.8f;
                b->group = _nextGroup++;
                ++_total;
            }
            _wait = 1.7f;
        } else if (flagger) {                                 // the flagger with the STOP/GO paddle
            if (Actor* w = spawn(Kind::Worker, "worker", 2, spawnX, gy, 74.f * k, false)) {
                w->fps = 3.f;
                w->w = 46.f * k;
                w->group = _nextGroup++;
                w->fallDur = _rng.chance(0.5f) ? 1.f : 0.f;    // final paddle: 1 = STOP (hazard), 0 = GO (safe)
                w->vy = -1.f;                                  // lamp state drawn: -1 none, 0 GO, 1 STOP
                w->lamp = DrawNode::create();
                w->lamp->setPosition(Vec2(0.f, w->h + 16.f));
                w->root->addChild(w->lamp, 3);
                say("construction", w->id);
            }
            _wait = 1.9f;
        } else {                                              // the worker who turned the sign the wrong way
            if (Actor* w = spawn(Kind::Worker, "worker", 2, spawnX, gy, 74.f * k, false)) {
                w->laneLift = 20.f;
                w->fps = 3.f;
                w->y = gy + 20.f;
                say("construction", w->id);
            }
            _wait = 0.f;
        }
        ++_step;
        _waited = 0.f;
        return;
    }
    for (auto& a : _actors) {
        if (a->kind == Kind::Cone || a->kind == Kind::Barrel || a->kind == Kind::Worker || a->kind == Kind::Sign || a->kind == Kind::BonusCoin) return;
    }
    const bool clean = !_hitThisEvent;
    settleRisk(ctx);
    if (clean && _total > 0) reward(cfg().rewardConstruction, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "SITE CLEARED! +" + std::to_string(cfg().rewardConstruction));
    finishEvent(clean);
}

// ------------------------------------------------------------------------------------------------ Meteor Shower

void RoadEventDirector::impact(Actor& m, const Context& ctx) {
    float gy = m.targetY;
    ground(m.x, gy);
    const float k = kDeviceScale();
    // CHAIN REACTION: the blast flings nearby road hazards (cones, barrels, ...) away, clearing the lane for the rider.
    if (_cur == EventId::MeteorShower) {
        int flung = 0;
        for (auto& o : _actors) {
            if (o->root && o->hazard && !o->launched && o->kind != Kind::Rock && o->kind != Kind::Meteor && std::abs(o->x - m.x) < 170.f) { launch(*o, ctx.rider.x); ++flung; }
        }
        if (flung > 0) reward(3, Vec2(m.x, gy + 120.f), "CHAIN REACTION +3");
    }
    // Outcomes vary: usually a rock, but one strike in four is a LUCKY METEOR that cracks open into a gold arc (never in the
    // T-Rex chase, which stays a pure dodge test, and only when the previous strike was a rock so luck never comes in pairs).
    const bool lucky = _cur == EventId::MeteorShower && !_lastLucky && _rng.chance(0.25f);
    _lastLucky = lucky;
    if (lucky) {
        const int g = _nextGroup++;
        for (int i = 0; i < 4; ++i) {
            const float arc = std::sin(3.14159265f * (i + 0.5f) / 4.f) * 0.30f * ctx.apex;
            if (Actor* c = spawn(Kind::BonusCoin, "coin_gold", 1, m.x - 60.f + i * 40.f, gy + 22.f + arc, 34.f * k, false)) {
                c->w = 34.f * k; c->h = 34.f * k; c->group = g;
            }
        }
        ++_passed;                                        // nothing to dodge: counts as passed for the objective
        reward(4, Vec2(m.x, gy + 90.f), "LUCKY METEOR! +4");
    } else if (Actor* r = spawn(Kind::Rock, "rock", 1, m.x, gy, std::min(0.30f * ctx.apex, 50.f * k), false)) {
        r->hazard = true;
        r->w = r->spr->getContentSize().width * r->spr->getScale() * 0.62f;
        r->group = _nextGroup++;
        ++_total;
    }
    PZUI::burst(_scene, Vec2(m.x, gy + 10.f), Color3B(255, 170, 60), 14, 90.f, 0.5f, 900);
    PZAudio::play("fx_explosion", 0.5f, 0.1f);
    if (_host.shake) _host.shake(4.f, 0.2f);
    // a dodge bonus when the rider was close to the impact and did not get hit
    const bool close = !_hitThisEvent && std::abs(m.x - ctx.rider.x) < 150.f;
    const float mx = m.x;
    removeActor(m);
    if (close && _dodgesEv < 3) {
        ++_dodgesEv;
        ++_rs.closeDodges;
        reward(cfg().rewardCloseDodge, Vec2(mx, gy + 70.f), "CLOSE DODGE +" + std::to_string(cfg().rewardCloseDodge));
        ++_score;
    }
}

// Variant 0 "shower": four single strikes.  Variant 1 "twin strike" (standard tier+): two rounds of two meteors 230 units
// apart with an EDGE RUN coin trail on the road between the rocks (optional: land in the gap).
void RoadEventDirector::updateMeteor(const Context& ctx) {
    const float winH = Director::getInstance()->getWinSize().height;
    const float k = kDeviceScale();
    const bool twin = _variant >= 1;
    const int rounds = _variant == 2 ? 3 : (twin ? 2 : 4);        // variant 2 BARRAGE (expert): three twin strikes
    if (_step == 0) {                                          // a line from the rider first, then the sky opens up
        if (_t <= ctx.dt * 1.5f + 0.001f) { say("meteor", 0); _target = _variant == 2 ? 6 : 4; }
        if (_t > 1.5f) { _step = 1; _wait = 0.f; }
        return;
    }
    setObjective(std::string(_variant == 2 ? "BARRAGE  " : (twin ? "TWIN STRIKE  " : "METEOR SHOWER  ")) + std::to_string(std::min(_passed, _target)) + "/" + std::to_string(_target) +
                 (_riskNeed > 0 ? "\n" + _riskLabel + "  " + std::to_string(_riskGot) + "/" + std::to_string(_riskNeed) : std::string()));
    // _step 1..N: one strike (or pair) every ~1.7 s, each one telegraphed by a growing shadow
    if (_step <= rounds) {
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        const float dur = std::max(1.35f, cfg().telegraphFloor + 0.05f);
        const float X = ctx.rider.x + _speed * (dur + 0.85f) + _rng.range(40.f, 200.f);   // lands well ahead: >= 0.85 s to react to the rock
        const float span = twin ? 230.f : 0.f;
        if (!roadClear(X - 70.f, X + span + 70.f, ctx.apex)) {
            if (_waited > 4.f) { ++_step; _waited = 0.f; _wait = 0.5f; _target -= twin ? 2 : 1; }
            return;
        }
        for (int m = 0; m < (twin ? 2 : 1); ++m) {
            const float mx = X + m * span;
            float gy = 0.f;
            ground(mx, gy);
            if (Actor* sh = spawn(Kind::Shadow, "shadow", 1, mx, gy, 16.f * k, false)) {
                sh->fallDur = dur;
                sh->spr->setScale(0.3f);
                sh->spr->setOpacity(150);
            }
            if (Actor* mt = spawn(Kind::Meteor, "meteor", 4, mx, winH + 130.f, 96.f * k, false)) {
                mt->fallDur = dur;
                mt->targetY = gy;
                mt->fps = 14.f;
            }
        }
        if (twin) {
            float gy = 0.f;
            ground(X + span * 0.5f, gy);
            spawnRiskLine(X + span * 0.5f - 40.f, gy, ctx.apex, 3, 0.10f, 40.f, _nextGroup++);
            _riskReward = cfg().rewardEdgeRun;
            _riskLabel = "EDGE RUN";
        }
        PZAudio::play("fx_tick", 0.4f, 0.1f);
        ++_step;
        _waited = 0.f;
        _wait = twin ? _rng.range(2.4f, 2.8f) : _rng.range(1.6f, 2.0f);
        return;
    }
    for (auto& a : _actors) {
        if (a->kind == Kind::Meteor || a->kind == Kind::Shadow || a->kind == Kind::Rock || a->kind == Kind::BonusCoin) return;
    }
    const bool clean = !_hitThisEvent;
    settleRisk(ctx);
    if (clean) reward(cfg().rewardMeteor, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "SKY CLEARED! +" + std::to_string(cfg().rewardMeteor));
    finishEvent(clean);
}

// ------------------------------------------------------------------------------------------------ Grandma Turbo

void RoadEventDirector::updateGrandma(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float k = kDeviceScale();
    Actor* g = find(_rivalId);
    if (_step == 0) {
        float gy = 0.f;
        if (!ground(ctx.rider.x, gy)) {
            _waited += ctx.dt;
            if (_waited > 8.f) finishEvent(Outcome::SkippedNoRoad);
            return;
        }
        g = spawn(Kind::Rider, "rider_grandma", 4, ctx.rider.x - 260.f, gy, riderVisualHeight(ctx), true);
        if (!g) { finishEvent(Outcome::SkippedNoRoad); return; }
        g->fps = 8.f;                                     // slow wheels: she is taking her time
        g->laneLift = 10.f;
        _rivalId = g->id;
        _lead = -260.f;
        _raceT = 0.f;
        _wait = 0.f;
        _step = 1;
        say("grandma", g->id);
        return;
    }
    if (_step == 3) {                                     // she is gone; the coin trail stays collectable for a moment
        _wait -= ctx.dt;
        if (_wait <= 0.f) {
            settleRisk(ctx);
            const bool good = _score >= 4 || _drafted;
            if (good) reward(cfg().rewardGrandma, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "BASKET BONUS +" + std::to_string(cfg().rewardGrandma));
            finishEvent(good);
        }
        return;
    }
    if (!g) { finishEvent(Outcome::Failed); return; }
    if (_variant == 1) setObjective("SLIPSTREAM  " + std::to_string(std::min(100, static_cast<int>(_draft / 1.2f * 100.f))) + "%  -  " + std::to_string(_score) + "/4 COINS" + (_riskNeed > 0 ? "\nJACKPOT  " + std::to_string(_riskGot) + "/" + std::to_string(_riskNeed) : std::string()));
    else setObjective("GRANDMA TURBO  " + std::to_string(_score) + "/4 COINS" + (_riskNeed > 0 ? "\nJACKPOT  " + std::to_string(_riskGot) + "/" + std::to_string(_riskNeed) : std::string()));
    _raceT += ctx.dt;
    if (_step == 1) {                                     // slow entrance
        _lead = approach(_lead, -130.f, 46.f * ctx.dt);
        if (_raceT > 2.4f) {
            _step = 2;
            _raceT = 0.f;
            _wait = 0.f;
            if (!g->extra) {
                g->extra = Sprite::createWithSpriteFrameName(Content::shared().frame("flame", 0));
                if (g->extra) {
                    g->extra->setAnchorPoint(Vec2(1.f, 0.5f));
                    g->extra->setPosition(Vec2(-26.f * k, 26.f * k));
                    g->extra->setScale(0.9f * k);
                    g->root->addChild(g->extra, -1);
                }
            }
            g->fps = 22.f;
            bubble(Content::shared().line("grandma_route"), g->id, 1.6f);   // the choice: the safe low trail or the risky high JACKPOT coins
            PZAudio::play("fx_rocket", 0.5f, 0.2f);
            if (_host.shake) _host.shake(3.f, 0.5f);
        }
    } else if (_step == 2) {                              // absurd rocket overtake, dropping coins from the basket
        _lead += (160.f + (_variant == 1 ? 380.f : 900.f) * _raceT) * ctx.dt;   // variant 1: she lingers ahead, so you can draft
        if (_variant == 1 && _lead > 20.f && _lead < 400.f && !ctx.airborne) {
            _draft += ctx.dt;                                  // riding in her slipstream on the ground charges the meter
            if (_draft >= 1.2f && !_drafted) {
                _drafted = true;
                reward(cfg().rewardGrandma, Vec2(ctx.rider.x + 30.f, ctx.rider.y + 90.f), "SLIPSTREAM! +" + std::to_string(cfg().rewardGrandma));
                notifyBoost(0.f);
                PZAudio::play("fx_turbo", 0.5f, 0.1f);
            }
        }
        _wait -= ctx.dt;
        if (_wait <= 0.f && _lead > -80.f && _lead < winW - 200.f && _total < 9) {
            float gy = 0.f;
            if (ground(g->x - 40.f, gy)) {
                // every third coin is a JACKPOT coin hanging high (risky: needs a real jump); the rest ride the safe low trail
                const bool jackpot = (_total % 3) == 2;
                const Kind ck = jackpot ? Kind::BonusCoin : Kind::Coin;
                if (Actor* c = spawn(ck, "coin_gold", 1, g->x - 40.f,
                                     gy + (jackpot ? 0.82f * ctx.apex : 52.f * k + 34.f * std::sin(_total * 0.9f)), 28.f * k, false)) {
                    c->w = 34.f * k;
                    c->h = 34.f * k;
                    c->group = _nextGroup;
                    if (jackpot) { ++_riskNeed; _riskReward = cfg().rewardEdgeRun; _riskLabel = "JACKPOT"; }
                    ++_total;
                }
            }
            _wait = 0.26f;
        }
        if (_lead > winW + 220.f) {
            removeActor(*g);
            _rivalId = 0;
            _step = 3;
            _wait = 3.0f;                                 // the trail is still collectable for a moment
            return;                                       // the actor (and its root) is gone
        }
    } else {
        _wait -= ctx.dt;
        if (_wait <= 0.f) {
            settleRisk(ctx);
            const bool good = _score >= 4 || _drafted;
            if (good) reward(cfg().rewardGrandma, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "BASKET BONUS +" + std::to_string(cfg().rewardGrandma));
            finishEvent(good);
        }
        return;
    }
    g->x = ctx.rider.x + _lead;
    g->root->setRotation(_step == 2 ? -9.f : 0.f);        // nose up under rocket power
}

// ------------------------------------------------------------------------------------------------ T-Rex chase (chapter climax)

// A 30 second set piece in three recognisable phases (scheduled after several cleared events, never drawn from the bag):
//   1 PURSUIT      four telegraphed boulders shaken loose by his stomps;
//   2 LOG KICKS    he kicks three fast logs along the road (read the rumble, jump late);
//   3 TWIN STOMPS  two double strikes with an EDGE RUN coin trail between the boulders.
// The payoff line is held for ~2 s after the last hazard so it can actually be read.
void RoadEventDirector::updateTrex(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float winH = Director::getInstance()->getWinSize().height;
    const float k = kDeviceScale();
    Actor* t = find(_rivalId);
    if (_step == 0) {
        float gy = 0.f;
        if (!ground(ctx.rider.x, gy)) {
            _waited += ctx.dt;
            if (_waited > 8.f) finishEvent(Outcome::SkippedNoRoad);
            return;
        }
        t = spawn(Kind::Ambient, "dino_big", 4, 125.f, gy, std::min(0.9f * ctx.apex, 150.f * k), false);   // fully on screen, clear of the camera cut-out
        if (!t) { finishEvent(Outcome::SkippedNoRoad); return; }
        t->fps = 7.f;
        t->w = 0.f;
        t->vx = 0.f;
        t->hazard = false;
        _rivalId = t->id;
        _step = 1;
        _target = 12;
        _wait = 2.4f;
        say("trex", t->id);
        if (_host.shake) _host.shake(8.f, 0.6f);
        PZAudio::play("fx_explosion", 0.6f, 0.1f);
        return;
    }
    if (!t) { finishEvent(Outcome::Failed); return; }
    const int phase = _step <= 4 ? 1 : (_step <= 7 ? 2 : (_step <= 8 || _step == 81 ? 3 : 4));
    if (_step == 81) {                                          // BITE: a telegraphed lunge; be in the air when his jaws close
        t->root->setRotation(std::sin(_t * 6.f) * 1.5f);
        setObjective("T-REX 3/4  BITE - JUMP!\n" + std::to_string(std::min(_passed, _target)) + "/" + std::to_string(_target));
        _biteT -= ctx.dt;
        if (_biteT > 0.40f) {                                   // crouch and shake: the readable wind-up
            t->x = 125.f + std::sin(_t * 40.f) * 5.f;
            t->root->setRotation(-5.f);
        } else if (_biteT > 0.f) {                              // the lunge across the screen
            const float p = 1.f - _biteT / 0.40f;
            t->x = 125.f + 240.f * std::sin(p * 3.14159f * 0.5f);
            if (!_biteHit && _biteT < 0.30f && !ctx.airborne && ctx.rider.x - t->x < 90.f) {   // jaws closed on a rider still on the ground
                _biteHit = true;
                _hitThisEvent = true;
                const bool absorbed = _host.absorbHit ? _host.absorbHit(Vec2(t->x + 60.f, t->y + 40.f)) : false;
                if (!absorbed && _host.lethal) { Analytics::shared().log("road_event_hit", {{"event", "trex_chase"}, {"kind", "bite"}}); _host.lethal(); }
            }
        } else {
            t->x = 125.f;
            if (!_biteHit) {
                ++_passed;
                reward(5, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 80.f), "DODGED THE BITE! +5");
            }
            _step = 9;
            _wait = 1.3f;
        }
        return;
    }
    if (_step <= 10) {
        t->x = 125.f + std::sin(_t * 2.f) * 14.f;                // lumbering at the left edge
        t->root->setRotation(std::sin(_t * 6.f) * 1.5f);
        static const char* kPhase[4] = {"PURSUIT", "LOG KICKS", "BITE", "TWIN STOMPS"};
        setObjective("T-REX " + std::to_string(phase) + "/4  " + kPhase[phase - 1] + "\n" + std::to_string(std::min(_passed, _target)) + "/" + std::to_string(_target) +
                     (_riskNeed > 0 ? "  -  " + _riskLabel + " " + std::to_string(_riskGot) + "/" + std::to_string(_riskNeed) : std::string()));
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        const float dur = std::max(1.35f, cfg().telegraphFloor + 0.05f);
        const float spawnX = winW + 110.f;
        if (phase == 2) {                                       // a log kicked along the road
            if (!roadClear(spawnX - 70.f, spawnX + 150.f, ctx.apex)) {
                if (_waited > 3.f) { ++_step; _waited = 0.f; _wait = 0.4f; --_target; }
                return;
            }
            float gy = 0.f;
            ground(spawnX, gy);
            if (Actor* b = spawn(Kind::Barrel, "barrel", 1, spawnX, gy, std::min(0.34f * ctx.apex, 54.f * k), false)) {
                b->hazard = true;
                b->vx = -200.f;                                 // fast, but still >= the telegraph floor from the screen edge
                b->w = b->h * 0.8f;
                b->group = _nextGroup++;
                ++_total;
            }
            bubble(Content::shared().line("trex_kick"), t->id, 0.9f);
            if (_host.shake) _host.shake(5.f, 0.25f);
            PZAudio::play("fx_tick", 0.5f, 0.1f);
            ++_step;
            _waited = 0.f;
            _wait = 1.9f + _rng.range(0.f, 0.3f);
            return;
        }
        if (phase == 3) {                                       // start the bite wind-up (resolved in step 81)
            bubble(Content::shared().line("trex_bite"), t->id, 1.2f);
            if (_host.shake) _host.shake(4.f, 0.4f);
            PZAudio::play("fx_tick", 0.6f, 0.1f);
            _biteT = 1.5f;
            _biteHit = false;
            _step = 81;
            return;
        }
        const bool twin = phase == 4;
        const float X = ctx.rider.x + _speed * (dur + 0.85f) + _rng.range(40.f, 200.f);
        const float span = twin ? 230.f : 0.f;
        if (!roadClear(X - 70.f, X + span + 70.f, ctx.apex)) {
            if (_waited > 3.f) { ++_step; _waited = 0.f; _wait = 0.4f; _target -= twin ? 2 : 1; }
            return;
        }
        for (int m = 0; m < (twin ? 2 : 1); ++m) {
            const float mx = X + m * span;
            float gy = 0.f;
            ground(mx, gy);
            if (Actor* sh = spawn(Kind::Shadow, "shadow", 1, mx, gy, 16.f * k, false)) {
                sh->fallDur = dur;
                sh->spr->setScale(0.3f);
                sh->spr->setOpacity(150);
            }
            if (Actor* mt = spawn(Kind::Meteor, "meteor", 4, mx, winH + 130.f, 96.f * k, false)) {
                mt->fallDur = dur;
                mt->targetY = gy;
                mt->fps = 14.f;
            }
        }
        if (twin) {
            float gy = 0.f;
            ground(X + span * 0.5f, gy);
            spawnRiskLine(X + span * 0.5f - 40.f, gy, ctx.apex, 3, 0.10f, 40.f, _nextGroup++);
            _riskReward = cfg().rewardEdgeRun;
            _riskLabel = "EDGE RUN";
        }
        if (_host.shake) _host.shake(4.f, 0.25f);               // the stomp that shook it loose
        PZAudio::play("fx_tick", 0.4f, 0.1f);
        if (_step == 1 || _step == 5 || _step == 8) say("trex", t->id);
        ++_step;
        _waited = 0.f;
        _wait = twin ? 2.5f + _rng.range(0.f, 0.3f) : (_tier == Tier::Expert ? 1.65f : 1.9f) + _rng.range(0.f, 0.3f);
        return;
    }
    if (_step == 11) {                                          // wait for the last hazards to clear
        for (auto& a : _actors) {
            if (a->kind == Kind::Meteor || a->kind == Kind::Shadow || a->kind == Kind::Rock || a->kind == Kind::Barrel || a->kind == Kind::BonusCoin) return;
        }
        const bool clean = !_hitThisEvent;
        settleRisk(ctx);
        bubble(Content::shared().line(clean ? "trex_win" : "trex_loss"), t->id, 2.0f);
        if (clean) {
            reward(cfg().rewardClimax, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 80.f), "T-REX ESCAPED! +" + std::to_string(cfg().rewardClimax));
            PZAudio::play("fx_stage", 0.7f, 0.2f);
            PZHaptics::play(PZHaptics::Kind::Success);
        }
        _score = clean ? 1 : 0;
        _step = 12;
        _wait = 2.1f;                                           // keep the T-Rex (and so his line) alive long enough to read it
        return;
    }
    t->root->setRotation(std::sin(_t * 3.f) * 2.f);
    _wait -= ctx.dt;
    if (_wait <= 0.f) {
        removeActor(*t);
        _rivalId = 0;
        finishEvent(_score == 1);
    }
}

// ------------------------------------------------------------------------------------------------ ambient traffic

void RoadEventDirector::updateAmbient(const Context& ctx) {
    if (ctx.controlLocked || ctx.ridesActive) return;
    if (_active && _cur == EventId::CoolKidRace) return;
    for (auto& a : _actors) {
        if (a->kind == Kind::Ambient) return;
    }
    _ambientCool -= ctx.dt;
    if (_ambientCool > 0.f || _clock < 12.f) return;
    float gy = 0.f;
    if (!ground(ctx.rider.x, gy)) return;
    const bool a = _rng.chance(0.5f);
    if (Actor* p = spawn(Kind::Ambient, a ? "rider_ambient_a" : "rider_ambient_b", 4, -190.f, gy, riderVisualHeight(ctx) * 0.94f, true)) {
        p->fps = 14.f;
        p->laneLift = 18.f;                               // rides the back lane
        p->vx = 330.f;                                    // faster than the road: he passes you
        p->w = 0.f;
        p->hazard = false;
        // setup / payoff gags (non-blocking): 1 wobble, 2 loses a cone, 3 startled by a jump, 4 returns later with the cone fixed
        if (_gagState == 1) { p->group = 4; _gagState = 0; }
        else {
            const int r = _rng.index(10);
            p->group = r < 4 ? 0 : (r < 6 ? 1 : (r < 8 ? 2 : 3));
            if (p->group == 0 && _rng.chance(0.35f)) bubble(Content::shared().line("gag_beep"), p->id, 1.4f);
        }
    }
    _ambientCool = _rng.range(20.f, 35.f);
}

// ------------------------------------------------------------------------------------------------ per-frame actor motion

void RoadEventDirector::updateActors(const Context& ctx) {
    const float dt = ctx.dt;
    _dodgeCool = std::max(0.f, _dodgeCool - dt);
    _chainT = std::max(0.f, _chainT - dt);
    if (_chainT <= 0.f) _chain = 0;
    for (size_t ai = 0; ai < _actors.size(); ++ai) {      // index loop: impact() / drops may spawn new actors mid-loop
        Actor& a = *_actors[ai];
        if (!a.root) continue;
        const bool screenLocked = a.kind == Kind::Ambient || a.kind == Kind::Rider;
        if (a.launched) {                                 // comic knock-away
            a.life -= dt;
            a.x += a.vx * dt;
            a.y += a.vy * dt;
            a.vy -= kGravity * dt;
            a.root->setRotation(a.root->getRotation() + a.spin * dt);
            a.root->setOpacity(static_cast<uint8_t>(255.f * clampf(a.life / 0.5f, 0.f, 1.f)));
            if (a.life <= 0.f) removeActor(a);
            else a.root->setPosition(Vec2(a.x, a.y));
            continue;
        }
        if (!screenLocked) a.x -= ctx.scroll;
        switch (a.kind) {
        case Kind::DinoSmall:
        case Kind::DinoBig:
            if (!a.stopped) {
                a.x += a.vx * dt;
                float ahead = 0.f;
                if (!ground(a.x - 44.f, ahead) || obstacleNear(a.x - 70.f, 40.f)) a.stopped = true;   // never walks off the road / into rocks
            }
            if (ctx.dt > 0.f) { float gy = 0.f; if (ground(a.x, gy)) a.y = gy; }
            animate(a, a.stopped ? 0.f : dt);
            break;
        case Kind::Rider:
            followRoad(a, dt, true);
            animate(a, dt);
            break;
        case Kind::Ambient:
            if (a.vx > 0.f && !a.counted && ctx.dt > 0.f) {                       // gags (the T-Rex is an Ambient actor with vx = 0)
                const float winW2 = Director::getInstance()->getWinSize().width;
                if (a.group == 1 && a.x > winW2 * 0.3f) { a.counted = true; bubble(Content::shared().line("gag_wobble"), a.id, 1.3f); a.spin = 1.f; }
                else if (a.group == 2 && a.x > winW2 * 0.45f) {
                    a.counted = true;
                    bubble(Content::shared().line("gag_cone_lost"), a.id, 1.5f);
                    _gagState = 1;
                    if (Actor* c = spawn(Kind::Cone, "cone", 1, a.x - 30.f, a.y + 20.f, 40.f * kDeviceScale(), false)) c->hazard = false;   // harmless prop
                } else if (a.group == 3 && ctx.airborne && std::abs(a.x - ctx.rider.x) < 280.f) { a.counted = true; bubble(Content::shared().line("gag_startled"), a.id, 1.4f); a.spin = 1.f; }
                else if (a.group == 4 && a.x > winW2 * 0.3f) { a.counted = true; bubble(Content::shared().line("gag_cone_fixed"), a.id, 1.6f); }
            }
            if (a.spin > 0.f && a.root) { a.spin += dt; a.root->setRotation(std::sin(a.spin * 18.f) * 9.f * std::max(0.f, 1.f - (a.spin - 1.f) / 0.8f)); }
            a.x += a.vx * dt;
            if (a.x > Director::getInstance()->getWinSize().width + 220.f) { removeActor(a); continue; }
            followRoad(a, dt, true);
            animate(a, dt);
            break;
        case Kind::Box: {
            if (!a.grounded) {
                a.vy -= kGravity * 0.8f * dt;
                a.y += a.vy * dt;
                a.root->setRotation(a.root->getRotation() + a.spin * dt);
                float gy = 0.f;
                if (ground(a.x, gy) && a.y <= gy + 2.f && a.vy < 0.f) {
                    a.y = gy + 2.f;
                    if (a.vy < -240.f) a.vy = -a.vy * 0.32f;       // one small bounce
                    else { a.grounded = true; a.vy = 0.f; a.root->setRotation(0.f); }
                } else if (a.y < -80.f) {
                    removeActor(a);
                    continue;
                }
            } else {
                float gy = 0.f;
                if (ground(a.x, gy)) a.y = gy + 2.f;
            }
            break;
        }
        case Kind::Sign:
        case Kind::Cone:
        case Kind::Rock: {
            float gy = 0.f;
            if (ground(a.x, gy)) a.y = gy;
            break;
        }
        case Kind::Barrel: {
            a.x += a.vx * dt;
            float gy = 0.f;
            if (!ground(a.x, gy)) { removeActor(a); continue; }       // rolls off the end of the road
            a.y = gy;
            a.root->setRotation(a.root->getRotation() + (a.vx * dt - ctx.scroll) / std::max(10.f, a.h * 0.5f) * 57.3f);
            break;
        }
        case Kind::Worker: {
            float gy = 0.f;
            if (ground(a.x, gy)) a.y = gy + a.laneLift;
            animate(a, dt);
            if (a.lamp) {                                     // flagger: flicker while deciding, lock >= telegraphFloor before arrival
                a.fallT += dt;
                const bool locked = (a.x - ctx.rider.x) < _speed * (cfg().telegraphFloor + 0.3f);
                const bool stop = locked ? a.fallDur > 0.5f : std::fmod(a.fallT, 0.9f) < 0.45f;
                if (static_cast<int>(a.vy) != (stop ? 1 : 0)) { a.vy = stop ? 1.f : 0.f; drawLamp(a.lamp, stop); }
                a.hazard = locked && stop;
            }
            break;
        }
        case Kind::BonusCoin:
        case Kind::Coin: {
            a.spr->setScaleX(std::max(0.2f, std::abs(std::cos(_clock * 5.f + a.x * 0.02f))) * a.spr->getScaleY());
            break;
        }
        case Kind::Shadow: {
            a.fallT += dt;
            const float p = clampf(a.fallT / std::max(0.1f, a.fallDur), 0.f, 1.f);
            a.spr->setScale(0.3f + 1.5f * p);
            a.spr->setOpacity(static_cast<uint8_t>(110.f + 120.f * p));
            float gy = 0.f;
            if (ground(a.x, gy)) a.y = gy - 2.f;
            if (p >= 1.f) { removeActor(a); continue; }
            break;
        }
        case Kind::Meteor: {
            a.fallT += dt;
            const float p = clampf(a.fallT / std::max(0.1f, a.fallDur), 0.f, 1.f);
            float gy = a.targetY;
            ground(a.x, gy);
            const float top = Director::getInstance()->getWinSize().height + 130.f;
            a.y = top + (gy - top) * p * p;                    // accelerates as it falls
            animate(a, dt);
            if (p >= 1.f) { impact(a, ctx); continue; }
            break;
        }
        }
        if ((a.hazard || a.kind == Kind::Worker) && !a.counted && ctx.dt > 0.f && a.x + a.w * 0.5f < ctx.rider.x - 4.f) {
            a.counted = true;                                 // it is behind the rider now: progress, and maybe a close-dodge bonus
            if (a.group == 0 || a.group != _lastCountedGroup) { ++_passed; _lastCountedGroup = a.group; }
            const float clearance = ctx.riderBox.getMinY() - (a.y + a.h);
            if (a.hazard && !_hitThisEvent && ctx.airborne && clearance > -2.f && clearance < 38.f && _dodgeCool <= 0.f && _dodgesEv < 3) {
                _dodgeCool = 1.2f;
                ++_dodgesEv;
                ++_rs.closeDodges;
                _chain = _chainT > 0.f ? _chain + 1 : 1;                 // near-miss chain: each dodge within 6 s extends it
                _chainT = 6.f;
                _rs.bestChain = std::max(_rs.bestChain, _chain);
                if (_chain >= 2) {
                    reward(cfg().rewardCloseDodge + _chain, Vec2(ctx.rider.x + 10.f, ctx.rider.y + 60.f), "CHAIN x" + std::to_string(_chain) + "! +" + std::to_string(cfg().rewardCloseDodge + _chain));
                    PZAudio::play("fx_levelup", 0.3f, 0.1f);
                } else {
                    reward(cfg().rewardCloseDodge, Vec2(ctx.rider.x + 10.f, ctx.rider.y + 60.f), "CLOSE! +" + std::to_string(cfg().rewardCloseDodge));
                }
            }
        }
        if (a.x < -230.f && !screenLocked) { removeActor(a); continue; }
        if (a.root) {
            a.root->setPosition(Vec2(a.x, a.y));
        }
    }
    _actors.erase(std::remove_if(_actors.begin(), _actors.end(),
                                 [](const std::shared_ptr<Actor>& a) { return a->life < 0.f && !a->root; }),
                  _actors.end());
}

void RoadEventDirector::updateBubbles(float dt, const Context& ctx) {
    for (size_t i = 0; i < _bubbles.size();) {
        Bubble& b = _bubbles[i];
        b.t += dt;
        Vec2 at = ctx.rider + Vec2(0.f, 96.f);
        bool alive = true;
        if (b.actorId != 0) {
            Actor* a = find(b.actorId);
            if (a && a->root) at = Vec2(a->x, a->y + a->h + 24.f);
            else alive = false;
        }
        if (!alive || b.t >= b.life) {
            if (b.node) {
                b.node->runAction(Sequence::create(FadeOut::create(0.15f), RemoveSelf::create(), nullptr));
            }
            _bubbles.erase(_bubbles.begin() + i);
            continue;
        }
        const float winW = Director::getInstance()->getWinSize().width;
        at.x = clampf(at.x, 110.f, winW - 110.f);
        b.node->setPosition(at);
        ++i;
    }
}

// ------------------------------------------------------------------------------------------------ collisions

void RoadEventDirector::hitTest(const Context& ctx) {
    // rider box, slightly forgiving
    Rect rb = ctx.riderBox;
    rb.origin.x += rb.size.width * 0.18f;
    rb.size.width *= 0.64f;
    rb.origin.y += rb.size.height * 0.10f;
    rb.size.height *= 0.78f;
    for (auto& ap : _actors) {
        Actor& a = *ap;
        if (!a.root || a.launched) continue;
        if ((a.kind == Kind::Box || a.kind == Kind::Coin || a.kind == Kind::BonusCoin) && !a.collected) {
            Rect bb(a.x - a.w * 0.5f - 8.f, a.y - 4.f, a.w + 16.f, a.h + 12.f);
            if (bb.intersectsRect(rb)) {
                a.collected = true;
                ++_rs.objectsCollected;
                if (_micro.active && _micro.type == 1) ++_micro.progress;
                if (a.kind == Kind::BonusCoin) { ++_riskGot; reward(2, Vec2(a.x, a.y + 30.f), "+2"); }
                else if (a.kind == Kind::Coin) { ++_score; reward(4, Vec2(a.x, a.y + 30.f), "+4"); }
                else {
                    ++_score; ++_rs.boxesCaught;
                    if (a.group == 9) reward(cfg().rewardPizzaBox * 3, Vec2(a.x, a.y + 40.f), "GOLDEN SLICE +" + std::to_string(cfg().rewardPizzaBox * 3));
                    else reward(cfg().rewardPizzaBox, Vec2(a.x, a.y + 40.f), "PIZZA +" + std::to_string(cfg().rewardPizzaBox));
                }
                PZAudio::play("fx_powerup", 0.5f, 0.05f);
                PZUI::burst(_scene, Vec2(a.x, a.y + 14.f), Color3B(255, 210, 90), 8, 46.f, 0.4f, 900);
                removeActor(a);
            }
            continue;
        }
        if (!a.hazard) continue;
        Rect hb(a.x - a.w * 0.5f, a.y + 3.f, a.w, a.h * 0.9f);
        if (!hb.intersectsRect(rb)) continue;
        _hitThisEvent = true;
        const bool absorbed = _host.absorbHit ? _host.absorbHit(Vec2(a.x, a.y + a.h * 0.5f)) : false;
        if (absorbed) {
            launch(a, ctx.rider.x);
            if (_host.shake) _host.shake(6.f, 0.2f);
            PZUI::burst(_scene, Vec2(a.x, a.y + a.h * 0.5f), Color3B(255, 220, 120), 10, 60.f, 0.4f, 900);
            if (_rng.chance(0.5f)) say("hit_absorbed", 0);
        } else if (_host.lethal) {
            Analytics::shared().log("road_event_hit", {{"event", eventName(_cur)}, {"at", std::to_string(static_cast<int>(_t))}, {"kind", std::to_string(static_cast<int>(a.kind))}, {"x", std::to_string(static_cast<int>(a.x))}, {"rider", std::to_string(static_cast<int>(ctx.rider.x))}, {"y", std::to_string(static_cast<int>(a.y))}, {"ry", std::to_string(static_cast<int>(ctx.rider.y))}});
            _host.lethal();
            return;
        }
    }
}

// ------------------------------------------------------------------------------------------------ rewards, risk lines, objective HUD, micro challenges

float RoadEventDirector::telegraphLead(float base) const { return std::max(base, cfg().telegraphFloor); }

void RoadEventDirector::reward(int coins, const Vec2& at, const std::string& text) {
    if (coins <= 0) return;
    _evCoins += coins;
    _rs.eventCoins += coins;                              // the daily score does not count these twice
    if (_host.award) _host.award(coins, at, text);
}

void RoadEventDirector::spawnRiskLine(float x, float gy, float apex, int count, float heightFrac, float spacing, int group) {
    const float k = kDeviceScale();
    for (int i = 0; i < count; ++i) {
        const float arc = std::sin(3.14159265f * (i + 0.5f) / count) * 0.12f * apex;
        if (Actor* c = spawn(Kind::BonusCoin, "coin_gold", 1, x + i * spacing, gy + heightFrac * apex + arc, 34.f * k, false)) {
            c->w = 34.f * k;
            c->h = 34.f * k;
            c->group = group;
            ++_riskNeed;
        }
    }
}

int RoadEventDirector::countGroup(int group) const {
    int n = 0;
    for (const auto& a : _actors) if (a->root && a->group == group && a->kind == Kind::BonusCoin && !a->collected) ++n;
    return n;
}

void RoadEventDirector::settleRisk(const Context& ctx) {
    if (_riskNeed > 0 && _riskGot >= _riskNeed && _riskReward > 0) {
        reward(_riskReward, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 100.f), _riskLabel + "! +" + std::to_string(_riskReward));
        Analytics::shared().log("risk_line", {{"event", eventName(_cur)}, {"label", _riskLabel}, {"got", "1"}});
    } else if (_riskNeed > 0) {
        Analytics::shared().log("risk_line", {{"event", eventName(_cur)}, {"label", _riskLabel}, {"got", "0"},
                                              {"coins", std::to_string(_riskGot) + "/" + std::to_string(_riskNeed)}});
    }
    _riskGot = _riskNeed = _riskReward = 0;
}

int RoadEventDirector::rivalWins() const { return UserDefault::getInstance()->getIntegerForKey("PZP_V2_rv_cool_w", 0); }
int RoadEventDirector::rivalLosses() const { return UserDefault::getInstance()->getIntegerForKey("PZP_V2_rv_cool_l", 0); }
void RoadEventDirector::noteRivalResult(bool playerWon) {
    const char* key = playerWon ? "PZP_V2_rv_cool_w" : "PZP_V2_rv_cool_l";
    UserDefault::getInstance()->setIntegerForKey(key, UserDefault::getInstance()->getIntegerForKey(key, 0) + 1);
    UserDefault::getInstance()->setIntegerForKey("PZP_V2_rv_cool_last", playerWon ? 1 : -1);   // drives the rematch dialogue
}

void RoadEventDirector::notifySmoothLanding(bool perfect) {
    ++_rs.smooth;
    if (perfect) ++_rs.perfect;
    if (_micro.active && _micro.type == 0) ++_micro.progress;
}

void RoadEventDirector::notifyPowerUp() {
    if (_active) _powerUpInEvent = true;
}

Node* RoadEventDirector::makePlate(float w, const Color3B& edge) {
    auto* n = Node::create();
    auto* bg = PZUI::roundedRect(w, 44.f, 16.f, Color3B(22, 18, 34), 0.82f, edge, 1.f, 2.f);
    bg->setPosition(Vec2(w * 0.5f, 0.f));
    n->addChild(bg);
    n->setCascadeOpacityEnabled(true);
    return n;
}

static void fitPlateLabel(Label* l, float w) {
    l->setScale(1.f);
    const float cw = l->getContentSize().width;
    if (cw > w - 24.f) l->setScale((w - 24.f) / cw);
}

void RoadEventDirector::setObjective(const std::string& text) {
    if (!_hud) return;
    if (_objResultT > 0.f) return;                        // a result flash owns the plate
    if (!_objPlate) {
        _objPlate = makePlate(222.f, Color3B(255, 205, 80));
        _objLabel = PZUI::label("", 9.f, Color3B(255, 250, 235));
        _objLabel->setAnchorPoint(Vec2(0.f, 0.5f));
        _objLabel->setPosition(Vec2(14.f, 0.f));
        _objLabel->setAlignment(TextHAlignment::LEFT);
        _objPlate->addChild(_objLabel, 2);
        _hud->addChild(_objPlate, 19);
        _objPlate->setScale(0.01f);
        _objPlate->runAction(EaseBackOut::create(ScaleTo::create(0.25f, PZLayout::uiScale())));
        layoutHud();
        _objText.clear();
    }
    if (text == _objText) return;
    _objText = text;
    _objLabel->setString(text);
    fitPlateLabel(_objLabel, 222.f);
}

void RoadEventDirector::flashResult(const std::string& text, const Color3B& colour) {
    if (!_hud) return;
    _objResultT = 0.f;
    if (!_objPlate) setObjective(text);
    _objText = text;
    _objLabel->setString(text);
    _objLabel->setTextColor(Color4B(colour));
    fitPlateLabel(_objLabel, 222.f);
    _objResultT = 1.5f;
}

void RoadEventDirector::hideObjective() {
    _objResultT = 0.f;
    _objText.clear();
    if (_objPlate) {
        _objPlate->stopAllActions();
        _objPlate->runAction(Sequence::create(ScaleTo::create(0.15f, 0.01f), RemoveSelf::create(), nullptr));
        _objPlate = nullptr;
        _objLabel = nullptr;
    }
}

void RoadEventDirector::updateObjectiveHud(float dt) {
    if (_objResultT > 0.f) {
        _objResultT -= dt;
        if (_objResultT <= 0.f) hideObjective();
    } else if (_objPlate && !_active) {
        hideObjective();
    }
}

// Micro challenges: a small side goal between events. Three kinds, cycled in order (deterministic): 3 smooth landings,
// collect 5 event objects, clear the next event without a power-up. Finishing pays a small bonus; failing costs nothing.
void RoadEventDirector::updateMicro(const Context& ctx) {
    auto showPlate = [this](const std::string& text) {
        if (!_hud) return;
        if (!_microPlate) {
            _microPlate = makePlate(222.f, Color3B(120, 220, 255));
            _microLabel = PZUI::label("", 9.f, Color3B(220, 245, 255));
            _microLabel->setAnchorPoint(Vec2(0.f, 0.5f));
            _microLabel->setPosition(Vec2(14.f, 0.f));
            _microPlate->addChild(_microLabel, 2);
            _hud->addChild(_microPlate, 19);
            _microPlate->setScale(0.01f);
            _microPlate->runAction(EaseBackOut::create(ScaleTo::create(0.25f, PZLayout::uiScale())));
            layoutHud();
        }
        _microLabel->setString(text);
        fitPlateLabel(_microLabel, 222.f);
    };
    auto hidePlate = [this]() {
        if (!_microPlate) return;
        _microPlate->stopAllActions();
        _microPlate->runAction(Sequence::create(ScaleTo::create(0.15f, 0.01f), RemoveSelf::create(), nullptr));
        _microPlate = nullptr;
        _microLabel = nullptr;
    };
    if (ctx.controlLocked || ctx.ridesActive || ctx.dt <= 0.f) return;
    if (!_micro.active) {
        _micro.cooldown -= ctx.dt;
        if (_micro.cooldown > 0.f || _clock < 20.f || _active || _cool < 3.f) return;
        _micro.type = _microNext % 3;
        ++_microNext;
        _micro.active = true;
        _micro.progress = 0;
        _micro.armedNoPowerUp = false;
        _micro.target = _micro.type == 0 ? 3 : (_micro.type == 1 ? 5 : 1);
        _micro.timeLeft = _micro.type == 2 ? 40.f : (_micro.type == 1 ? cfg().microTime + 10.f : cfg().microTime);
        Analytics::shared().log("micro_challenge_start", {{"type", std::to_string(_micro.type)}});
        PZAudio::play("fx_tick", 0.4f, 0.1f);
    }
    _micro.timeLeft -= ctx.dt;
    std::string text;
    if (_micro.type == 0) text = "3 SMOOTH LANDINGS  " + std::to_string(_micro.progress) + "/3\nBONUS +" + std::to_string(cfg().rewardMicro);
    else if (_micro.type == 1) text = "GRAB 5 EVENT ITEMS  " + std::to_string(_micro.progress) + "/5\nBONUS +" + std::to_string(cfg().rewardMicro);
    else text = "NEXT EVENT: NO POWER-UPS\nBONUS +" + std::to_string(cfg().rewardMicro);
    showPlate(text);
    if (_micro.type != 2 && _micro.progress >= _micro.target) {
        ++_rs.microDone;
        reward(cfg().rewardMicro, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 90.f), "CHALLENGE! +" + std::to_string(cfg().rewardMicro));
        Analytics::shared().log("micro_challenge_end", {{"type", std::to_string(_micro.type)}, {"success", "1"}});
        _micro.active = false;
        _micro.cooldown = cfg().microEvery;
        hidePlate();
    } else if (_micro.timeLeft <= 0.f && !(_micro.type == 2 && _micro.armedNoPowerUp)) {
        Analytics::shared().log("micro_challenge_end", {{"type", std::to_string(_micro.type)}, {"success", "0"}, {"why", "timeout"}});
        _micro.active = false;
        _micro.cooldown = cfg().microEvery;
        hidePlate();
    }
    if (!_micro.active && _microPlate) hidePlate();
}

} // namespace V2
