//
//  PZRunDirector.cpp
//
#include "PZRunDirector.h"
#include "PZUI.h"
#include "PZPopups.h"
#include "PZLayout.h"
#include "PZHaptics.h"

#include "PZGGameFieldScene.h"
#include "PZGGamefieldPlatforms.h"
#include "PZGCharacter.h"
#include "PZGObstacle.h"
#include "PZGCoin.h"
#include "constants.h"
#include "Ads/AdsController.h"

#include <algorithm>
#include <cmath>
#include <string>

USING_NS_AX;

namespace {
constexpr int kZTint  = 150;     // above the world, below game-over / pause overlays (200)
constexpr int kZAura  = 120;
constexpr int kZOrb   = 125;
constexpr int kZHud   = 1001;
constexpr int kZFx    = 900;
constexpr int kZFlash = 940;

float rnd(float lo, float hi) { return lo + (hi - lo) * CCRANDOM_0_1(); }
} // namespace

PZRunDirector* PZRunDirector::create(PZGGameFieldScene* scene) {
    PZCfg::ensureLoaded();
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

    _flash = LayerColor::create(Color4B(255, 255, 255, 0), win.width, win.height);
    _flash->setBlendFunc(BlendFunc::ADDITIVE);
    scene->addChild(_flash, kZFlash);

    _hud = Node::create();
    scene->addChild(_hud, kZHud);

    // Combo read-out under the coin counter.
    _comboLabel = PZUI::heading("", 13.f, PZCfg::kColAccent2);
    _comboLabel->setAnchorPoint(Vec2(0.f, 0.5f));
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
    _shieldAura->drawCircle(Vec2::ZERO, 56.f, 0.f, 28, false, PZUI::c4f(Color3B(255, 255, 255), 0.35f), 1.5f);
    _magnetAura->drawCircle(Vec2::ZERO, 74.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColMagnet, 0.8f), 2.f);
    _magnetAura->drawCircle(Vec2::ZERO, 52.f, 0.f, 24, false, PZUI::c4f(PZCfg::kColMagnet, 0.5f), 2.f);
    _turboAura->drawSolidCircle(Vec2::ZERO, 70.f, 0.f, 28, PZUI::c4f(PZCfg::kColTurbo, 0.20f));
    _turboAura->drawCircle(Vec2::ZERO, 70.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColTurbo, 0.9f), 3.f);

    // Power-up chips along the top of the screen: bronze orb icon with a framed timer bar underneath.
    for (int i = 0; i < 4; ++i) {
        HudRow& r = _rows[i];
        r.root = Node::create();
        r.root->setVisible(false);
        _hud->addChild(r.root);
        r.icon = makeOrbVisual(static_cast<PowerUp>(i), 0.7f);
        r.icon->setPosition(Vec2(0.f, 8.f));
        r.root->addChild(r.icon);
        r.bar = PZUI::bar(66.f, 9.f, 1.f, PZCfg::kColAccent);
        r.bar->setPosition(Vec2(0.f, -26.f));
        r.root->addChild(r.bar);
        r.name = nullptr;
    }

    // Live mission tracker (top-right, under the pause button).
    _tracker = Node::create();
    _tracker->addChild(PZUI::panel(250.f, 54.f, 10.f));
    _trackerText = PZUI::heading("", 7.f, PZCfg::kColText);
    _trackerText->setAnchorPoint(Vec2(0.f, 0.5f));
    _trackerText->setPosition(Vec2(-112.f, 12.f));
    _tracker->addChild(_trackerText);
    _trackerBar = PZUI::bar(150.f, 9.f, 0.f, PZCfg::kColAccent2);
    _trackerBar->setPosition(Vec2(-37.f, -12.f));
    _tracker->addChild(_trackerBar);
    _trackerCount = PZUI::heading("", 7.f, PZCfg::kColAccent);
    _trackerCount->setAnchorPoint(Vec2(1.f, 0.5f));
    _trackerCount->setPosition(Vec2(112.f, -12.f));
    _tracker->addChild(_trackerCount);
    _tracker->setVisible(false);
    _hud->addChild(_tracker);

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

    // Ambient motes (embers / dust) drifting through the scene.
    if (PZCfg::kEnableFx) {
        for (int i = 0; i < 18; ++i) {
            auto* m = DrawNode::create();
            m->drawDot(Vec2::ZERO, rnd(1.2f, 2.8f), Color4F(1.f, 0.86f, 0.6f, rnd(0.18f, 0.4f)));
            m->setPosition(Vec2(CCRANDOM_0_1() * win.width, CCRANDOM_0_1() * win.height));
            scene->addChild(m, kZAura - 20);
            _motes.push_back(m);
        }
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

float PZRunDirector::apex() const {
    return platform() ? platform()->riderApex : 160.f;
}

bool PZRunDirector::frozen() const {
    return std::chrono::steady_clock::now() < _freezeUntil;
}

void PZRunDirector::freezeFor(float seconds) {
    if (seconds <= 0.f) return;
    _freezeUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(seconds * 1000.f));
}

void PZRunDirector::layoutHud() {
    const Rect safe = PZLayout::safeRect();
    // Chips sit centred just under the distance / best read-outs; the combo counter hangs under the coin counter and
    // the mission tracker under the pause button, so nothing overlaps the original HUD.
    for (int i = 0; i < 4; ++i) {
        _rows[i].root->setPosition(Vec2(safe.getMidX(), safe.getMaxY() - 126.f));
    }
    _comboLabel->setPosition(Vec2(safe.getMinX() + 24.f, safe.getMaxY() - 100.f));
    if (_tracker) _tracker->setPosition(Vec2(safe.getMaxX() - 130.f, safe.getMaxY() - 112.f));
}

void PZRunDirector::onLoaded() {
    if (!_scene) return;
    // Best-score read-out tucked under the live distance counter (same gold arcade style as the HUD).
    if (_bestLabel) _bestLabel->removeFromParent();
    _bestLabel = PZUI::heading("", 8.f, PZCfg::kColDim);
    _bestLabel->setAnchorPoint(Vec2(0.f, 0.5f));
    if (_scene->distance_label) {
        const Vec2 p = _scene->distance_label->getPosition();
        _bestLabel->setPosition(Vec2(p.x, p.y - 30.f));
    } else {
        _bestLabel->setPosition(Vec2(PZLayout::safeRect().getMidX(), Director::getInstance()->getWinSize().height - 92.f));
    }
    _hud->addChild(_bestLabel);
    layoutHud();
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
    for (auto& w : _waves) {
        if (w.ring) w.ring->removeFromParent();
    }
    _waves.clear();
    _kills.clear();
    _shieldAura->setVisible(false);
    _magnetAura->setVisible(false);
    _turboAura->setVisible(false);
    for (auto& r : _rows) r.root->setVisible(false);
    _comboLabel->setVisible(false);
    _tint->stopAllActions();
    _tint->setOpacity(0);
    _flash->stopAllActions();
    _flash->setOpacity(0);
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
    _mercyUsed = 0;
    _haveOffset = false;
    _prevJump = 0;
    _airTime = 0.f;
    _flying = 0;
    _freezeUntil = std::chrono::steady_clock::time_point{};
    _stats = PZRunStats();
    _nextOrbAt = PZCfg::kPowerUpFirstDelay;
    _trackerSlot = -1;
    _trackerClock = 0.f;
    for (bool& d : _trackerDone) d = false;
    if (PZCfg::kEnableRevive && PZCfg::kEnableReviveVideo) PZ::AdsController::shared()->requestRewarded();   // warm a rewarded ad

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
    updateWaves(dt);
    updateKills(dt);
    updateNearMiss();
    updateAuras(dt);
    updateSpeedLines(dt);
    updateFx(dt, scroll);
    updateMissionTracker(dt);

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
    flash(PZCfg::kColAccent, 0.35f, 0.4f);
    ring(riderPos(), PZCfg::kColAccent, 260.f, 0.6f);
    PZHaptics::play(PZHaptics::Kind::Success);
    _scene->addBonusCoins(PZCfg::kStageCoinBonus);
    _stats.coins += PZCfg::kStageCoinBonus;
    playPickupSound();

    if (!PZCfg::kStageTints.empty()) {
        const auto& t = PZCfg::kStageTints[(st - 1) % PZCfg::kStageTints.size()];
        _tint->stopAllActions();
        _tint->runAction(TintTo::create(1.2f, t.r, t.g, t.b));
        _tint->runAction(FadeTo::create(1.2f, t.a));
    }
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
    PZHaptics::play(PZHaptics::Kind::Tick);
    if (PZCfg::kEnableFx) coinFly(at);

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
        _comboLabel->setScale(1.45f);
        _comboLabel->runAction(EaseBackOut::create(ScaleTo::create(0.2f, 1.f)));
    }
    if (_combo % PZCfg::kComboBonusEvery == 0) {
        const int bonus = PZCfg::kComboBonusCoins * (_combo / PZCfg::kComboBonusEvery);
        char buf[40];
        std::snprintf(buf, sizeof(buf), PZCfg::kStrComboBonus, bonus);
        awardBonus(bonus, at + Vec2(0, 34), buf, PZCfg::kColAccent2);
        PZHaptics::play(PZHaptics::Kind::Light);
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
    if (!text.empty()) PZUI::floatText(_scene, text, at, color, 10.f, kZFx);
}

// ---- near miss -------------------------------------------------------------------------------

void PZRunDirector::updateNearMiss() {
    if (!PZCfg::kEnableNearMiss || !platform()) return;
    const float k = kDeviceScale();
    const Rect rb = riderBox();
    for (int i = 0; i < platform()->obstacleSlots(); ++i) {
        PZGObstacle* o = platform()->obstacleAt(i);
        if (!o || o->dead || !o->destroyCharacter || o->nearMissDone || !o->sprite) continue;
        const Rect ob = o->worldBox();
        if (ob.getMaxX() >= rb.getMinX()) continue;     // not yet passed
        o->nearMissDone = true;
        if (rb.getMaxX() - ob.getMaxX() > 400.f * k) continue;
        const float clearance = rb.getMinY() - ob.getMaxY();
        if (clearance >= 0.f && clearance < PZCfg::kNearMissRange * k) {
            ++_stats.nearMisses;
            char buf[32];
            std::snprintf(buf, sizeof(buf), PZCfg::kStrCloseCall, PZCfg::kNearMissCoins);
            awardBonus(PZCfg::kNearMissCoins, Vec2(rb.getMidX(), rb.getMaxY() + 20.f), buf, PZCfg::kColGood);
            ring(Vec2(rb.getMidX(), rb.getMinY()), PZCfg::kColGood, 90.f, 0.35f);
            PZHaptics::play(PZHaptics::Kind::Medium);
        }
    }
}

// ---- power-ups -------------------------------------------------------------------------------

Color3B PZRunDirector::colorFor(PowerUp kind) {
    switch (kind) {
    case PowerUp::Magnet: return PZCfg::kColMagnet;
    case PowerUp::Shield: return PZCfg::kColShield;
    case PowerUp::Blast:  return PZCfg::kColBlast;
    default:              return PZCfg::kColTurbo;
    }
}

const char* PZRunDirector::nameFor(PowerUp kind) {
    switch (kind) {
    case PowerUp::Magnet: return PZCfg::kStrMagnet;
    case PowerUp::Shield: return PZCfg::kStrShield;
    case PowerUp::Blast:  return PZCfg::kStrBlastName;
    default:              return PZCfg::kStrTurbo;
    }
}

void PZRunDirector::playPickupSound() {
    if (_scene->coin_pickup_sound) _scene->coin_pickup_sound->playAsSound(false);
}

Node* PZRunDirector::makeOrbVisual(PowerUp kind, float scale) const {
    auto* root = Node::create();
    const Color3B col = colorFor(kind);
    const char* art = kind == PowerUp::Magnet ? PZCfg::kArtOrbMagnet
                    : kind == PowerUp::Shield ? PZCfg::kArtOrbShield
                    : kind == PowerUp::Blast  ? PZCfg::kArtOrbBlast : PZCfg::kArtOrbTurbo;
    const float r = PZCfg::kOrbRadius * scale;
    if (PZUI::fileExists(art)) {
        auto* spr = Sprite::create(art);
        if (spr) {
            spr->setScale((r * 2.f) / std::max(spr->getContentSize().width, 1.f));
            root->addChild(spr);
            return root;
        }
    }
    // Bronze-ringed navy disc with a coloured core: the same language as the game's round buttons.
    auto* d = DrawNode::create();
    d->drawSolidCircle(Vec2::ZERO, r * 1.55f, 0.f, 24, PZUI::c4f(col, 0.20f));
    d->drawSolidCircle(Vec2::ZERO, r + 3.f * scale, 0.f, 24, PZUI::c4f(PZCfg::kColPanelEdge));
    d->drawSolidCircle(Vec2::ZERO, r, 0.f, 24, PZUI::c4f(Color3B(14, 18, 32), 1.f));
    d->drawCircle(Vec2::ZERO, r * 0.82f, 0.f, 24, false, PZUI::c4f(col), 3.f * scale);
    d->drawCircle(Vec2::ZERO, r + 3.f * scale, 0.f, 24, false, PZUI::c4f(PZCfg::kColPanelEdge2, 0.8f), 1.2f);
    root->addChild(d);
    const char* icon = kind == PowerUp::Magnet ? PZCfg::kStrMagnetIcon
                     : kind == PowerUp::Shield ? PZCfg::kStrShieldIcon
                     : kind == PowerUp::Blast  ? PZCfg::kStrBlastIcon : PZCfg::kStrTurboIcon;
    auto* l = PZUI::heading(icon, 18.f * scale, col);
    root->addChild(l);
    if (scale >= 0.9f) {
        root->runAction(RepeatForever::create(Sequence::create(ScaleTo::create(0.45f, 1.12f), ScaleTo::create(0.45f, 0.94f), nullptr)));
    }
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
    // Always reachable with a jump, on every platform's physics scale.
    const float lift = std::min(PZCfg::kOrbHeightAboveGround * k, apex() * 0.78f);
    const float y = std::min(win.height - 120.f, gy + lift + CCRANDOM_0_1() * 30.f);

    const float wM = PZCfg::kWeightMagnet, wS = PZCfg::kWeightShield, wT = PZCfg::kWeightTurbo, wB = PZCfg::kWeightBlast;
    const float total = std::max(0.001f, wM + wS + wT + wB);
    const float r = CCRANDOM_0_1() * total;
    PowerUp kind = PowerUp::Blast;
    if (r < wM) kind = PowerUp::Magnet;
    else if (r < wM + wS) kind = PowerUp::Shield;
    else if (r < wM + wS + wT) kind = PowerUp::Turbo;

    Orb orb;
    orb.kind = kind;
    orb.node = makeOrbVisual(kind);
    orb.node->setPosition(Vec2(x, y));
    _scene->addChild(orb.node, kZOrb);
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
            const PowerUp kind = o.kind;
            o.node->removeFromParent();
            _orbs.erase(_orbs.begin() + i);
            collect(kind, p);
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

void PZRunDirector::grant(PowerUp kind) {
    collect(kind, riderPos());
}

void PZRunDirector::grantByName(const std::string& name) {
    grant(name == "magnet" ? PowerUp::Magnet : name == "shield" ? PowerUp::Shield : name == "blast" ? PowerUp::Blast : PowerUp::Turbo);
}

void PZRunDirector::collect(PowerUp kind, const Vec2& at, float durationOverride) {
    const bool bonus = durationOverride > 0.f;      // granted by revive: no pickup chrome
    const Color3B col = colorFor(kind);
    if (!bonus) {
        ++_stats.powerUps;
        playPickupSound();
        PZUI::floatText(_scene, nameFor(kind), at + Vec2(0, 34), col, 12.f, kZFx);
        PZHaptics::play(kind == PowerUp::Blast ? PZHaptics::Kind::Heavy : PZHaptics::Kind::Medium);
    }
    PZUI::burst(_scene, at, col, 12, 70.f, 0.5f, kZFx);
    ring(at, col, 110.f, 0.45f);

    switch (kind) {
    case PowerUp::Magnet:
        _magnetT = _magnetMax = bonus ? durationOverride : PZCfg::kMagnetDuration;
        break;
    case PowerUp::Shield:
        _shieldT = _shieldMax = bonus ? durationOverride : PZCfg::kShieldDuration;
        break;
    case PowerUp::Turbo:
        if (_turboT <= 0.f && _scene->character && _scene->character->p_body) {
            // Cruise at the current surface height; gravity off so gaps can't catch the rider.
            _turboLockY = riderPos().y;
            _scene->character->p_body->SetGravityScale(0.f);
            _scene->character->jump_counter = 1;
            PZUI::shake(_scene, Vec2::ZERO, 7.f, 0.35f);
            flash(col, 0.3f, 0.3f);
        }
        _turboT = _turboMax = bonus ? durationOverride : PZCfg::kTurboDuration;
        break;
    case PowerUp::Blast:
        startWave(riderPos(), PZCfg::kBlastRadius, PZCfg::kBlastWaveSpeed, col);
        flash(Color3B(255, 255, 255), 0.55f, 0.35f);
        PZUI::shake(_scene, Vec2::ZERO, 14.f, 0.5f);
        freezeFor(PZCfg::kHitStop * 1.2f);
        PZUI::banner(_scene, PZCfg::kStrBlast, "", col, Director::getInstance()->getWinSize().height * 0.6f, 0.6f);
        runAction(Sequence::create(DelayTime::create(0.14f), CallFunc::create([]() { PZHaptics::play(PZHaptics::Kind::Medium); }), nullptr));
        break;
    }
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

    const float left[4] = {_magnetT, _shieldT, _turboT, 0.f};
    const float full[4] = {std::max(_magnetMax, 0.01f), std::max(_shieldMax, 0.01f), std::max(_turboMax, 0.01f), 1.f};
    const PowerUp kinds[4] = {PowerUp::Magnet, PowerUp::Shield, PowerUp::Turbo, PowerUp::Blast};
    const Rect safe = PZLayout::safeRect();
    int active = 0;
    for (int i = 0; i < 3; ++i) if (left[i] > 0.f) ++active;
    const float spacing = 92.f;
    int slot = 0;
    for (int i = 0; i < 3; ++i) {
        HudRow& r = _rows[i];
        if (left[i] > 0.f) {
            const float frac = left[i] / full[i];
            PZUI::redrawBar(r.bar, 66.f, 9.f, frac, frac < 0.25f ? PZCfg::kColBad : colorFor(kinds[i]));
            r.root->setPosition(Vec2(safe.getMidX() + (slot - (active - 1) * 0.5f) * spacing, safe.getMaxY() - 126.f));
            // Gentle pulse when about to run out.
            r.icon->setScale(frac < 0.25f ? 1.f + 0.12f * std::sin(_auraClock * 14.f) : 1.f);
            r.root->setVisible(true);
            ++slot;
        } else {
            r.root->setVisible(false);
        }
    }
    _rows[3].root->setVisible(false);
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

    // Plough through anything lethal the rider touches (launched away with a spin).
    if (platform()) {
        const Rect rb = riderBox();
        for (int i = 0; i < platform()->obstacleSlots(); ++i) {
            PZGObstacle* o = platform()->obstacleAt(i);
            if (!o || o->dead || !o->sprite || !o->destroyCharacter) continue;
            if (o->worldBox().intersectsRect(rb)) destroyObstacle(o, Style::Launch);
        }
    }
    // Flame trail.
    _trailClock += dt;
    if (_trailClock > 0.045f) {
        _trailClock = 0.f;
        PZUI::burst(_scene, ch->loc + Vec2(-34, -8), PZCfg::kColTurbo, 2, 26.f, 0.3f, kZFx - 10);
        riderAfterimage(PZCfg::kColTurbo);
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
    ring(ch->loc, PZCfg::kColTurbo, 150.f, 0.45f);
    PZHaptics::play(PZHaptics::Kind::Medium);
}

// ---- obstacle destruction --------------------------------------------------------------------

bool PZRunDirector::obstacleAlive(PZGObstacle* o) const {
    if (!o || o->dead || !platform()) return false;
    for (int i = 0; i < platform()->obstacleSlots(); ++i) {
        if (platform()->obstacleAt(i) == o) return true;
    }
    return false;
}

void PZRunDirector::queueKill(PZGObstacle* o, float delay, Style style) {
    for (const auto& k : _kills) {
        if (k.obstacle == o) return;
    }
    _kills.push_back({o, delay, style});
}

void PZRunDirector::updateKills(float dt) {
    for (size_t i = 0; i < _kills.size();) {
        _kills[i].delay -= dt;
        if (_kills[i].delay <= 0.f) {
            PZGObstacle* o = _kills[i].obstacle;
            const Style st = _kills[i].style;
            _kills.erase(_kills.begin() + i);
            if (obstacleAlive(o)) destroyObstacle(o, st);
            continue;
        }
        ++i;
    }
}

void PZRunDirector::startWave(const Vec2& origin, float maxRadius, float speed, const Color3B& colour) {
    Wave w;
    w.origin = origin;
    w.maxRadius = maxRadius;
    w.speed = speed;
    w.colour = colour;
    w.ring = DrawNode::create();
    w.ring->setPosition(origin);
    _scene->addChild(w.ring, kZFx);
    _waves.push_back(w);
}

void PZRunDirector::updateWaves(float dt) {
    for (size_t i = 0; i < _waves.size();) {
        Wave& w = _waves[i];
        w.radius += w.speed * dt;
        const float t = std::min(1.f, w.radius / std::max(w.maxRadius, 1.f));

        // Redraw so the line thickness stays constant while the ring grows.
        w.ring->clear();
        const unsigned segs = static_cast<unsigned>(std::max(24.f, std::min(120.f, w.radius / 6.f)));
        const float a = std::pow(1.f - t, 1.1f);
        w.ring->drawCircle(Vec2::ZERO, w.radius, 0.f, segs, false, PZUI::c4f(w.colour, 0.95f * a), 7.f);
        w.ring->drawCircle(Vec2::ZERO, std::max(1.f, w.radius - 12.f), 0.f, segs, false, PZUI::c4f(Color3B(255, 255, 255), 0.55f * a), 2.5f);

        if (platform()) {
            for (int s = 0; s < platform()->obstacleSlots(); ++s) {
                PZGObstacle* o = platform()->obstacleAt(s);
                if (!o || o->dead || !o->destroyCharacter || !o->sprite) continue;
                const Rect b = o->worldBox();
                if (Vec2(b.getMidX(), b.getMidY()).distance(w.origin) <= w.radius) {
                    queueKill(o, rnd(0.f, 0.07f), Style::Explode);
                }
            }
        }

        if (w.radius >= w.maxRadius) {
            w.ring->removeFromParent();
            _waves.erase(_waves.begin() + i);
            continue;
        }
        ++i;
    }
}

void PZRunDirector::spawnGhost(PZGObstacle* o, Style style) {
    if (!o || !o->sprite) return;
    const Rect b = o->worldBox();
    const Vec2 centre(b.getMidX(), b.getMidY());
    SpriteFrame* frame = o->sprite->getSpriteFrame();
    const float sc = o->sprite->getScale() * std::max(std::abs(o->getScaleX()), 0.01f);
    const int z = kZFx - 5;

    if (frame) {
        auto* ghost = Sprite::createWithSpriteFrame(frame);
        ghost->setPosition(centre);
        ghost->setScale(sc);
        ghost->setRotation(o->sprite->getRotation());
        _scene->addChild(ghost, z);
        if (style == Style::Explode) {
            // The hit sprite bursts apart: a quick bright flash over a fading, puffing-up husk.
            auto* hot = Sprite::createWithSpriteFrame(frame);
            hot->setPosition(centre);
            hot->setScale(sc);
            hot->setBlendFunc(BlendFunc::ADDITIVE);
            _scene->addChild(hot, z + 1);
            hot->runAction(Sequence::create(Spawn::create(ScaleTo::create(0.14f, sc * 1.35f), FadeOut::create(0.14f), nullptr),
                                            RemoveSelf::create(), nullptr));
            ghost->runAction(Sequence::create(
                Spawn::create(ScaleTo::create(0.3f, sc * 1.5f), FadeOut::create(0.3f), RotateBy::create(0.3f, rnd(-40.f, 40.f)), nullptr),
                RemoveSelf::create(), nullptr));
        } else {
            // Plough-through: the obstacle is flung forward and up, tumbling.
            const float dir = rnd(0.f, 1.f) > 0.5f ? 1.f : -1.f;
            ghost->runAction(Sequence::create(
                Spawn::create(JumpBy::create(0.8f, Vec2(rnd(380.f, 520.f), rnd(40.f, 120.f)), rnd(180.f, 260.f), 1),
                              RotateBy::create(0.8f, dir * rnd(540.f, 900.f)), ScaleTo::create(0.8f, sc * 0.55f),
                              Sequence::create(DelayTime::create(0.45f), FadeOut::create(0.35f), nullptr), nullptr),
                RemoveSelf::create(), nullptr));
        }
    }

    // Debris shards.
    const Color3B palette[4] = {PZCfg::kColAccent2, PZCfg::kColAccent, Color3B(120, 90, 60), Color3B(90, 96, 104)};
    const int shards = style == Style::Explode ? 10 : 6;
    for (int i = 0; i < shards; ++i) {
        auto* s = DrawNode::create();
        const float sz = rnd(5.f, 12.f);
        Vec2 tri[3] = {Vec2(-sz, -sz * 0.6f), Vec2(sz, -sz * 0.3f), Vec2(rnd(-sz, sz), sz)};
        s->drawSolidPoly(tri, 3, PZUI::c4f(palette[i % 4]), 1.f, PZUI::c4f(Color3B(30, 18, 8), 0.9f), true);
        s->setPosition(centre + Vec2(rnd(-b.size.width * 0.3f, b.size.width * 0.3f), rnd(-b.size.height * 0.3f, b.size.height * 0.3f)));
        _scene->addChild(s, z + 2);
        const float dur = rnd(0.5f, 0.85f);
        s->runAction(Sequence::create(
            Spawn::create(JumpBy::create(dur, Vec2(rnd(-170.f, 220.f), rnd(-20.f, 40.f)), rnd(70.f, 170.f), 1),
                          RotateBy::create(dur, rnd(-720.f, 720.f)), Sequence::create(DelayTime::create(dur * 0.55f), FadeOut::create(dur * 0.45f), nullptr), nullptr),
            RemoveSelf::create(), nullptr));
    }

    const Color3B ringCol = style == Style::Explode ? PZCfg::kColAccent2 : PZCfg::kColTurbo;
    ring(centre, ringCol, 95.f, 0.35f);
    PZUI::burst(_scene, centre, PZCfg::kColAccent, 10, 90.f, 0.5f, z + 3);
    dustPuff(centre, 5, 60.f);
}

void PZRunDirector::destroyObstacle(PZGObstacle* o, Style style) {
    if (!o || o->dead) return;
    spawnGhost(o, style);
    o->dead = true;
    o->deleteReady = true;
    ++_stats.smashed;

    const Rect b = o->worldBox();
    if (PZCfg::kSmashCoins > 0) {
        _scene->addBonusCoins(PZCfg::kSmashCoins);
        _stats.coins += PZCfg::kSmashCoins;
    }
    PZUI::floatText(_scene, PZCfg::kStrSmash, Vec2(b.getMidX(), b.getMaxY() + 18.f), PZCfg::kColTurbo, 10.f, kZFx);
    PZUI::shake(_scene, Vec2::ZERO, 4.f, 0.2f);
    PZHaptics::play(PZHaptics::Kind::Heavy);
    if (!frozen()) freezeFor(PZCfg::kHitStop);
}

bool PZRunDirector::mercyAvailable() const {
    return PZCfg::kEnableMercy && _mercyUsed < PZCfg::kMercySaves && PZProgress::shared()->bestScore() < PZCfg::kMercyBestBelow;
}

bool PZRunDirector::onObstacleHit(PZGObstacle* o) {
    if (!o) return false;
    if (turboActive()) {
        destroyObstacle(o, Style::Launch);
        return true;
    }
    if (_graceT > 0.f) {
        destroyObstacle(o, Style::Explode);
        return true;
    }
    if (_shieldT > 0.f) {
        _shieldT = 0.f;
        _graceT = PZCfg::kHitGrace;
        destroyObstacle(o, Style::Explode);
        startWave(riderPos(), PZCfg::kShieldBreakRadius, 1600.f, PZCfg::kColShield);
        PZUI::floatText(_scene, PZCfg::kStrShieldBreak, riderPos() + Vec2(0, 56), PZCfg::kColShield, 12.f, kZFx);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 16, 100.f, 0.55f, kZFx);
        flash(PZCfg::kColShield, 0.3f, 0.3f);
        PZUI::shake(_scene, Vec2::ZERO, 9.f, 0.3f);
        PZHaptics::play(PZHaptics::Kind::Heavy);
        return true;
    }
    if (mercyAvailable()) {
        ++_mercyUsed;
        _graceT = PZCfg::kHitGrace;
        destroyObstacle(o, Style::Explode);
        startWave(riderPos(), PZCfg::kShieldBreakRadius * 0.7f, 1500.f, PZCfg::kColGood);
        PZUI::banner(_scene, PZCfg::kStrSaved, "", PZCfg::kColGood, Director::getInstance()->getWinSize().height * 0.6f, 0.7f);
        flash(PZCfg::kColGood, 0.25f, 0.3f);
        PZHaptics::play(PZHaptics::Kind::Warning);
        return true;
    }
    return false;
}

// ---- visuals ---------------------------------------------------------------------------------

void PZRunDirector::flash(const Color3B& colour, float alpha, float duration) {
    if (!PZCfg::kEnableFx) return;
    _flash->stopAllActions();
    _flash->setColor(colour);
    _flash->setOpacity(static_cast<uint8_t>(std::min(1.f, alpha) * 255.f));
    _flash->runAction(FadeOut::create(duration));
}

void PZRunDirector::ring(const Vec2& at, const Color3B& colour, float radius, float duration) {
    if (!PZCfg::kEnableFx) return;
    auto* d = DrawNode::create();
    d->drawCircle(Vec2::ZERO, 30.f, 0.f, 36, false, PZUI::c4f(colour, 0.95f), 4.f);
    d->setPosition(at);
    d->setScale(0.3f);
    _scene->addChild(d, kZFx - 2);
    d->runAction(Sequence::create(Spawn::create(EaseOut::create(ScaleTo::create(duration, radius / 30.f), 2.f), FadeOut::create(duration), nullptr),
                                  RemoveSelf::create(), nullptr));
}

void PZRunDirector::dustPuff(const Vec2& at, int count, float spread) {
    if (!PZCfg::kEnableFx || PZCfg::kDustPuffs <= 0) return;
    for (int i = 0; i < count; ++i) {
        auto* d = DrawNode::create();
        d->drawDot(Vec2::ZERO, rnd(3.f, 7.f), Color4F(0.86f, 0.78f, 0.66f, 0.55f));
        d->setPosition(at + Vec2(rnd(-8.f, 8.f), rnd(-3.f, 3.f)));
        _scene->addChild(d, kZAura + 2);
        const float dur = rnd(0.35f, 0.6f);
        d->runAction(Sequence::create(Spawn::create(MoveBy::create(dur, Vec2(rnd(-spread, spread * 0.4f), rnd(10.f, 34.f))),
                                                    ScaleTo::create(dur, rnd(1.6f, 2.4f)), FadeOut::create(dur), nullptr),
                                      RemoveSelf::create(), nullptr));
    }
}

void PZRunDirector::coinFly(const Vec2& from) {
    if (_flying >= 10 || !_scene->coins_label) return;
    auto* icon = PZUI::coinIcon(24.f);
    icon->setPosition(from);
    _scene->addChild(icon, kZFx + 5);
    ++_flying;
    const Vec2 to = _scene->coins_label->getPosition() + Vec2(-26.f, 0.f);
    icon->runAction(Sequence::create(
        Spawn::create(EaseIn::create(MoveTo::create(0.5f, to), 1.8f), ScaleTo::create(0.5f, 0.55f), nullptr),
        CallFunc::create([this]() {
            --_flying;
            if (_scene && _scene->coins_label) {
                _scene->coins_label->stopActionByTag(77);
                auto* a = Sequence::create(ScaleBy::create(0.06f, 1.25f), ScaleBy::create(0.09f, 1.f / 1.25f), nullptr);
                a->setTag(77);
                _scene->coins_label->runAction(a);
            }
        }),
        RemoveSelf::create(), nullptr));
}

void PZRunDirector::riderAfterimage(const Color3B& tint) {
    if (!_scene->character || !_scene->character->s_body) return;
    Sprite* body = _scene->character->s_body;
    SpriteFrame* frame = body->getSpriteFrame();
    if (!frame) return;
    auto* g = Sprite::createWithSpriteFrame(frame);
    g->setPosition(body->getPosition());
    g->setScaleX(body->getScaleX());
    g->setScaleY(body->getScaleY());
    g->setRotation(body->getRotation());
    g->setColor(tint);
    g->setOpacity(150);
    g->setBlendFunc(BlendFunc::ADDITIVE);
    _scene->addChild(g, kZAura - 1);
    g->runAction(Sequence::create(Spawn::create(FadeOut::create(0.3f), MoveBy::create(0.3f, Vec2(-110.f, 0.f)), nullptr), RemoveSelf::create(), nullptr));
}

void PZRunDirector::updateAuras(float dt) {
    const Vec2 rp = riderPos();
    const float pulse = 1.f + 0.05f * std::sin(_auraClock * 8.f);
    _shieldAura->setVisible(_shieldT > 0.f);
    if (_shieldT > 0.f) {
        _shieldAura->setPosition(rp);
        _shieldAura->setScale(pulse);
        _shieldAura->setOpacity(_shieldT < 2.f && std::fmod(_auraClock, 0.3f) < 0.15f ? 90 : 255);   // blink when about to expire
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

void PZRunDirector::updateFx(float dt, float scroll) {
    if (!PZCfg::kEnableFx || !_scene->character) return;
    PZGCharacter* ch = _scene->character;
    const Size win = Director::getInstance()->getWinSize();

    // Ambient motes.
    for (DrawNode* m : _motes) {
        m->setPosition(m->getPosition() + Vec2(-(scroll * 0.35f + 18.f * dt), 7.f * dt));
        if (m->getPositionX() < -10.f || m->getPositionY() > win.height + 10.f) {
            m->setPosition(Vec2(win.width + rnd(0.f, 60.f), rnd(40.f, win.height - 40.f)));
        }
    }

    // Jump / landing events, detected from the rider's grounded counter.
    const int jc = ch->jump_counter;
    const float groundY = riderPos().y - (_haveOffset ? _riderOffset : 40.f);
    if (!turboActive()) {
        if (_prevJump == 0 && jc != 0 && ch->p_body && ch->p_body->GetLinearVelocity().y > 0.f) {
            dustPuff(Vec2(riderPos().x - 30.f, groundY + 6.f), 4, 70.f);
            PZHaptics::play(PZHaptics::Kind::Light);
            _airTime = 0.f;
        } else if (_prevJump != 0 && jc == 0) {
            const float strength = std::min(1.f, _airTime / 0.7f);
            dustPuff(Vec2(riderPos().x, groundY + 6.f), 4 + static_cast<int>(strength * 5.f), 110.f);
            if (_airTime > 0.25f) {
                ring(Vec2(riderPos().x, groundY + 4.f), Color3B(220, 205, 180), 55.f + 40.f * strength, 0.3f);
                PZUI::shake(_scene, Vec2::ZERO, 1.5f + 2.5f * strength, 0.12f);
                PZHaptics::play(strength > 0.55f ? PZHaptics::Kind::Medium : PZHaptics::Kind::Light);
            }
        }
        _airTime = (jc != 0) ? _airTime + dt : 0.f;
    }
    _prevJump = jc;

    // Running dust at the rear wheel while grounded.
    if (jc == 0 && !turboActive()) {
        _dustClock += dt;
        if (_dustClock > 0.07f) {
            _dustClock = 0.f;
            dustPuff(Vec2(riderPos().x - 44.f, groundY + 4.f), 1, 40.f);
        }
    }
}

// ---- mission tracker -------------------------------------------------------------------------

void PZRunDirector::updateMissionTracker(float dt) {
    if (!_tracker) return;
    if (!PZCfg::kEnableMissions) {
        _tracker->setVisible(false);
        return;
    }
    _trackerClock += dt;
    if (_trackerClock < 0.25f) return;
    _trackerClock = 0.f;

    auto* pg = PZProgress::shared();
    _stats.distance = static_cast<int>(_scene->gf_distance);
    _stats.stage = _stage;

    // Celebrate any mission completed live (reward is claimed from the MISSIONS screen).
    int pick = -1;
    for (int i = 0; i < pg->missionCount(); ++i) {
        const auto& m = pg->mission(i);
        const bool doneAtStart = m.progress >= pg->missionTemplate(i).target;
        const int live = pg->livePreview(i, _stats);
        if (!doneAtStart && !_trackerDone[i] && live >= pg->missionTemplate(i).target) {
            _trackerDone[i] = true;
            PZUI::banner(_scene, PZCfg::kStrMissionToast, pg->missionText(i), PZCfg::kColGood,
                         Director::getInstance()->getWinSize().height * 0.5f, 1.2f);
            PZHaptics::play(PZHaptics::Kind::Success);
            playPickupSound();
        }
        if (pick < 0 && !m.claimed && !doneAtStart && !_trackerDone[i]) pick = i;
    }
    if (pick < 0) {
        _tracker->setVisible(false);
        return;
    }
    const auto& t = pg->missionTemplate(pick);
    const int live = std::min(pg->livePreview(pick, _stats), t.target);
    if (pick != _trackerSlot) {
        _trackerSlot = pick;
        std::string txt = pg->missionText(pick);
        _trackerText->setString(txt);
        const float maxW = 232.f;
        _trackerText->setScale(1.f);
        if (_trackerText->getContentSize().width > maxW) _trackerText->setScale(maxW / _trackerText->getContentSize().width);
    }
    PZUI::redrawBar(_trackerBar, 150.f, 9.f, static_cast<float>(live) / t.target, PZCfg::kColAccent2);
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%d/%d", live, t.target);
    _trackerCount->setString(buf);
    _tracker->setVisible(true);
}

// ---- death / revive --------------------------------------------------------------------------

bool PZRunDirector::onDeath() {
    // An active shield saves the rider from one fall (hits are already absorbed in onObstacleHit).
    if (_shieldT > 0.f) {
        _shieldT = 0.f;
        rescueRider();
        PZUI::floatText(_scene, PZCfg::kStrShieldBreak, riderPos() + Vec2(0, 56), PZCfg::kColShield, 12.f, kZFx);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 14, 90.f, 0.5f, kZFx);
        flash(PZCfg::kColShield, 0.3f, 0.3f);
        PZHaptics::play(PZHaptics::Kind::Warning);
        return true;
    }
    // Beginners get a free second chance so early runs don't end on the first mistake.
    if (mercyAvailable()) {
        ++_mercyUsed;
        rescueRider();
        PZUI::banner(_scene, PZCfg::kStrSaved, "", PZCfg::kColGood, Director::getInstance()->getWinSize().height * 0.6f, 0.8f);
        PZUI::burst(_scene, riderPos(), PZCfg::kColGood, 14, 90.f, 0.5f, kZFx);
        flash(PZCfg::kColGood, 0.3f, 0.35f);
        PZHaptics::play(PZHaptics::Kind::Warning);
        return true;
    }

    flash(PZCfg::kColBad, 0.4f, 0.5f);
    PZHaptics::play(PZHaptics::Kind::Error);
    if (!PZCfg::kEnableRevive || _reviveOffered) return false;
    auto* sc = PZSettingsController::shared();
    const bool video = PZCfg::kEnableReviveVideo && PZ::AdsController::shared()->isRewardedAvailable();
    if (sc->coins < PZCfg::kReviveCost && !video) return false;     // nothing to offer: go straight to game over
    _reviveOffered = true;

    _scene->gameOver = true;     // freeze the world while the offer is up
    PZUI::shake(_scene, Vec2::ZERO, 10.f, 0.4f);

    // A short beat so the player sees what killed them, then the offer.
    runAction(Sequence::create(DelayTime::create(0.45f), CallFunc::create([this, video]() {
        PZPopups::showRevive(_scene, PZCfg::kReviveCost, PZSettingsController::shared()->coins, video,
                             [this]() { reviveAccepted(true); }, [this]() { reviveFromVideo(); }, [this]() { reviveDeclined(); });
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

void PZRunDirector::reviveFromVideo() {
    // The rewarded ad takes over the screen; revive only if the player watched it to the end.
    PZ::AdsController::shared()->showRewardedWithCallback([this](bool earned) {
        if (earned) {
            reviveAccepted(false);
        } else {
            reviveDeclined();
        }
    });
}

void PZRunDirector::reviveAccepted(bool paidWithCoins) {
    auto* sc = PZSettingsController::shared();
    if (paidWithCoins) {
        sc->coins = std::max(0, static_cast<int>(sc->coins) - PZCfg::kReviveCost);
        sc->save();
        _scene->gf_coins = sc->coins;
        if (_scene->coins_label) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(_scene->gf_coins));
            _scene->coins_label->setString(buf);
        }
    }
    _stats.revived = true;

    rescueRider();
    _shieldT = 0.f;
    _scene->gameOver = false;
    _scene->gamePause = false;

    // A short turbo sprint carries the rider clear of whatever just killed them.
    if (PZCfg::kReviveTurbo > 0.f && PZCfg::kEnablePowerUps) {
        collect(PowerUp::Turbo, riderPos(), PZCfg::kReviveTurbo);
    }

    const Size win = Director::getInstance()->getWinSize();
    PZUI::banner(_scene, PZCfg::kStrGo, "", PZCfg::kColGood, win.height * 0.6f, 0.5f);
    PZUI::burst(_scene, riderPos(), PZCfg::kColGood, 14, 90.f, 0.5f, kZFx);
    flash(Color3B(255, 255, 255), 0.4f, 0.35f);
    PZHaptics::play(PZHaptics::Kind::Success);
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
    for (auto& r : _rows) r.root->setVisible(false);
    if (_tracker) _tracker->setVisible(false);
    _shieldAura->setVisible(false);
    _magnetAura->setVisible(false);
    _turboAura->setVisible(false);
    _comboLabel->setVisible(false);
    _magnetT = _shieldT = _turboT = 0.f;
    const PZRunResult result = PZProgress::shared()->commitRun(_stats);
    refreshHud();
    PZPopups::attachResults(gameOverLayer, distanceLabel, _stats, result);
}
