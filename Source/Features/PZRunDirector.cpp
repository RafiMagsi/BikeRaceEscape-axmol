//
//  PZRunDirector.cpp
//
#include "PZRunDirector.h"
#include "PZUI.h"
#include "PZPopups.h"

#include "PZGGameFieldScene.h"
#include "PZGGamefieldPlatforms.h"
#include "PZGCharacter.h"
#include "PZGObstacle.h"
#include "PZGCoin.h"
#include "constants.h"

#include <cmath>
#include <string>

USING_NS_AX;

namespace {
constexpr int kZTint = 150;      // above the world, below game-over / pause overlays (200)
constexpr int kZAura = 120;
constexpr int kZHud  = 1001;
constexpr int kZFx   = 900;
}

PZRunDirector* PZRunDirector::create(PZGGameFieldScene* scene) {
    auto* d = new PZRunDirector();
    if (d->initDirector(scene)) {
        d->autorelease();
        return d;
    }
    delete d;
    return nullptr;
}

bool PZRunDirector::initDirector(PZGGameFieldScene* scene) {
    if (!Node::init()) return false;
    _scene = scene;
    // The scene drives update() explicitly (its own update is custom), so don't self-schedule.

    const Size win = Director::getInstance()->getWinSize();

    _tint = LayerColor::create(Color4B(0, 0, 0, 0), win.width, win.height);
    scene->addChild(_tint, kZTint);

    _hud = Node::create();
    scene->addChild(_hud, kZHud);

    // Combo read-out under the HUD.
    _comboLabel = PZUI::label("", 12.f, PZCfg::kColAccent2);
    _comboLabel->enableOutline(Color4B(0, 0, 0, 220), 2);
    _comboLabel->setVisible(false);
    _hud->addChild(_comboLabel);

    // Auras that follow the rider.
    _shieldAura = DrawNode::create();
    _magnetAura = DrawNode::create();
    _turboAura = DrawNode::create();
    for (DrawNode* a : {_shieldAura, _magnetAura, _turboAura}) {
        a->setVisible(false);
        scene->addChild(a, kZAura);
    }
    _shieldAura->drawSolidCircle(Vec2::ZERO, 62.f, 0.f, 28, PZUI::c4f(PZCfg::kColShield, 0.16f));
    _shieldAura->drawCircle(Vec2::ZERO, 62.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColShield, 0.95f), 3.f);
    _magnetAura->drawCircle(Vec2::ZERO, 74.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColMagnet, 0.8f), 2.f);
    _magnetAura->drawCircle(Vec2::ZERO, 52.f, 0.f, 24, false, PZUI::c4f(PZCfg::kColMagnet, 0.5f), 2.f);
    _turboAura->drawSolidCircle(Vec2::ZERO, 70.f, 0.f, 28, PZUI::c4f(PZCfg::kColTurbo, 0.20f));
    _turboAura->drawCircle(Vec2::ZERO, 70.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColTurbo, 0.9f), 3.f);

    // Power-up HUD rows.
    for (int i = 0; i < 3; ++i) {
        _hudBars[i] = PZUI::bar(110.f, 9.f, 1.f, PZCfg::kColAccent);
        _hudBars[i]->setVisible(false);
        _hud->addChild(_hudBars[i]);
        _hudNames[i] = PZUI::label("", 8.f, PZCfg::kColText);
        _hudNames[i]->setAnchorPoint(Vec2(1.f, 0.5f));
        _hudNames[i]->setVisible(false);
        _hud->addChild(_hudNames[i]);
    }

    // Speed lines (pooled).
    for (int i = 0; i < 9; ++i) {
        auto* line = DrawNode::create();
        const float len = 70.f + CCRANDOM_0_1() * 110.f;
        line->drawSegment(Vec2::ZERO, Vec2(len, 0), 1.2f, Color4F(1.f, 1.f, 1.f, 0.55f));
        line->setPosition(Vec2(CCRANDOM_0_1() * win.width, 40.f + CCRANDOM_0_1() * (win.height - 80.f)));
        line->setOpacity(0);
        scene->addChild(line, kZAura);
        _speedLines.push_back(line);
    }

    layoutHud();
    return true;
}

PZGGamefieldPlatforms* PZRunDirector::platform() const {
    return _scene ? _scene->platformLayer() : nullptr;
}

Vec2 PZRunDirector::riderPos() const {
    return (_scene && _scene->character) ? _scene->character->loc : Vec2::ZERO;
}

Rect PZRunDirector::riderBox() const {
    if (_scene && _scene->character && _scene->character->s_body) {
        return _scene->character->s_body->getBoundingBox();
    }
    const Vec2 p = riderPos();
    return Rect(p.x - 30, p.y - 30, 60, 60);
}

void PZRunDirector::layoutHud() {
    const Size win = Director::getInstance()->getWinSize();
    for (int i = 0; i < 3; ++i) {
        const float y = win.height - 118.f - i * 20.f;
        _hudBars[i]->setPosition(Vec2(win.width * 0.5f + 60.f, y));
        _hudNames[i]->setPosition(Vec2(win.width * 0.5f - 2.f, y));
    }
    _comboLabel->setAnchorPoint(Vec2(0.f, 0.5f));
    _comboLabel->setPosition(Vec2(34.f, win.height - 112.f));
}

void PZRunDirector::onLoaded() {
    if (!_scene) return;
    // Best-score read-out tucked under the live distance counter.
    if (_bestLabel) _bestLabel->removeFromParent();
    _bestLabel = PZUI::label("", 8.f, PZCfg::kColDim);
    _bestLabel->enableOutline(Color4B(0, 0, 0, 200), 2);
    _bestLabel->setAnchorPoint(Vec2(0.f, 0.5f));
    if (_scene->distance_label) {
        const Vec2 p = _scene->distance_label->getPosition();
        _bestLabel->setPosition(Vec2(p.x, p.y - 30.f));
    } else {
        _bestLabel->setPosition(Vec2(30.f, Director::getInstance()->getWinSize().height - 100.f));
    }
    _hud->addChild(_bestLabel);
    refreshHud();
}

void PZRunDirector::refreshHud() {
    if (_bestLabel) {
        char buf[48];
        std::snprintf(buf, sizeof(buf), PZCfg::kStrBest, PZProgress::shared()->bestScore());
        _bestLabel->setString(buf);
    }
}

void PZRunDirector::clearRunVisuals() {
    for (auto& o : _orbs) {
        if (o.node) o.node->removeFromParent();
    }
    _orbs.clear();
    _shieldAura->setVisible(false);
    _magnetAura->setVisible(false);
    _turboAura->setVisible(false);
    for (int i = 0; i < 3; ++i) {
        _hudBars[i]->setVisible(false);
        _hudNames[i]->setVisible(false);
    }
    _comboLabel->setVisible(false);
    _tint->stopAllActions();
    _tint->setOpacity(0);
}

// ---- run lifecycle ---------------------------------------------------------------------------

void PZRunDirector::startRun() {
    clearRunVisuals();
    PZProgress::shared()->refreshMissions();

    _time = 0.f;
    _stage = 1;
    _boost = 1.f;
    _magnetT = _shieldT = _turboT = _graceT = 0.f;
    _combo = 0;
    _comboT = 0.f;
    _reviveOffered = false;
    _committed = false;
    _haveOffset = false;
    _stats = PZRunStats();
    _nextOrbAt = PZCfg::kPowerUpFirstDelay;

    if (_scene && _scene->character && _scene->character->p_body) {
        _scene->character->p_body->SetGravityScale(1.f);
    }
    refreshHud();
}

void PZRunDirector::update(float dt) {
    if (!_scene || !_scene->character) return;
    if (_scene->gameOver || _scene->gamePause) return;
    if (dt > 0.1f) dt = 0.1f;

    _time += dt;
    _auraClock += dt;

    const float k = kDeviceScale();
    const float effSpeed = _scene->gf_speed * _boost;
    const float scroll = dt * effSpeed * 32.f * k * 2.f;

    updateStage();
    updateCombo(dt);
    updatePowerUps(dt);
    updateOrbs(dt, scroll);
    updateMagnet(dt);
    updateTurboRide(dt);
    updateNearMiss();
    updateAuras(dt);
    updateSpeedLines(dt);

    if (_graceT > 0.f) _graceT -= dt;

    // Remember where the rider sits on the surface so turbo / revive can put it back.
    if (!turboActive() && _scene->character->jump_counter == 0) {
        float gy = 0.f;
        if (platform() && platform()->groundYAt(riderPos().x, gy)) {
            _riderOffset = riderPos().y - gy;
            _haveOffset = true;
        }
    }
}

// ---- stages ----------------------------------------------------------------------------------

void PZRunDirector::updateStage() {
    if (!PZCfg::kEnableStages) return;
    const int st = 1 + static_cast<int>(_scene->gf_distance / PZCfg::kStageLength);
    if (st <= _stage) return;
    _stage = st;
    _stats.stage = st;

    const Size win = Director::getInstance()->getWinSize();
    char title[32], sub[32];
    std::snprintf(title, sizeof(title), PZCfg::kStrStage, st);
    std::snprintf(sub, sizeof(sub), PZCfg::kStrStageSub, PZCfg::kStageCoinBonus);
    PZUI::banner(_scene, title, sub, PZCfg::kColAccent, win.height * 0.62f, 1.3f);
    PZUI::shake(_scene, Vec2::ZERO, 5.f, 0.3f);
    _scene->addBonusCoins(PZCfg::kStageCoinBonus);
    _stats.coins += PZCfg::kStageCoinBonus;
    playPickupSound();

    const auto& t = PZCfg::kStageTints[(st - 1) % PZCfg::kStageTintCount];
    _tint->stopAllActions();
    _tint->runAction(TintTo::create(1.2f, t.r, t.g, t.b));
    _tint->runAction(FadeTo::create(1.2f, t.a));
}

// ---- coins / combo ---------------------------------------------------------------------------

void PZRunDirector::onCoin(PZGCoin* coin) {
    if (!coin) return;
    const int reward = static_cast<int>(coin->reward);
    _stats.coins += reward;

    Vec2 at = riderPos();
    if (coin->sprite) {
        const Rect b = coin->sprite->getBoundingBox();
        at = coin->getPosition() + Vec2(b.getMidX(), b.getMidY());
    }
    PZUI::burst(_scene, at, PZCfg::kColAccent, 6, 36.f, 0.35f, kZFx);

    if (!PZCfg::kEnableCombos) return;
    ++_combo;
    _comboT = PZCfg::kComboWindow;
    _stats.maxCombo = std::max(_stats.maxCombo, _combo);

    if (_combo >= 2) {
        char buf[24];
        std::snprintf(buf, sizeof(buf), PZCfg::kStrCombo, _combo);
        _comboLabel->setString(buf);
        _comboLabel->setVisible(true);
        _comboLabel->stopAllActions();
        _comboLabel->setScale(1.35f);
        _comboLabel->runAction(EaseBackOut::create(ScaleTo::create(0.18f, 1.f)));
    }
    if (_combo % PZCfg::kComboBonusEvery == 0) {
        char buf[40];
        std::snprintf(buf, sizeof(buf), PZCfg::kStrComboBonus, PZCfg::kComboBonusCoins * (_combo / PZCfg::kComboBonusEvery));
        awardBonus(PZCfg::kComboBonusCoins * (_combo / PZCfg::kComboBonusEvery), at + Vec2(0, 34), buf, PZCfg::kColAccent2);
    }
}

void PZRunDirector::updateCombo(float dt) {
    if (_combo <= 0) return;
    _comboT -= dt;
    if (_comboT <= 0.f) {
        _combo = 0;
        _comboLabel->setVisible(false);
    }
}

void PZRunDirector::awardBonus(int coins, const Vec2& at, const std::string& text, const Color3B& color) {
    if (coins <= 0) return;
    _scene->addBonusCoins(coins);
    _stats.coins += coins;
    PZUI::floatText(_scene, text, at, color, 10.f, kZFx);
}

// ---- near miss -------------------------------------------------------------------------------

void PZRunDirector::updateNearMiss() {
    if (!PZCfg::kEnableNearMiss || !platform()) return;
    const float k = kDeviceScale();
    const Rect rb = riderBox();
    for (int i = 0; i < platform()->obstacleSlots(); ++i) {
        PZGObstacle* o = platform()->obstacleAt(i);
        if (!o || o->dead || !o->destroyCharacter || o->nearMissDone || !o->sprite) continue;
        Rect ob = o->sprite->getBoundingBox();
        ob.origin += o->getPosition();
        if (ob.getMaxX() >= rb.getMinX()) continue;     // not yet passed
        o->nearMissDone = true;
        if (rb.getMaxX() - ob.getMaxX() > 400.f * k) continue;
        const float clearance = rb.getMinY() - ob.getMaxY();
        if (clearance >= 0.f && clearance < PZCfg::kNearMissRange * k) {
            ++_stats.nearMisses;
            char buf[32];
            std::snprintf(buf, sizeof(buf), PZCfg::kStrCloseCall, PZCfg::kNearMissCoins);
            awardBonus(PZCfg::kNearMissCoins, Vec2(rb.getMidX(), rb.getMaxY() + 20.f), buf, PZCfg::kColGood);
        }
    }
}

// ---- power-ups -------------------------------------------------------------------------------

Color3B PZRunDirector::colorFor(PowerUp kind) {
    switch (kind) {
    case PowerUp::Magnet: return PZCfg::kColMagnet;
    case PowerUp::Shield: return PZCfg::kColShield;
    default:              return PZCfg::kColTurbo;
    }
}

const char* PZRunDirector::nameFor(PowerUp kind) {
    switch (kind) {
    case PowerUp::Magnet: return PZCfg::kStrMagnet;
    case PowerUp::Shield: return PZCfg::kStrShield;
    default:              return PZCfg::kStrTurbo;
    }
}

void PZRunDirector::playPickupSound() {
    if (_scene->coin_pickup_sound) _scene->coin_pickup_sound->playAsSound(false);
}

Node* PZRunDirector::makeOrbVisual(PowerUp kind) const {
    auto* root = Node::create();
    const Color3B col = colorFor(kind);
    const char* art = kind == PowerUp::Magnet ? PZCfg::kArtOrbMagnet
                    : kind == PowerUp::Shield ? PZCfg::kArtOrbShield : PZCfg::kArtOrbTurbo;
    if (PZUI::fileExists(art)) {
        auto* spr = Sprite::create(art);
        if (spr) {
            const float s = (PZCfg::kOrbRadius * 2.f) / std::max(spr->getContentSize().width, 1.f);
            spr->setScale(s);
            root->addChild(spr);
            return root;
        }
    }
    auto* d = DrawNode::create();
    const float r = PZCfg::kOrbRadius;
    d->drawSolidCircle(Vec2::ZERO, r * 1.55f, 0.f, 24, PZUI::c4f(col, 0.20f));
    d->drawSolidCircle(Vec2::ZERO, r, 0.f, 24, PZUI::c4f(Color3B(col.r / 5, col.g / 5, col.b / 5), 0.95f));
    d->drawCircle(Vec2::ZERO, r, 0.f, 24, false, PZUI::c4f(col), 3.f);
    root->addChild(d);
    const char* icon = kind == PowerUp::Magnet ? PZCfg::kStrMagnetIcon
                     : kind == PowerUp::Shield ? PZCfg::kStrShieldIcon : PZCfg::kStrTurboIcon;
    auto* l = PZUI::label(icon, 18.f, col);
    root->addChild(l);
    root->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.45f, 1.12f), ScaleTo::create(0.45f, 0.94f), nullptr)));
    return root;
}

void PZRunDirector::spawnOrb() {
    const Size win = Director::getInstance()->getWinSize();
    const float k = kDeviceScale();
    const float x = win.width + 80.f;
    float gy = 0.f;
    if (!platform() || !platform()->groundYAt(x - 200.f, gy)) {
        gy = riderPos().y - (_haveOffset ? _riderOffset : 30.f);
    }
    const float y = std::min(win.height - 120.f, gy + PZCfg::kOrbHeightAboveGround * k + CCRANDOM_0_1() * 40.f);

    // Rotate through kinds, weighted: shield & magnet common, turbo rarer.
    const float r = CCRANDOM_0_1();
    const PowerUp kind = r < 0.38f ? PowerUp::Magnet : (r < 0.76f ? PowerUp::Shield : PowerUp::Turbo);

    Orb orb;
    orb.kind = kind;
    orb.node = makeOrbVisual(kind);
    orb.node->setPosition(Vec2(x, y));
    _scene->addChild(orb.node, kZAura + 5);
    _orbs.push_back(orb);
}

void PZRunDirector::updateOrbs(float dt, float scroll) {
    if (PZCfg::kEnablePowerUps && _time >= _nextOrbAt && _orbs.empty() && _turboT <= 0.f) {
        spawnOrb();
        _nextOrbAt = _time + PZCfg::kPowerUpIntervalMin +
                     CCRANDOM_0_1() * (PZCfg::kPowerUpIntervalMax - PZCfg::kPowerUpIntervalMin);
    }
    const Vec2 rp = riderPos();
    for (size_t i = 0; i < _orbs.size();) {
        Orb& o = _orbs[i];
        o.node->setPositionX(o.node->getPositionX() - scroll);
        const Vec2 p = o.node->getPosition();
        const bool offscreen = p.x < -80.f;
        const float pick = PZCfg::kOrbRadius + 34.f * kDeviceScale();
        if (!offscreen && p.distance(rp) < pick) {
            collect(o.kind, p);
            o.node->removeFromParent();
            _orbs.erase(_orbs.begin() + i);
            continue;
        }
        if (offscreen) {
            o.node->removeFromParent();
            _orbs.erase(_orbs.begin() + i);
            continue;
        }
        ++i;
    }
}

void PZRunDirector::collect(PowerUp kind, const Vec2& at) {
    ++_stats.powerUps;
    playPickupSound();
    PZUI::burst(_scene, at, colorFor(kind), 12, 70.f, 0.5f, kZFx);
    PZUI::floatText(_scene, nameFor(kind), at + Vec2(0, 30), colorFor(kind), 12.f, kZFx);
    switch (kind) {
    case PowerUp::Magnet: _magnetT = PZCfg::kMagnetDuration; break;
    case PowerUp::Shield: _shieldT = PZCfg::kShieldDuration; break;
    case PowerUp::Turbo:
        if (_turboT <= 0.f && _scene->character && _scene->character->p_body) {
            // Cruise at the current surface height; gravity off so gaps can't catch the rider.
            _turboLockY = riderPos().y;
            _scene->character->p_body->SetGravityScale(0.f);
            _scene->character->jump_counter = 1;
            PZUI::shake(_scene, Vec2::ZERO, 7.f, 0.35f);
        }
        _turboT = PZCfg::kTurboDuration;
        break;
    }
}

void PZRunDirector::grant(PowerUp kind) {
    collect(kind, riderPos());
}

void PZRunDirector::updatePowerUps(float dt) {
    if (_magnetT > 0.f) _magnetT = std::max(0.f, _magnetT - dt);
    if (_shieldT > 0.f) _shieldT = std::max(0.f, _shieldT - dt);
    if (_turboT > 0.f) {
        _turboT -= dt;
        if (_turboT <= 0.f) endTurbo();
    }
    const float target = _turboT > 0.f ? PZCfg::kTurboSpeedMult : 1.f;
    _boost += (target - _boost) * std::min(1.f, dt * 4.f);
    if (std::abs(_boost - target) < 0.002f) _boost = target;

    const float durations[3] = {PZCfg::kMagnetDuration, PZCfg::kShieldDuration, PZCfg::kTurboDuration};
    const float left[3] = {_magnetT, _shieldT, _turboT};
    const PowerUp kinds[3] = {PowerUp::Magnet, PowerUp::Shield, PowerUp::Turbo};
    int row = 0;
    for (int i = 0; i < 3; ++i) {
        if (left[i] > 0.f) {
            const Color3B col = colorFor(kinds[i]);
            PZUI::redrawBar(_hudBars[row], 110.f, 9.f, left[i] / durations[i], col);
            _hudBars[row]->setVisible(true);
            _hudNames[row]->setString(nameFor(kinds[i]));
            _hudNames[row]->setColor(col);
            _hudNames[row]->setVisible(true);
            ++row;
        }
    }
    const Size win = Director::getInstance()->getWinSize();
    for (int r = 0; r < 3; ++r) {
        if (r >= row) {
            _hudBars[r]->setVisible(false);
            _hudNames[r]->setVisible(false);
        } else {
            const float y = win.height - 118.f - r * 20.f;
            _hudBars[r]->setPositionY(y);
            _hudNames[r]->setPositionY(y);
        }
    }
}

void PZRunDirector::updateMagnet(float dt) {
    if (!platform()) return;
    float radius = 0.f;
    if (turboActive()) radius = PZCfg::kTurboMagnetRadius;
    else if (_magnetT > 0.f) radius = PZCfg::kMagnetRadius;
    if (radius <= 0.f) return;

    const float k = kDeviceScale();
    const Vec2 rp = riderPos();
    for (int i = 0; i < platform()->coinSlots(); ++i) {
        PZGCoin* c = platform()->coinAt(i);
        if (!c || c->disabled || !c->sprite) continue;
        const Rect b = c->sprite->getBoundingBox();
        const Vec2 centre = c->getPosition() + Vec2(b.getMidX(), b.getMidY());
        const Vec2 to = rp - centre;
        const float d = to.length();
        if (d > radius * k) continue;
        if (d < 38.f * k) {
            _scene->coinPickup(c);
            continue;
        }
        const float pull = (700.f + (radius * k - d) * 4.f) * dt;
        c->setPosition(c->getPosition() + to / d * std::min(pull, d));
    }
}

void PZRunDirector::updateTurboRide(float dt) {
    if (!turboActive() || !_scene->character || !_scene->character->p_body) return;
    PZGCharacter* ch = _scene->character;
    // Ease the rider toward the cruise line; the world keeps scrolling underneath.
    const float bob = std::sin(_auraClock * 9.f) * 6.f;
    const float dy = (_turboLockY + bob) - ch->loc.y;
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, dy * 9.f * kPhysicsWorldScale()));
    ch->jump_counter = 1;

    // Smash anything lethal the rider touches.
    if (platform()) {
        const Rect rb = riderBox();
        for (int i = 0; i < platform()->obstacleSlots(); ++i) {
            PZGObstacle* o = platform()->obstacleAt(i);
            if (!o || o->dead || !o->sprite) continue;
            Rect ob = o->sprite->getBoundingBox();
            ob.origin += o->getPosition();
            if (ob.intersectsRect(rb)) smash(o);
        }
    }
    // Flame trail.
    _trailClock += dt;
    if (_trailClock > 0.05f) {
        _trailClock = 0.f;
        PZUI::burst(_scene, ch->loc + Vec2(-34, -8), PZCfg::kColTurbo, 2, 26.f, 0.3f, kZFx - 10);
    }
}

void PZRunDirector::endTurbo() {
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) return;
    ch->p_body->SetGravityScale(1.f);
    _graceT = std::max(_graceT, 1.0f);

    // Land on solid ground: slide the world if the rider is over a gap.
    float gy = 0.f;
    PZGGamefieldPlatforms* p = platform();
    if (p) {
        if (!p->groundYAt(ch->loc.x, gy)) {
            p->alignForRevive(ch->loc.x, gy);
            if (!p->groundYAt(ch->loc.x, gy)) gy = _turboLockY - (_haveOffset ? _riderOffset : 0.f);
        }
        const float y = gy + (_haveOffset ? _riderOffset : 0.f) + 4.f;
        ch->setLoc(Vec2(ch->loc.x, y));
        ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
    }
    ch->jump_counter = 1;   // airborne until the platform contact re-grounds the rider
    PZUI::burst(_scene, ch->loc, PZCfg::kColTurbo, 14, 80.f, 0.5f, kZFx);
}

void PZRunDirector::smash(PZGObstacle* o) {
    if (!o || o->dead) return;
    o->dead = true;
    o->deleteReady = true;
    if (o->sprite) {
        const Rect b = o->sprite->getBoundingBox();
        const Vec2 c = o->getPosition() + Vec2(b.getMidX(), b.getMidY());
        PZUI::burst(_scene, c, PZCfg::kColAccent, 10, 70.f, 0.45f, kZFx);
        PZUI::floatText(_scene, PZCfg::kStrSmash, c + Vec2(0, 30), PZCfg::kColTurbo, 10.f, kZFx);
    }
    PZUI::shake(_scene, Vec2::ZERO, 4.f, 0.2f);
}

bool PZRunDirector::onObstacleHit(PZGObstacle* o) {
    if (!o) return false;
    if (turboActive() || _graceT > 0.f) {
        smash(o);
        return true;
    }
    if (_shieldT > 0.f) {
        _shieldT = 0.f;
        _graceT = PZCfg::kHitGrace;
        smash(o);
        PZUI::floatText(_scene, PZCfg::kStrShieldBreak, riderPos() + Vec2(0, 50), PZCfg::kColShield, 12.f, kZFx);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 14, 90.f, 0.5f, kZFx);
        return true;
    }
    return false;
}

// ---- visuals ---------------------------------------------------------------------------------

void PZRunDirector::updateAuras(float dt) {
    const Vec2 rp = riderPos();
    const float pulse = 1.f + 0.05f * std::sin(_auraClock * 8.f);
    _shieldAura->setVisible(_shieldT > 0.f);
    if (_shieldT > 0.f) {
        _shieldAura->setPosition(rp);
        _shieldAura->setScale(pulse);
        // Blink while about to expire.
        _shieldAura->setOpacity(_shieldT < 2.f && std::fmod(_auraClock, 0.3f) < 0.15f ? 90 : 255);
    }
    _magnetAura->setVisible(_magnetT > 0.f);
    if (_magnetT > 0.f) {
        _magnetAura->setPosition(rp);
        _magnetAura->setRotation(_auraClock * 120.f);
        _magnetAura->setScale(1.f + 0.12f * std::sin(_auraClock * 5.f));
    }
    _turboAura->setVisible(turboActive());
    if (turboActive()) {
        _turboAura->setPosition(rp);
        _turboAura->setScale(pulse * 1.05f);
    }
}

void PZRunDirector::updateSpeedLines(float dt) {
    const Size win = Director::getInstance()->getWinSize();
    const float maxSpeed = std::max(_scene->gf_speed_max, 1.f);
    const float eff = _scene->gf_speed * _boost;
    const float intensity = std::max(0.f, std::min(1.f, (eff - 0.8f * maxSpeed) / (0.6f * maxSpeed)));
    for (DrawNode* line : _speedLines) {
        line->setOpacity(static_cast<uint8_t>(190.f * intensity));
        if (intensity <= 0.01f) continue;
        line->setPositionX(line->getPositionX() - (900.f + 700.f * intensity) * dt);
        if (line->getPositionX() < -200.f) {
            line->setPosition(Vec2(win.width + CCRANDOM_0_1() * 300.f, 40.f + CCRANDOM_0_1() * (win.height - 80.f)));
        }
    }
}

// ---- death / revive --------------------------------------------------------------------------

bool PZRunDirector::onDeath() {
    // An active shield also saves the rider from one fall (hits are already absorbed in onObstacleHit).
    if (_shieldT > 0.f) {
        _shieldT = 0.f;
        rescueRider();
        PZUI::floatText(_scene, PZCfg::kStrShieldBreak, riderPos() + Vec2(0, 50), PZCfg::kColShield, 12.f, kZFx);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 14, 90.f, 0.5f, kZFx);
        return true;
    }
    if (!PZCfg::kEnableRevive || _reviveOffered) return false;
    auto* sc = PZSettingsController::shared();
    if (sc->coins < PZCfg::kReviveCost) return false;     // can't afford: go straight to game over
    _reviveOffered = true;

    _scene->gameOver = true;     // freeze the world while the offer is up
    PZUI::shake(_scene, Vec2::ZERO, 10.f, 0.4f);

    // A short beat so the player sees what killed them, then the offer.
    runAction(Sequence::create(DelayTime::create(0.45f), CallFunc::create([this]() {
        PZPopups::showRevive(_scene, PZCfg::kReviveCost, PZSettingsController::shared()->coins,
                             [this]() { reviveAccepted(); }, [this]() { reviveDeclined(); });
    }), nullptr));
    return true;
}

void PZRunDirector::rescueRider() {
    PZGCharacter* ch = _scene->character;
    PZGGamefieldPlatforms* p = platform();
    if (ch && ch->p_body) {
        float gy = 0.f;
        if (p && p->alignForRevive(ch->loc.x, gy)) {
            const float y = gy + (_haveOffset ? _riderOffset : 30.f) + 6.f;
            ch->setLoc(Vec2(ch->loc.x, y));
        } else if (p && p->groundYAt(ch->loc.x, gy)) {
            ch->setLoc(Vec2(ch->loc.x, gy + (_haveOffset ? _riderOffset : 30.f) + 6.f));
        } else {
            ch->setLoc(Vec2(ch->loc.x, Director::getInstance()->getWinSize().height * 0.6f));
        }
        ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
        ch->reset();
        ch->jump_counter = 1;
    }
    _graceT = std::max(_graceT, PZCfg::kHitGrace + 1.2f);
}

void PZRunDirector::reviveAccepted() {
    auto* sc = PZSettingsController::shared();
    sc->coins = std::max(0, static_cast<int>(sc->coins) - PZCfg::kReviveCost);
    sc->save();
    _scene->gf_coins = sc->coins;
    if (_scene->coins_label) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(_scene->gf_coins));
        _scene->coins_label->setString(buf);
    }
    _stats.revived = true;

    rescueRider();
    _graceT = PZCfg::kHitGrace + 1.2f;
    _shieldT = 0.f;
    _scene->gameOver = false;
    _scene->gamePause = false;

    const Size win = Director::getInstance()->getWinSize();
    PZUI::banner(_scene, "GO!", "", PZCfg::kColGood, win.height * 0.6f, 0.5f);
    PZUI::burst(_scene, riderPos(), PZCfg::kColGood, 14, 90.f, 0.5f, kZFx);
}

void PZRunDirector::reviveDeclined() {
    _scene->gameOverCallBack(true);   // _reviveOffered is set, so this proceeds to the real game over
}

// ---- results ---------------------------------------------------------------------------------

void PZRunDirector::presentResults(Node* gameOverLayer, Label* distanceLabel) {
    if (_committed || !gameOverLayer) return;
    _committed = true;
    _stats.distance = static_cast<int>(_scene->gf_distance);
    _stats.stage = _stage;

    // Run is over: drop the live power-up read-outs.
    for (int i = 0; i < 3; ++i) {
        _hudBars[i]->setVisible(false);
        _hudNames[i]->setVisible(false);
    }
    _shieldAura->setVisible(false);
    _magnetAura->setVisible(false);
    _turboAura->setVisible(false);
    _comboLabel->setVisible(false);
    _magnetT = _shieldT = _turboT = 0.f;
    const PZRunResult result = PZProgress::shared()->commitRun(_stats);
    refreshHud();
    PZPopups::attachResults(gameOverLayer, distanceLabel, _stats, result);
}
