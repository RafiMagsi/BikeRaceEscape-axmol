#include "V2/V2RoadEvents.h"
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
}

const char* eventName(EventId id) {
    switch (id) {
    case EventId::DinoCrossing: return "dino_crossing";
    case EventId::PizzaPanic:   return "pizza_panic";
    case EventId::CoolKidRace:  return "cool_kid_race";
    case EventId::ConstructionChaos: return "construction_chaos";
    case EventId::MeteorShower: return "meteor_shower";
    case EventId::GrandmaTurbo: return "grandma_turbo";
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
    if (!SpriteFrameCache::getInstance()->getSpriteFrameByName("sign_dino_0")) {
        SpriteFrameCache::getInstance()->addSpriteFramesWithFile("v2_events.plist");
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
    _barks.resetHistory();
    {
        std::vector<int> all;
        for (int i = 0; i < kEventCount; ++i) all.push_back(i);
        _bag.setItems(all);
    }
    _clock = 0.f;
    _successes = 0;
    _cool = kFirstEventAt * _breatherScale;
    _ambientCool = 14.f + _rng.range(0.f, 8.f);
    _speed = 300.f;
    Analytics::shared().log("v2_run_seed", {{"seed", std::to_string(seed)}});
}

void RoadEventDirector::clear() {
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

bool RoadEventDirector::forceStart(const std::string& name) {
    if (_active) return false;
    for (int i = 0; i < kEventCount; ++i) {
        if (name == eventName(static_cast<EventId>(i))) {
            startEvent(static_cast<EventId>(i));
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
    a->spr = Sprite::createWithSpriteFrameName(base + "_0");
    if (!a->spr) {
        AXLOGW("[V2] missing sprite frame {}_0", base);
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
        a.extra->setSpriteFrame("flame_" + std::to_string(f));
    }
    if (a.frames <= 1 || !a.spr) return;
    a.t += dt;
    const int f = static_cast<int>(a.t * a.fps) % a.frames;
    a.spr->setSpriteFrame(a.base + "_" + std::to_string(f));
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
    a.vx = 420.f + _rng.range(0.f, 160.f);
    a.vy = 620.f + _rng.range(0.f, 160.f);
    a.spin = _rng.range(360.f, 720.f);
    a.life = 1.4f;
    PZAudio::play("fx_smash", 0.8f, 0.08f);
}

void RoadEventDirector::say(const std::string& tag, int actorId) {
    const std::string line = _barks.pick(tag, _rng);
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
    _score = _total = 0;
    _hitThisEvent = false;
    _rivalId = 0;
    _raceState = 0;
    _raceT = 0.f;
    _pendingGain = 0.f;
    _lead = -240.f;
    Analytics::shared().log("road_event_start", {{"event", eventName(id)}, {"t", std::to_string(static_cast<int>(_clock))}});
}

void RoadEventDirector::finishEvent(bool success) {
    Analytics::shared().log("road_event_end", {{"event", eventName(_cur)}, {"success", success ? "1" : "0"},
                                               {"duration", std::to_string(static_cast<int>(_t))}});
    if (success) ++_successes;
    _active = false;
    _cool = _rng.range(kBreatherMin, kBreatherMax) * _breatherScale;
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
    startEvent(static_cast<EventId>(_bag.next(_rng)));
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

    updateScheduler(ctx);
    if (_active) {
        _t += ctx.dt;
        switch (_cur) {
        case EventId::DinoCrossing: updateDino(ctx); break;
        case EventId::PizzaPanic:   updatePizza(ctx); break;
        case EventId::CoolKidRace:  updateRace(ctx); break;
        case EventId::ConstructionChaos: updateConstruction(ctx); break;
        case EventId::MeteorShower: updateMeteor(ctx); break;
        case EventId::GrandmaTurbo: updateGrandma(ctx); break;
        }
    }
    updateAmbient(ctx);
    updateActors(ctx);
    if (!ctx.controlLocked) hitTest(ctx);
    updateBubbles(ctx.dt, ctx);
}

// ------------------------------------------------------------------------------------------------ Dino Crossing

void RoadEventDirector::updateDino(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float spawnX = winW + 110.f;
    const float k = kDeviceScale();
    // script: sign, then three small dinos, then the big straggler
    if (_step == 0) {
        _waited += ctx.dt;
        if (roadClear(spawnX - 90.f, spawnX + 260.f, ctx.apex)) {
            float gy = 0.f;
            ground(spawnX, gy);
            if (Actor* s = spawn(Kind::Sign, "sign_dino", 1, spawnX, gy, 118.f * k, false)) say("dino", s->id);
            _step = 1;
            _wait = 1.9f;                                 // the sign passes well before the first dino arrives
            _waited = 0.f;
        } else if (_waited > 7.f) {
            finishEvent(false);                           // no clean stretch of road: skip the event
        }
        return;
    }
    if (_step >= 1 && _step <= 4) {
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        if (!roadClear(spawnX - 70.f, spawnX + 150.f, ctx.apex)) {
            if (_waited > 4.f) { ++_step; _waited = 0.f; _wait = 0.4f; }   // give up on this one, keep the script moving
            return;
        }
        const bool big = _step == 4;
        float gy = 0.f;
        ground(spawnX, gy);
        const float h = big ? std::min(0.62f * ctx.apex, 118.f * k) : std::min(0.42f * ctx.apex, 78.f * k);
        if (Actor* d = spawn(big ? Kind::DinoBig : Kind::DinoSmall, big ? "dino_big" : "dino_small", 4, spawnX, gy, h, false)) {
            d->hazard = true;
            d->vx = big ? -34.f : -54.f;                  // dinos wander toward you a little
            d->fps = big ? 6.f : 9.f;
            ++_total;
        }
        ++_step;
        _waited = 0.f;
        _wait = big ? 0.f : _rng.range(1.25f, 1.6f);
        return;
    }
    // all spawned: finished when every dino has gone by (or been launched away)
    for (auto& a : _actors) {
        if (a->kind == Kind::DinoSmall || a->kind == Kind::DinoBig) return;
    }
    const bool clean = !_hitThisEvent;
    if (clean && _total > 0) {
        if (_host.award) _host.award(12, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "CLEARED! +12");
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
            r = spawn(Kind::Rider, "rider_delivery", 4, winW + 190.f, gy, 82.f * k, false);
            if (r) {
                r->fps = 12.f;
                r->laneLift = 20.f;                   // back lane: he never rides through the player's obstacles
                _rivalId = r->id;
                say("pizza_ok", r->id);
                _step = 1;
            }
        } else if (_waited > 6.f) {
            finishEvent(false);
        }
        return;
    }
    if (!r) { finishEvent(false); return; }
    const float targetX = std::min(winW - 150.f, ctx.rider.x + 330.f);
    if (_step == 1) {                                     // delivering: ride ahead of the player, dropping boxes
        r->x = approach(r->x, targetX + std::sin(_t * 1.3f) * 26.f, 260.f * ctx.dt);
        _wait -= ctx.dt;
        if (_wait <= 0.f && _total < 7 && r->x < winW - 30.f) {
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
                    ++_total;
                    _wait = 1.5f;
                    PZAudio::play("fx_powerup", 0.25f, 0.2f);
                }
            } else {
                _wait = 0.25f;                            // no safe landing spot right now: try again in a moment
            }
        }
        if (_total >= 7 && _wait <= 0.f) { _step = 2; _wait = 2.4f; }
        return;
    }
    // delivery done: he speeds off, the payoff line comes up, boxes still lying on the road can be collected
    _wait -= ctx.dt;
    r->x += (240.f + 180.f * (2.4f - std::max(0.f, _wait))) * ctx.dt;
    if (r->x > winW + 220.f) {
        removeActor(*r);
        _rivalId = 0;
        const bool good = _score >= 5;
        if (_host.award && good) _host.award(20, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "PIZZA DELIVERED! +20");
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
    auto* name = PZUI::heading("COOL KID", 6.f, Color3B(120, 190, 255));
    name->setPosition(Vec2(0.f, 20.f));
    _marker->addChild(name);
    auto* player = DrawNode::create();
    player->drawSolidCircle(Vec2::ZERO, 9.f, 0.f, 20, Color4F(1.f, 0.82f, 0.2f, 1.f));
    player->drawCircle(Vec2::ZERO, 9.f, 0.f, 20, false, Color4F(0.15f, 0.09f, 0.07f, 1.f), 1.6f);
    _markerPlayer = player;
    _marker->addChild(player, 2);
    if (auto* rv = Sprite::createWithSpriteFrameName("rider_cool_0")) {
        rv->setScale(26.f / rv->getContentSize().height);
        _markerRival = rv;
        _marker->addChild(rv, 3);
    }
    _hud->addChild(_marker, 20);
    layoutHud();
}

void RoadEventDirector::layoutHud() {
    if (!_marker) return;
    const Rect safe = PZLayout::safeRect();
    const float S = PZLayout::uiScale();
    _marker->setScale(S);
    _marker->setPosition(Vec2(safe.getMidX(), safe.getMaxY() - 90.f * S));
}

void RoadEventDirector::updateRaceMarker() {
    if (!_marker || !_markerRival) return;
    const float f = clampf(_lead / kRaceLoseLead, -1.f, 1.f);
    _markerRival->setPositionX(f * 110.f);
    _markerPlayer->setPositionX(0.f);
}

void RoadEventDirector::updateRace(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float k = kDeviceScale();
    Actor* r = find(_rivalId);
    if (_step == 0) {
        float gy = 0.f;
        if (!ground(ctx.rider.x, gy)) {                  // wait until the rider is on solid road
            _waited += ctx.dt;
            if (_waited > 8.f) finishEvent(false);
            return;
        }
        r = spawn(Kind::Rider, "rider_cool", 4, ctx.rider.x - 240.f, gy, 82.f * k, true);
        if (!r) { finishEvent(false); return; }
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
    if (!r) { finishEvent(false); return; }
    _raceT += ctx.dt;
    switch (_raceState) {
    case 0: {                                             // safe introduction: he overtakes you and takes the lead
        _lead += (kRaceStartLead + 240.f) / 1.9f * ctx.dt;
        if (_lead > 30.f && !_hitThisEvent) { say("rival_intro", r->id); _hitThisEvent = true; }
        if (_raceT >= 1.9f) { _raceState = 1; _raceT = 0.f; _hitThisEvent = false; _lead = kRaceStartLead; }
        break;
    }
    case 1: {                                             // racing: he drifts away, you close in with coins, near misses, air
        float gain = std::min(_pendingGain, 280.f * ctx.dt);
        _pendingGain -= gain;
        if (ctx.airborne) gain += kGainAirPerSec * ctx.dt;
        _lead += kRaceDrift * ctx.dt - gain;
        if (_lead <= kRaceWinLead) {
            _raceState = 2;
            _raceT = 0.f;
            if (_host.award) _host.award(40, Vec2(ctx.rider.x + 30.f, ctx.rider.y + 80.f), "OVERTAKE! +40");
            say("rival_win", r->id);
            PZAudio::play("fx_stage", 0.6f, 0.2f);
            PZHaptics::play(PZHaptics::Kind::Success);
            _score = 1;
        } else if (_lead >= kRaceLoseLead || _raceT >= kRaceMaxTime) {
            _raceState = 3;
            _raceT = 0.f;
            say("rival_loss", r->id);
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
        if (_lead > winW + 200.f) { removeActor(*r); _rivalId = 0; finishEvent(false); return; }
        break;
    }
    r->x = ctx.rider.x + _lead;
    if (_raceState <= 1 || _raceState == 3) {
        r->root->setRotation(_lead > 0.f ? -7.f - 2.f * std::sin(_t * 8.f) : 0.f);   // wheelie while ahead
    }
    updateRaceMarker();
}

// ------------------------------------------------------------------------------------------------ Construction Chaos

void RoadEventDirector::updateConstruction(const Context& ctx) {
    const float winW = Director::getInstance()->getWinSize().width;
    const float spawnX = winW + 110.f;
    const float k = kDeviceScale();
    if (_step == 0) {
        _waited += ctx.dt;
        if (roadClear(spawnX - 90.f, spawnX + 300.f, ctx.apex)) {
            float gy = 0.f;
            ground(spawnX, gy);
            if (Actor* s = spawn(Kind::Sign, "sign_work", 1, spawnX, gy, 112.f * k, false)) say("construction", s->id);
            _step = 1;
            _wait = 1.9f;
            _waited = 0.f;
        } else if (_waited > 7.f) {
            finishEvent(false);
        }
        return;
    }
    if (_step >= 1 && _step <= 4) {
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        if (!roadClear(spawnX - 70.f, spawnX + 240.f, ctx.apex)) {
            if (_waited > 4.f) { ++_step; _waited = 0.f; _wait = 0.4f; }
            return;
        }
        float gy = 0.f;
        ground(spawnX, gy);
        const float coneH = std::min(0.30f * ctx.apex, 46.f * k);
        if (_step == 1 || _step == 3) {                       // three cones close together: one jump clears all of them
            for (int i = 0; i < 3; ++i) {
                if (Actor* c = spawn(Kind::Cone, "cone", 1, spawnX + i * 34.f, gy, coneH, false)) {
                    c->hazard = true;
                    c->w = 20.f * k;
                    ++_total;
                }
            }
            _wait = _rng.range(1.4f, 1.8f);
        } else if (_step == 2) {                              // a paint drum rolls toward you: timing, not just distance
            if (Actor* b = spawn(Kind::Barrel, "barrel", 1, spawnX, gy, std::min(0.34f * ctx.apex, 54.f * k), false)) {
                b->hazard = true;
                b->vx = -170.f;
                b->w = b->h * 0.8f;
                ++_total;
            }
            _wait = 1.7f;
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
        if (a->kind == Kind::Cone || a->kind == Kind::Barrel || a->kind == Kind::Worker || a->kind == Kind::Sign) return;
    }
    const bool clean = !_hitThisEvent;
    if (clean && _total > 0 && _host.award) _host.award(12, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "SITE CLEARED! +12");
    finishEvent(clean);
}

// ------------------------------------------------------------------------------------------------ Meteor Shower

void RoadEventDirector::impact(Actor& m, const Context& ctx) {
    float gy = m.targetY;
    ground(m.x, gy);
    const float k = kDeviceScale();
    if (Actor* r = spawn(Kind::Rock, "rock", 1, m.x, gy, std::min(0.30f * ctx.apex, 50.f * k), false)) {
        r->hazard = true;
        r->w = r->spr->getContentSize().width * r->spr->getScale() * 0.62f;
        ++_total;
    }
    PZUI::burst(_scene, Vec2(m.x, gy + 10.f), Color3B(255, 170, 60), 14, 90.f, 0.5f, 900);
    PZAudio::play("fx_explosion", 0.5f, 0.1f);
    if (_host.shake) _host.shake(4.f, 0.2f);
    removeActor(m);
    // a dodge bonus when the rider was close to the impact and did not get hit
    if (!_hitThisEvent && std::abs(m.x - ctx.rider.x) < 150.f && _host.award) {
        _host.award(5, Vec2(m.x, gy + 70.f), "CLOSE DODGE +5");
        ++_score;
    }
}

void RoadEventDirector::updateMeteor(const Context& ctx) {
    const float winH = Director::getInstance()->getWinSize().height;
    const float k = kDeviceScale();
    if (_step == 0) {                                          // a line from the rider first, then the sky opens up
        if (_t <= ctx.dt * 1.5f + 0.001f) say("meteor", 0);
        if (_t > 1.5f) { _step = 1; _wait = 0.f; }
        return;
    }
    // _step 1..N: one strike every ~1.7 s, each one telegraphed by a growing shadow
    if (_step <= 4) {
        _wait -= ctx.dt;
        if (_wait > 0.f) return;
        _waited += ctx.dt;
        const float dur = 1.35f;
        const float X = ctx.rider.x + _speed * (dur + 0.85f) + _rng.range(40.f, 200.f);   // lands well ahead: >= 0.85 s to react to the rock
        if (!roadClear(X - 70.f, X + 70.f, ctx.apex)) {
            if (_waited > 4.f) { ++_step; _waited = 0.f; _wait = 0.5f; }
            return;
        }
        float gy = 0.f;
        ground(X, gy);
        if (Actor* sh = spawn(Kind::Shadow, "shadow", 1, X, gy, 16.f * k, false)) {
            sh->fallDur = dur;
            sh->spr->setScale(0.3f);
            sh->spr->setOpacity(150);
        }
        if (Actor* m = spawn(Kind::Meteor, "meteor", 4, X, winH + 130.f, 96.f * k, false)) {
            m->fallDur = dur;
            m->targetY = gy;
            m->fps = 14.f;
        }
        PZAudio::play("fx_tick", 0.4f, 0.1f);
        ++_step;
        _waited = 0.f;
        _wait = _rng.range(1.6f, 2.0f);
        return;
    }
    for (auto& a : _actors) {
        if (a->kind == Kind::Meteor || a->kind == Kind::Shadow || a->kind == Kind::Rock) return;
    }
    const bool clean = !_hitThisEvent;
    if (clean && _host.award) _host.award(10, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "SKY CLEARED! +10");
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
            if (_waited > 8.f) finishEvent(false);
            return;
        }
        g = spawn(Kind::Rider, "rider_grandma", 4, ctx.rider.x - 260.f, gy, 80.f * k, true);
        if (!g) { finishEvent(false); return; }
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
            const bool good = _score >= 6;
            if (good && _host.award) _host.award(10, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "BASKET BONUS +10");
            finishEvent(good);
        }
        return;
    }
    if (!g) { finishEvent(false); return; }
    _raceT += ctx.dt;
    if (_step == 1) {                                     // slow entrance
        _lead = approach(_lead, -130.f, 46.f * ctx.dt);
        if (_raceT > 2.4f) {
            _step = 2;
            _raceT = 0.f;
            _wait = 0.f;
            if (!g->extra) {
                g->extra = Sprite::createWithSpriteFrameName("flame_0");
                if (g->extra) {
                    g->extra->setAnchorPoint(Vec2(1.f, 0.5f));
                    g->extra->setPosition(Vec2(-26.f * k, 26.f * k));
                    g->extra->setScale(0.9f * k);
                    g->root->addChild(g->extra, -1);
                }
            }
            g->fps = 22.f;
            PZAudio::play("fx_rocket", 0.5f, 0.2f);
            if (_host.shake) _host.shake(3.f, 0.5f);
        }
    } else if (_step == 2) {                              // absurd rocket overtake, dropping coins from the basket
        _lead += (160.f + 900.f * _raceT) * ctx.dt;
        _wait -= ctx.dt;
        if (_wait <= 0.f && _lead > -80.f && _lead < winW - 200.f && _total < 9) {
            float gy = 0.f;
            if (ground(g->x - 40.f, gy)) {
                if (Actor* c = spawn(Kind::Coin, "coin_gold", 1, g->x - 40.f,
                                     gy + 52.f * k + 34.f * std::sin(_total * 0.9f), 28.f * k, false)) {
                    c->w = 34.f * k;
                    c->h = 34.f * k;
                    c->y -= 0.f;
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
            const bool good = _score >= 6;
            if (good && _host.award) _host.award(10, Vec2(ctx.rider.x + 20.f, ctx.rider.y + 70.f), "BASKET BONUS +10");
            finishEvent(good);
        }
        return;
    }
    g->x = ctx.rider.x + _lead;
    g->root->setRotation(_step == 2 ? -9.f : 0.f);        // nose up under rocket power
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
    if (Actor* p = spawn(Kind::Ambient, a ? "rider_ambient_a" : "rider_ambient_b", 4, -190.f, gy, 62.f * kDeviceScale(), true)) {
        p->fps = 14.f;
        p->laneLift = 18.f;                               // rides the back lane
        p->vx = 330.f;                                    // faster than the road: he passes you
        p->w = 0.f;
        p->hazard = false;
        if (_rng.chance(0.35f)) bubble("BEEP BEEP!", p->id, 1.4f);
    }
    _ambientCool = _rng.range(20.f, 35.f);
}

// ------------------------------------------------------------------------------------------------ per-frame actor motion

void RoadEventDirector::updateActors(const Context& ctx) {
    const float dt = ctx.dt;
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
            break;
        }
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
        if ((a.kind == Kind::Box || a.kind == Kind::Coin) && !a.collected) {
            Rect bb(a.x - a.w * 0.5f - 8.f, a.y - 4.f, a.w + 16.f, a.h + 12.f);
            if (bb.intersectsRect(rb)) {
                a.collected = true;
                ++_score;
                if (a.kind == Kind::Coin) { if (_host.award) _host.award(4, Vec2(a.x, a.y + 30.f), "+4"); }
                else if (_host.award) _host.award(3, Vec2(a.x, a.y + 40.f), "PIZZA +3");
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
            Analytics::shared().log("road_event_hit", {{"event", eventName(_cur)}, {"t", std::to_string(static_cast<int>(_t))}, {"kind", std::to_string(static_cast<int>(a.kind))}, {"x", std::to_string(static_cast<int>(a.x))}, {"rider", std::to_string(static_cast<int>(ctx.rider.x))}, {"y", std::to_string(static_cast<int>(a.y))}, {"ry", std::to_string(static_cast<int>(ctx.rider.y))}});
            _host.lethal();
            return;
        }
    }
}

} // namespace V2
