//
//  PZRunDirector.cpp
//
#include "PZRunDirector.h"
#include "PZUI.h"
#include "PZPopups.h"
#include "PZLayout.h"
#include "PZHaptics.h"
#include "PZAudio.h"

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
constexpr int kIntroDelayTag = 7740;
constexpr int kIntroRideTag = 7741;
constexpr int kCountdownTag = 7742;
constexpr int kDeathShowTag = 7743;
constexpr int kCameraActionTag = 7732;
constexpr int kCameraSnapTag = 7744;
constexpr int kCrowdCueTag = 7745;

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
    _jetpackAura = DrawNode::create();
    for (DrawNode* a : {_shieldAura, _magnetAura, _turboAura, _jetpackAura}) {
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
    _jetpackAura->drawSolidCircle(Vec2::ZERO, 62.f, 0.f, 28, PZUI::c4f(PZCfg::kColJetpack, 0.13f));
    _jetpackAura->drawCircle(Vec2::ZERO, 76.f, 0.f, 28, false, PZUI::c4f(PZCfg::kColJetpack, 0.55f), 2.f);
    _jetpackAura->drawCircle(Vec2::ZERO, 68.f, 0.f, 28, false, PZUI::c4f(Color3B(255, 255, 255), 0.30f), 1.2f);

    // Power-up chips along the top of the screen: the orb icon, the power-up name and the seconds left, with a
    // framed timer bar underneath. The name makes the chip readable at a glance instead of a letter inside a disc.
    for (int i = 0; i < kChipCount; ++i) {
        HudRow& r = _rows[i];
        r.root = Node::create();
        r.root->setVisible(false);
        _hud->addChild(r.root);
        r.icon = makeOrbVisual(chipKind(i), 0.58f);
        r.icon->setPosition(Vec2(-56.f, 7.f));
        r.root->addChild(r.icon);
        r.name = PZUI::heading("", 9.f, PZCfg::kColText);
        r.name->setAnchorPoint(Vec2(0.f, 0.5f));
        r.name->setPosition(Vec2(-32.f, 11.f));
        r.root->addChild(r.name);
        r.time = PZUI::heading("", 9.f, PZCfg::kColText);
        r.time->setAnchorPoint(Vec2(1.f, 0.5f));
        r.time->setPosition(Vec2(62.f, 11.f));
        r.root->addChild(r.time);
        r.bar = PZUI::bar(118.f, 7.f, 1.f, PZCfg::kColAccent);
        r.bar->setPosition(Vec2(3.f, -13.f));
        r.root->addChild(r.bar);
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

    // Equipped gadget strip (small bronze discs under the coin counter).
    _gadgetStrip = Node::create();
    _gadgetStrip->setVisible(false);
    _hud->addChild(_gadgetStrip);

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
    for (int i = 0; i < kChipCount; ++i) {
        _rows[i].root->setPosition(Vec2(safe.getMidX(), safe.getMaxY() - 126.f));
    }
    _comboLabel->setPosition(Vec2(safe.getMinX() + 24.f, safe.getMaxY() - 162.f));
    if (_tracker) _tracker->setPosition(Vec2(safe.getMaxX() - 130.f, safe.getMaxY() - 150.f));
    if (_gadgetStrip) _gadgetStrip->setPosition(Vec2(safe.getMinX() + 24.f, safe.getMaxY() - 122.f));
}

void PZRunDirector::onLoaded() {
    if (!_scene) return;
    // Best-score read-out tucked under the live distance counter (same gold arcade style as the HUD).
    if (_bestLabel) _bestLabel->removeFromParent();
    _bestLabel = PZUI::heading("", 10.f, PZCfg::kColDim);
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
    refreshGadgetStrip();
}

void PZRunDirector::refreshHud() {
    if (_bestLabel) {
        char buf[48];
        std::snprintf(buf, sizeof(buf), PZCfg::kStrBest, PZProgress::shared()->bestScore());
        _bestLabel->setString(buf);
    }
}

void PZRunDirector::refreshGadgetStrip() {
    if (!_gadgetStrip) return;
    _gadgetStrip->removeAllChildren();
    if (!PZCfg::kEnableGadgets) {
        _gadgetStrip->setVisible(false);
        return;
    }
    // One round icon per equipped gadget: bright = charge used this run, dim = out of charges / already spent.
    auto* pg = PZProgress::shared();
    const int count = std::min(pg->selectedGadgetCount(), std::max(0, PZCfg::kGadgetSlots));
    int shown = 0;
    for (int i = 0; i < count; ++i) {
        const std::string id = pg->gadgetAt(i);
        const int idx = pg->gadgetPoolIndex(id);
        if (idx < 0) continue;
        const auto& def = PZCfg::kGadgetPool[idx];
        const bool active = std::find(_activeGadgets.begin(), _activeGadgets.end(), id) != _activeGadgets.end();
        const bool spent = active && id == "helmet" && _helmetSaves <= 0;
        const bool dim = !active || spent;
        const float x = shown * 34.f;
        auto* disc = DrawNode::create();
        disc->drawSolidCircle(Vec2::ZERO, 15.f, 0.f, 22, PZUI::c4f(PZCfg::kColPanel, dim ? 0.7f : 0.94f));
        disc->drawCircle(Vec2::ZERO, 15.f, 0.f, 22, false, PZUI::c4f(dim ? Color3B(90, 92, 104) : PZCfg::kColAccent), 2.f);
        disc->setPosition(Vec2(x, 0.f));
        _gadgetStrip->addChild(disc);
        auto* icon = PZUI::gadgetIcon(id, 20.f, def.icon, dim);
        icon->setPosition(Vec2(x, 0.f));
        _gadgetStrip->addChild(icon);
        ++shown;
    }
    _gadgetStrip->setVisible(shown > 0);
}

void PZRunDirector::clearRunVisuals() {
    if (_cinematicFrame) closeCinematicFrame(0.08f);
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
    _jetpackAura->setVisible(false);
    for (auto& r : _rows) r.root->setVisible(false);
    if (_gunArt) _gunArt->setVisible(false);
    if (_rocketArt) _rocketArt->setVisible(false);
    _comboLabel->setVisible(false);
    _tint->stopAllActions();
    _tint->setOpacity(0);
    _flash->stopAllActions();
    _flash->setOpacity(0);
}

// ---- run lifecycle ---------------------------------------------------------------------------

void PZRunDirector::startRun() {
    for (int tag : {kIntroDelayTag, kIntroRideTag, kCountdownTag, kDeathShowTag, kCrowdCueTag}) stopActionByTag(tag);
    if (_cinematicFrame) {
        _cinematicFrame->removeFromParent();
        _cinematicFrame = nullptr;
    }
    _cinematic = false;
    _readyCountdown = false;
    _introActive = false;
    _scene->stopActionByTag(kCameraActionTag);
    _scene->stopActionByTag(kCameraSnapTag);
    _camScale = 1.f;
    _camPos = Vec2::ZERO;
    _scene->setScale(1.f);
    _scene->setPosition(Vec2::ZERO);
    setHudVisible(true);
    clearRunVisuals();
    PZProgress::shared()->refreshMissions();
    { static bool audioReady = false; if (!audioReady) { audioReady = true; PZAudio::preload(); } }

    _time = 0.f;
    _stage = 1;
    _boost = 1.f;
    _magnetT = _shieldT = _turboT = _graceT = 0.f;
    _combo = 0;
    _comboT = 0.f;
    _reviveOffered = false;
    _committed = false;
    _mercyUsed = 0;
    _readyCountdown = false;
    _reviveWait = false;
    _deathPending = false;
    _cinematic = false;
    _lastHitStop = -10.f;
    _haveOffset = false;
    _haveGroundY = false;
    _prevJump = 0;
    _airTime = 0.f;
    _flying = 0;
    _freezeUntil = std::chrono::steady_clock::time_point{};
    _stats = PZRunStats();
    _nextOrbAt = PZCfg::kPowerUpFirstDelay;
    _helmetSaves = 0;
    _trackerSlot = -1;
    _trackerClock = 0.f;
    for (bool& d : _trackerDone) d = false;
    if (PZCfg::kEnableRevive && PZCfg::kEnableReviveVideo) PZ::AdsController::shared()->requestRewarded();   // warm a rewarded ad

    if (_scene && _scene->character && _scene->character->p_body) {
        _scene->character->p_body->SetGravityScale(1.f);
    }
    if (PZCfg::kEnableStartGrace) _graceT = PZCfg::kStartGrace;
    applyGadgets();
    refreshGadgetStrip();
    refreshHud();

    _introActive = false;
    _introTip1Visible = _scene->tip1 && _scene->tip1->isVisible();
    _introTip2Visible = _scene->tip2 && _scene->tip2->isVisible();
    beginIntro();
    PZHaptics::play(PZHaptics::Kind::Selection);
}

// ------------------------------------------------------------------------------------------------
// Start-of-run show: hold the world, ride the bike in from the left while the camera is pushed in, then
// "GET READY ... 3 2 1 GO!" while the HUD drops in, and the crowd cheers as the run starts.
// ------------------------------------------------------------------------------------------------
void PZRunDirector::beginIntro() {
    if (!_scene || !_scene->character || !_scene->character->p_body) return;
    if (!PZCfg::kEnableCinematics) {   // reskin switch: no start show, just the classic GO banner
        const float winH = Director::getInstance()->getWinSize().height;
        PZUI::banner(_scene, PZCfg::kStrGo, PZCfg::kStrStartHint, PZCfg::kColGood, winH * 0.62f, 1.0f);
        return;
    }
    _introActive = true;
    // The platform generator fills the world in its first update, so start the show one beat later.
    auto* intro = Sequence::create(DelayTime::create(0.30f),
                                   CallFunc::create([this]() { startIntroNow(); }), nullptr);
    intro->setTag(kIntroDelayTag);
    runAction(intro);
}

void PZRunDirector::openCinematicFrame() {
    Node* host = _scene && _scene->getParent() ? _scene->getParent() : _scene;
    if (!host) return;
    if (_cinematicFrame) _cinematicFrame->removeFromParent();

    const Size win = Director::getInstance()->getWinSize();
    const float barHeight = std::max(28.f, win.height * 0.075f);
    auto* frame = Node::create();
    _cinematicFrame = frame;
    host->addChild(frame, 5000);

    auto makeBar = [frame, win, barHeight](float y, float fromY, float toY) {
        auto* bar = LayerColor::create(Color4B(7, 10, 20, 236), win.width, barHeight);
        bar->setPosition(Vec2(0.f, fromY));
        frame->addChild(bar);
        auto* edge = LayerColor::create(Color4B(255, 188, 74, 170), win.width, 2.f);
        edge->setPosition(Vec2(0.f, y < win.height * 0.5f ? barHeight - 2.f : 0.f));
        bar->addChild(edge);
        bar->runAction(EaseOut::create(MoveTo::create(0.28f, Vec2(0.f, toY)), 2.f));
    };
    makeBar(0.f, -barHeight, 0.f);
    makeBar(win.height, win.height, win.height - barHeight);
}

void PZRunDirector::closeCinematicFrame(float duration) {
    Node* frame = _cinematicFrame;
    _cinematicFrame = nullptr;
    if (!frame) return;
    const Size win = Director::getInstance()->getWinSize();
    const float barHeight = std::max(28.f, win.height * 0.075f);
    const auto& children = frame->getChildren();
    for (auto* child : children) {
        if (!child) continue;
        const float targetY = child->getPositionY() < win.height * 0.5f ? -barHeight : win.height;
        child->runAction(Sequence::create(
            Spawn::create(EaseIn::create(MoveTo::create(std::max(0.08f, duration), Vec2(0.f, targetY)), 2.f),
                          FadeOut::create(std::max(0.08f, duration)), nullptr),
            nullptr));
    }
    frame->runAction(Sequence::create(DelayTime::create(std::max(0.08f, duration) + 0.03f),
                                      RemoveSelf::create(), nullptr));
}

void PZRunDirector::startIntroNow() {
    if (!_introActive || !_scene || !_scene->character || !_scene->character->p_body) return;
    PZGCharacter* ch = _scene->character;
    const Size win = Director::getInstance()->getWinSize();

    // The bike enters on the first real platform (never at the bottom of a pit).
    float gy = win.height * 0.30f;
    if (!nearestSurface(gy)) gy = win.height * 0.30f;
    const float offset = _haveOffset ? _riderOffset : 30.f;
    const float restX = win.width / 5.f;
    const float startX = -win.width * 0.12f;

    _cinematic = true;                       // no taps, no kicks while the show runs
    freezeFor(PZCfg::kIntroEntrySeconds + PZCfg::kReadyLeadSeconds +
              PZCfg::kReadySeconds * 3.f + 0.9f);   // released explicitly at GO
    openCinematicFrame();
    setHudVisible(false);
    if (_scene->tip1) _scene->tip1->setVisible(false);
    if (_scene->tip2) _scene->tip2->setVisible(false);

    ch->p_body->SetGravityScale(0.f);
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
    ch->setLoc(Vec2(startX, gy + offset + 8.f));
    ch->jump_counter = 1;

    // Camera pushed in on the left where the bike comes to a stop.
    const Vec2 rest(restX, gy + offset + 24.f);
    const Vec2 anchor(win.width * 0.32f, win.height * 0.52f);
    cameraTo(rest, 1.32f, 0.45f, &anchor);

    auto* slide = ActionFloat::create(PZCfg::kIntroEntrySeconds, 0.f, 1.f, [this, startX, restX, gy, offset](float t) {
        if (!_scene || !_scene->character) return;
        const float e = 1.f - std::pow(1.f - t, 3.f);
        const float hop = std::sin(t * static_cast<float>(M_PI)) * 10.f;
        const float settle = t > 0.82f ? std::sin((t - 0.82f) * static_cast<float>(M_PI) / 0.18f) * 2.5f : 0.f;
        _scene->character->setLoc(Vec2(startX + (restX - startX) * e, gy + offset + 8.f + hop + settle));
        _scene->character->update(0.f);   // the world is held: the sprite only follows the body through this
    });
    auto* entrance = Sequence::create(slide,
        CallFunc::create([this, offset, gy]() {
            if (!_scene || !_scene->character || !_scene->character->p_body) return;
            PZGCharacter* ch = _scene->character;
            // Park on the surface the camera is already looking at (never at the bottom of a pit under it).
            ch->setLoc(Vec2(ch->loc.x, gy + offset + 6.f));
            ch->update(0.f);
            ch->p_body->SetGravityScale(1.f);
            ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
            dustPuff(ch->loc + Vec2(-30.f, -10.f), 5, 60.f);
            PZUI::burst(_scene, ch->loc, PZCfg::kColAccent2, 8, 50.f, 0.4f, kZFx);
            PZHaptics::play(PZHaptics::Kind::Light);
        }),
        CallFunc::create([this]() { beginReadyCountdown(false); }), nullptr);
    entrance->setTag(kIntroRideTag);
    runAction(entrance);
}

// HUD read-outs drop in one after the other instead of all being there at frame one.
void PZRunDirector::animateHudIn() {
    if (!_scene) return;
    int i = 0;
    auto in = [&i](Node* n) {
        if (!n || !n->isVisible()) return;
        PZUI::enterIn(n, 0.05f * static_cast<float>(i++), false, 22.f);
    };
    in(_scene->distance_label);
    in(_scene->coins_label);
    in(_bestLabel);
    in(_gadgetStrip);
    // The plist buttons (pause, IAP) arrive last.
    if (Menu* menu = dynamic_cast<Menu*>(_scene->getChildByTag(kBaseMenuItemTag))) {
        PZUI::stagger(menu, 0.05f, 0.05f + 0.05f * static_cast<float>(i));
    }
}

void PZRunDirector::applyGadgets() {
    _activeGadgets.clear();
    stopAllActionsByTag(kGadgetStartTag);
    if (!PZCfg::kEnableGadgets || !_scene) return;
    auto* pg = PZProgress::shared();
    const int count = std::min(pg->selectedGadgetCount(), std::max(0, PZCfg::kGadgetSlots));
    float delay = 0.6f;   // timed rides start once the scene has settled, one after the other
    for (int i = 0; i < count; ++i) {
        const std::string id = pg->gadgetAt(i);
        const int idx = pg->gadgetPoolIndex(id);
        if (idx < 0 || !pg->gadgetUnlocked(idx)) continue;
        if (!pg->consumeGadgetStock(id)) continue;      // out of charges: buy more in the shop
        _activeGadgets.push_back(id);
        const float value = PZCfg::kGadgetPool[idx].value;
        if (id == "magnet") {
            _magnetT = _magnetMax = value > 0.f ? value : PZCfg::kGadgetMagnetSeconds;
        } else if (id == "shield") {
            _shieldT = _shieldMax = value > 0.f ? value : PZCfg::kGadgetShieldSeconds;
        } else if (id == "headstart") {
            const int metres = static_cast<int>(value > 0.f ? value : PZCfg::kGadgetHeadStartM);
            _scene->gf_distance += metres;
            char buf[48];
            std::snprintf(buf, sizeof(buf), PZCfg::kStrHeadStart, metres);
            PZUI::floatText(_scene, buf, Vec2(PZLayout::safeRect().getMidX(),
                                             Director::getInstance()->getWinSize().height * 0.52f),
                            PZCfg::kColAccent2, 12.f, kZFx);
            PZUI::burst(_scene, riderPos(), PZCfg::kColAccent2, 12, 80.f, 0.5f, kZFx);
        } else if (id == "helmet") {
            _helmetSaves = std::max(1, static_cast<int>(value > 0.f ? value : PZCfg::kGadgetHelmetSaves));
        } else {
            PowerUp kind = PowerUp::Turbo;
            if (id == "jetpack") kind = PowerUp::Jetpack;
            else if (id == "rocket") kind = PowerUp::Rocket;
            else if (id == "gun") kind = PowerUp::Gun;
            else if (id != "turbo") continue;
            const float seconds = value > 0.f ? value : 5.f;
            auto* seq = Sequence::create(DelayTime::create(delay), CallFunc::create([this, kind, seconds]() {
                if (_scene && !_scene->gameOver && !_deathPending) collect(kind, riderPos(), seconds);
            }), nullptr);
            seq->setTag(kGadgetStartTag);
            runAction(seq);
            delay += seconds + 0.4f;
        }
    }
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
    updateJetpackRide(dt);
    updateRocketRide(dt);
    updateGun(dt);
    updateReviveWait(dt);
    updateWaves(dt);
    updateKills(dt);
    updateNearMiss();
    updateAuras(dt);
    updateSpeedLines(dt);
    updateFx(dt, scroll);
    updateMissionTracker(dt);

    if (_graceT > 0.f) _graceT -= dt;

    { float g = 0.f; nearestSurface(g); }    // keeps _lastGroundY fresh for powered rides started over a gap

    // Remember where the rider sits on the surface so turbo / revive can put it back.
    if (!turboActive() && _scene->character->jump_counter == 0) {
        float gy = 0.f;
        if (platform() && platform()->groundYAt(riderPos().x, gy)) {
            // Only trust it while the rider really rests on that surface (not while falling past a platform that
            // overlaps its x range, which gave offsets like -190 and parked revived riders under the deck).
            const float off = riderPos().y - gy;
            const float k = kDeviceScale();
            const b2Body* body = _scene->character->p_body;
            const bool still = body && std::abs(body->GetLinearVelocity().y) < 1.0f;
            if (still && off > 8.f * k && off < 120.f * k) {
                _riderOffset = off;
                _haveOffset = true;
            }
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
    shakeScene(5.f, 0.3f);
    flash(PZCfg::kColAccent, 0.35f, 0.4f);
    ring(riderPos(), PZCfg::kColAccent, 260.f, 0.6f);
    PZUI::glitter(_scene, riderPos(), PZCfg::kColAccent, 16, 150.f, 0.9f, kZFx + 5);
    cameraPunch(PZCfg::kPunchZoom, 0.14f, 0.32f);
    PZHaptics::play(PZHaptics::Kind::Success);
    _scene->addBonusCoins(PZCfg::kStageCoinBonus);
    _stats.coins += PZCfg::kStageCoinBonus;
    playPickupSound();
    PZAudio::play("fx_stage");

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
    if (CCRANDOM_0_1() < 0.35f) PZUI::glitter(_scene, at, PZCfg::kColAccent, 2, 24.f, 0.5f, kZFx + 4);
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
        PZAudio::play("fx_combo_bonus");
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
            PZAudio::play("fx_nearmiss");
            PZAudio::say("vo_close_call", 0);
        }
    }
}

// ---- power-ups -------------------------------------------------------------------------------

Color3B PZRunDirector::colorFor(PowerUp kind) {
    switch (kind) {
    case PowerUp::Magnet: return PZCfg::kColMagnet;
    case PowerUp::Shield: return PZCfg::kColShield;
    case PowerUp::Blast:  return PZCfg::kColBlast;
    case PowerUp::Jetpack:return PZCfg::kColJetpack;
    case PowerUp::Rocket: return PZCfg::kColRocket;
    case PowerUp::Gun:    return PZCfg::kColGun;
    default:              return PZCfg::kColTurbo;
    }
}

const char* PZRunDirector::nameFor(PowerUp kind) {
    switch (kind) {
    case PowerUp::Magnet: return PZCfg::kStrMagnet;
    case PowerUp::Shield: return PZCfg::kStrShield;
    case PowerUp::Blast:  return PZCfg::kStrBlastName;
    case PowerUp::Jetpack:return PZCfg::kStrJetpackName;
    case PowerUp::Rocket: return PZCfg::kStrRocketName;
    case PowerUp::Gun:    return PZCfg::kStrGunName;
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
                    : kind == PowerUp::Blast  ? PZCfg::kArtOrbBlast
                    : kind == PowerUp::Jetpack ? PZCfg::kArtOrbJetpack
                    : kind == PowerUp::Rocket ? PZCfg::kArtOrbRocket
                    : kind == PowerUp::Gun ? PZCfg::kArtOrbGun : PZCfg::kArtOrbTurbo;
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
                     : kind == PowerUp::Blast  ? PZCfg::kStrBlastIcon
                     : kind == PowerUp::Jetpack ? PZCfg::kStrJetpackIcon
                     : kind == PowerUp::Rocket ? PZCfg::kStrRocketIcon
                     : kind == PowerUp::Gun ? PZCfg::kStrGunIcon : PZCfg::kStrTurboIcon;
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

    // Height is always measured from a real platform surface ahead of the player. It used to fall back to the rider's
    // own height, so a power-up collected while falling into a pit spawned the next orb down at pit level (an orb that
    // then flew inside the wall of the following platform).
    float gy = 0.f;
    bool solid = false;
    if (PZGGamefieldPlatforms* p = platform()) {
        static const float probes[] = {200.f, 0.f, 420.f, 640.f, 860.f, -180.f};
        for (float dx : probes) {
            if (p->groundYAt(x - dx, gy)) { solid = true; break; }
        }
    }
    if (!solid) solid = nearestSurface(gy);
    if (!solid) gy = win.height * 0.42f;         // no platform anywhere near: pick a screen-relative line

    // Always reachable with a jump, on every platform's physics scale.
    const float lift = std::min(PZCfg::kOrbHeightAboveGround * k, apex() * 0.78f);
    const float y = std::max(90.f, std::min(win.height - 120.f, gy + lift + CCRANDOM_0_1() * 30.f));

    const float wM = PZCfg::kWeightMagnet, wS = PZCfg::kWeightShield, wT = PZCfg::kWeightTurbo, wB = PZCfg::kWeightBlast;
    const float wJ = PZCfg::kEnableJetpack ? PZCfg::kWeightJetpack : 0.f;
    const float wR = PZCfg::kEnableRocket ? PZCfg::kOrbWeightRocket : 0.f;
    const float wG = PZCfg::kEnableGun ? PZCfg::kOrbWeightGun : 0.f;
    const float total = std::max(0.001f, wM + wS + wT + wB + wJ + wR + wG);
    const float r = CCRANDOM_0_1() * total;
    PowerUp kind = PowerUp::Blast;
    if (r < wM) kind = PowerUp::Magnet;
    else if (r < wM + wS) kind = PowerUp::Shield;
    else if (r < wM + wS + wT) kind = PowerUp::Turbo;
    else if (r < wM + wS + wT + wB) kind = PowerUp::Blast;
    else if (r < wM + wS + wT + wB + wJ) kind = PowerUp::Jetpack;
    else if (r < wM + wS + wT + wB + wJ + wR) kind = PowerUp::Rocket;
    else kind = PowerUp::Gun;

    Orb orb;
    orb.kind = kind;
    orb.node = makeOrbVisual(kind);
    orb.node->setPosition(Vec2(x, y));
    _scene->addChild(orb.node, kZOrb);
    _orbs.push_back(orb);
}

void PZRunDirector::updateOrbs(float dt, float scroll) {
    if (PZCfg::kEnablePowerUps && _time >= _nextOrbAt && _orbs.empty() && _turboT <= 0.f && _jetpackT <= 0.f &&
        _rocketT <= 0.f) {
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
        // Orb sitting below the road (spawned while the rider was dropping into a pit): unreachable, so drop it.
        float orbGround = 0.f;
        if (!offscreen && platform() && platform()->groundYAt(p.x, orbGround) &&
            p.y < orbGround + 24.f * kDeviceScale()) {
            o.node->removeFromParent();
            _orbs.erase(_orbs.begin() + i);
            _nextOrbAt = _time + 0.6f;      // re-roll the spawn a moment later instead of every frame
            continue;
        }
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
    grant(name == "magnet" ? PowerUp::Magnet
        : name == "shield" ? PowerUp::Shield
        : name == "blast" ? PowerUp::Blast
        : name == "jetpack" ? PowerUp::Jetpack
        : name == "rocket" ? PowerUp::Rocket
        : name == "gun" ? PowerUp::Gun : PowerUp::Turbo);
}

void PZRunDirector::collect(PowerUp kind, const Vec2& at, float durationOverride) {
    if (_jetpackT > 0.f && (kind == PowerUp::Turbo || kind == PowerUp::Rocket)) return;   // flight owns gravity
    const bool bonus = durationOverride > 0.f;      // granted by revive: no pickup chrome
    const Color3B col = colorFor(kind);
    if (!bonus) {
        ++_stats.powerUps;
        playPickupSound();
        PZUI::floatText(_scene, nameFor(kind), at + Vec2(0, 34), col, 12.f, kZFx);
        PZHaptics::play(kind == PowerUp::Blast ? PZHaptics::Kind::Heavy : PZHaptics::Kind::Medium);
    }
    PZUI::burst(_scene, at, col, 12, 70.f, 0.5f, kZFx);
    PZUI::glitter(_scene, at, col, 10, 90.f, 0.8f, kZFx + 5);
    ring(at, col, 110.f, 0.45f);
    if (!bonus && kind != PowerUp::Blast) cameraPunch(1.0f + (PZCfg::kPunchZoom - 1.0f) * 0.7f, 0.1f, 0.28f);

    {   // pickup sounds: a generic chime + the power-up's own effect, and the announcer for those without a banner
        if (!bonus) PZAudio::play("fx_powerup");
        switch (kind) {
        case PowerUp::Magnet:  PZAudio::play("fx_magnet", 0.9f, 0.1f);  if (!bonus) PZAudio::say("vo_magnet"); break;
        case PowerUp::Shield:  PZAudio::play("fx_shield", 0.9f, 0.1f);  if (!bonus) PZAudio::say("vo_shield"); break;
        case PowerUp::Turbo:   PZAudio::play("fx_turbo", 0.9f, 0.1f);   if (!bonus) PZAudio::say("vo_turbo"); break;
        case PowerUp::Blast:   PZAudio::play("fx_blast", 1.0f, 0.1f);   break;
        case PowerUp::Jetpack: PZAudio::play("fx_jetpack", 0.9f, 0.1f); break;
        case PowerUp::Rocket:  PZAudio::play("fx_rocket", 0.9f, 0.1f);  break;
        case PowerUp::Gun:     PZAudio::play("fx_gun_on", 0.9f, 0.1f);  break;
        }
    }

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
            _turboLockY = cruiseLine();
            if (_rocketT > 0.f) endRocket();
            _scene->character->p_body->SetGravityScale(0.f);
            _scene->character->jump_counter = 1;
            shakeScene(7.f, 0.35f);
            flash(col, 0.3f, 0.3f);
        }
        _turboT = _turboMax = bonus ? durationOverride : PZCfg::kTurboDuration;
        break;
    case PowerUp::Rocket:
        if (_rocketT <= 0.f) startRocket();
        _rocketT = _rocketMax = bonus ? durationOverride : PZCfg::kRocketDuration;
        // The rocket bike carries a magnet and shrugs off hits while it burns.
        _magnetT = std::max(_magnetT, _rocketT);
        _shieldT = std::max(_shieldT, _rocketT);
        break;
    case PowerUp::Gun:
        if (_gunT <= 0.f && !bonus) {
            PZUI::banner(_scene, PZCfg::kStrGun, "", col,
                         Director::getInstance()->getWinSize().height * 0.6f, 0.7f);
            PZHaptics::play(PZHaptics::Kind::Medium);
        }
        _gunT = _gunMax = bonus ? durationOverride : PZCfg::kGunDuration;
        _gunClock = 0.f;
        break;
    case PowerUp::Blast:
        beginBlastCinematic();
        break;
    case PowerUp::Jetpack:
        if (_jetpackT <= 0.f) {
            if (turboActive()) endTurbo();
            if (_scene->character && _scene->character->p_body) {
                float gy = riderPos().y;
                float ground = 0.f;
                if (nearestSurface(ground)) gy = ground;
                const float maxY = Director::getInstance()->getWinSize().height - 150.f;
                _jetpackLockY = std::min(maxY, gy + PZCfg::kJetpackHeight * kDeviceScale());
                _scene->character->p_body->SetGravityScale(0.f);
                _scene->character->jump_counter = 1;
            }
            shakeScene(8.f, 0.35f);
            flash(col, 0.3f, 0.3f);
            PZUI::banner(_scene, PZCfg::kStrJetpack, "", col,
                         Director::getInstance()->getWinSize().height * 0.6f, 0.7f);
            PZHaptics::play(PZHaptics::Kind::Heavy);
        }
        _jetpackT = _jetpackMax = bonus ? durationOverride : PZCfg::kJetpackDuration;
        _jetpackCoinClock = 0.f;
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
    if (_jetpackT > 0.f) {
        _jetpackT -= dt;
        if (_jetpackT <= 0.f) endJetpack();
    }
    if (_rocketT > 0.f) {
        _rocketT -= dt;
        if (_rocketT <= 0.f) endRocket();
    }
    if (_gunT > 0.f) {
        _gunT -= dt;
        if (_gunT <= 0.f) endGun();
    }
    // The rocket bike is the faster ride; turbo is the fallback when it is not burning.
    float target = 1.f;
    if (_rocketT > 0.f) target = PZCfg::kRocketSpeedMult;
    else if (_turboT > 0.f) target = PZCfg::kTurboSpeedMult;
    _boost += (target - _boost) * std::min(1.f, dt * 4.f);
    if (std::abs(_boost - target) < 0.002f) _boost = target;

    constexpr int kChips = kChipCount;
    const float left[kChips] = {_magnetT, _shieldT, _turboT, _jetpackT, _rocketT, _gunT};
    const float full[kChips] = {std::max(_magnetMax, 0.01f), std::max(_shieldMax, 0.01f), std::max(_turboMax, 0.01f),
                                std::max(_jetpackMax, 0.01f), std::max(_rocketMax, 0.01f), std::max(_gunMax, 0.01f)};
    const Rect safe = PZLayout::safeRect();
    int active = 0;
    for (int i = 0; i < kChips; ++i) if (left[i] > 0.f) ++active;
    const float spacing = 152.f;
    int slot = 0;
    for (int i = 0; i < kChips; ++i) {
        HudRow& r = _rows[i];
        if (left[i] > 0.f) {
            const float frac = left[i] / full[i];
            const Color3B col = frac < 0.25f ? PZCfg::kColBad : colorFor(chipKind(i));
            PZUI::redrawBar(r.bar, 118.f, 7.f, frac, col);
            r.root->setPosition(Vec2(safe.getMidX() + (slot - (active - 1) * 0.5f) * spacing, safe.getMaxY() - 126.f));
            if (!r.root->isVisible()) {
                r.name->setString(nameFor(chipKind(i)));
                r.name->setColor(col);
            }
            char secs[16];
            std::snprintf(secs, sizeof(secs), "%.1fs", left[i]);
            r.time->setString(secs);
            // Gentle pulse when about to run out.
            r.icon->setScale(frac < 0.25f ? 1.f + 0.12f * std::sin(_auraClock * 14.f) : 1.f);
            r.root->setVisible(true);
            ++slot;
        } else {
            r.root->setVisible(false);
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
    // If the cruise line ended up below the world (pickup over a pit), land now instead of sinking until the timer runs
    // out and the rider dies with no power-up left to save them.
    if (ch->loc.y < -20.f || ch->loc.y < _turboLockY - 150.f) {      // fell out of the ride line: land now
        endTurbo();
        return;
    }
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
    // Grounded again the instant the ride ends, so the next tap is a jump (it used to be swallowed as a buffered tap).
    if (!groundRiderNow()) ch->jump_counter = 1;
    clearHazards(120.f * kDeviceScale(), 520.f * kDeviceScale());
    PZUI::burst(_scene, ch->loc, PZCfg::kColTurbo, 14, 80.f, 0.5f, kZFx);
    ring(ch->loc, PZCfg::kColTurbo, 150.f, 0.45f);
    PZHaptics::play(PZHaptics::Kind::Medium);
}

// Height the powered rides cruise at: the top of whatever platform is under the rider, so a pickup grabbed over a
// gap never parks the bike at the bottom of a pit for the rest of the power-up.
// Surface height the powered rides should be measured from. A pickup grabbed while the rider is over a gap (or falling
// into one) used to lock the ride line at the rider's current, pit-level height, which left the bike flying inside the
// wall of the next platform until the power-up expired. Search around the rider for a platform and remember the last one.
bool PZRunDirector::nearestSurface(float& outY) {
    PZGGamefieldPlatforms* p = platform();
    const float k = kDeviceScale();
    if (p) {
        static const float probes[] = {0.f, 160.f, 340.f, 560.f, -140.f, 900.f};
        for (float dx : probes) {
            float gy = 0.f;
            if (p->groundYAt(riderPos().x + dx * k, gy)) {
                outY = gy;
                _lastGroundY = gy;
                _haveGroundY = true;
                return true;
            }
        }
    }
    if (_haveGroundY) {
        outY = _lastGroundY;
        return true;
    }
    return false;
}

float PZRunDirector::cruiseLine() {
    float gy = 0.f;
    if (nearestSurface(gy)) {
        return gy + (_haveOffset ? _riderOffset : 30.f) + 2.f;
    }
    return std::max(riderPos().y, Director::getInstance()->getWinSize().height * 0.45f);
}

void PZRunDirector::startRocket() {
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) return;
    if (turboActive()) endTurbo();
    if (_jetpackT > 0.f) endJetpack();

    _rocketLockY = cruiseLine();
    ch->p_body->SetGravityScale(0.f);
    ch->jump_counter = 1;

    // Rocket pod strapped to the side of the bike: steel body with a red stripe, nose cone, fins, and a flame that
    // flickers behind it while it burns.
    if (!_rocketArt) {
        auto* root = Node::create();
        auto* d = DrawNode::create();
        d->drawSolidRect(Vec2(-30.f, -6.f), Vec2(20.f, 6.f), PZUI::c4f(Color3B(196, 202, 214)));
        d->drawSolidRect(Vec2(-30.f, -6.f), Vec2(20.f, -2.f), PZUI::c4f(Color3B(140, 146, 160)));        // shaded belly
        d->drawSolidRect(Vec2(-4.f, -6.f), Vec2(4.f, 6.f), PZUI::c4f(PZCfg::kColRocket));                // stripe
        d->drawTriangle(Vec2(20.f, -6.f), Vec2(20.f, 6.f), Vec2(40.f, 0.f), PZUI::c4f(PZCfg::kColRocket)); // nose cone
        d->drawTriangle(Vec2(-30.f, 6.f), Vec2(-24.f, 6.f), Vec2(-38.f, 17.f), PZUI::c4f(PZCfg::kColRocket));   // fins
        d->drawTriangle(Vec2(-30.f, -6.f), Vec2(-24.f, -6.f), Vec2(-38.f, -17.f), PZUI::c4f(PZCfg::kColRocket));
        d->drawSolidRect(Vec2(-34.f, -4.f), Vec2(-30.f, 4.f), PZUI::c4f(Color3B(60, 64, 80)));          // nozzle
        d->drawCircle(Vec2(8.f, 0.f), 3.f, 0.f, 10, false, PZUI::c4f(Color3B(60, 64, 80)), 1.5f);          // porthole
        root->addChild(d);
        _rocketFlame = DrawNode::create();
        _rocketFlame->drawTriangle(Vec2(-34.f, -7.f), Vec2(-34.f, 7.f), Vec2(-92.f, 0.f), PZUI::c4f(PZCfg::kColRocket, 0.85f));
        _rocketFlame->drawTriangle(Vec2(-34.f, -4.f), Vec2(-34.f, 4.f), Vec2(-66.f, 0.f), PZUI::c4f(Color3B(255, 226, 120), 0.95f));
        _rocketFlame->drawTriangle(Vec2(-34.f, -2.f), Vec2(-34.f, 2.f), Vec2(-48.f, 0.f), PZUI::c4f(Color3B(255, 255, 255), 0.95f));
        root->addChild(_rocketFlame);
        _rocketArt = root;
        _rocketArt->setVisible(false);
        _scene->addChild(_rocketArt, kZFx - 5);
    }
    _rocketArt->setVisible(true);

    const Size win = Director::getInstance()->getWinSize();
    shakeScene(11.f, 0.45f);
    flash(PZCfg::kColRocket, 0.26f, 0.3f);
    PZUI::banner(_scene, PZCfg::kStrRocket, "", PZCfg::kColRocket, win.height * 0.6f, 0.7f);
    PZHaptics::play(PZHaptics::Kind::Heavy);
}

void PZRunDirector::endRocket() {
    _rocketT = 0.f;
    if (_rocketArt) _rocketArt->setVisible(false);
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) return;
    // Shared landing assist: put the rider on solid ground instead of dropping them into a gap.
    endTurbo();
}

void PZRunDirector::updateRocketRide(float dt) {
    if (_rocketT <= 0.f) {
        if (_rocketArt) _rocketArt->setVisible(false);
        return;
    }
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) return;

    // Safety: if the ride line ends up under the world the rider is falling, so land now.
    if (ch->loc.y < -20.f || ch->loc.y < _rocketLockY - 150.f) {
        endRocket();
        return;
    }

    const float bob = std::sin(_auraClock * 11.f) * 4.f;
    const float dy = (_rocketLockY + bob) - ch->loc.y;
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, dy * 10.f * kPhysicsWorldScale()));
    ch->jump_counter = 1;

    if (_rocketArt) {
        _rocketArt->setPosition(ch->loc + Vec2(4.f, -22.f));         // along the bike's lower side
        if (_rocketFlame) _rocketFlame->setScale(0.75f + 0.5f * CCRANDOM_0_1(), 0.9f + 0.2f * CCRANDOM_0_1());
    }

    // Shielded: plough through anything lethal.
    if (platform()) {
        const Rect rb = riderBox();
        for (int i = 0; i < platform()->obstacleSlots(); ++i) {
            PZGObstacle* o = platform()->obstacleAt(i);
            if (!o || o->dead || !o->sprite || !o->destroyCharacter) continue;
            if (o->worldBox().intersectsRect(rb)) destroyObstacle(o, Style::Launch);
        }
    }

    _trailClock += dt;
    if (_trailClock > 0.03f) {
        _trailClock = 0.f;
        PZUI::burst(_scene, ch->loc + Vec2(-44.f, -6.f), PZCfg::kColRocket, 3, 34.f, 0.32f, kZFx - 10);
        riderAfterimage(PZCfg::kColRocket);
    }
}

// Auto-cannon: every `kGunInterval` seconds the bike shoots the closest hazard ahead.
void PZRunDirector::updateGun(float dt) {
    if (_gunT <= 0.f) {
        _gunT = 0.f;
        if (_gunArt) _gunArt->setVisible(false);
        return;
    }
    // Cannon mounted on the front of the bike.
    if (!_gunArt) {
        _gunArt = DrawNode::create();
        _gunArt->drawSolidRect(Vec2(0.f, -7.f), Vec2(20.f, 7.f), PZUI::c4f(Color3B(70, 76, 96)));               // receiver
        _gunArt->drawSolidRect(Vec2(20.f, -3.5f), Vec2(52.f, 3.5f), PZUI::c4f(Color3B(150, 156, 172)));        // barrel
        _gunArt->drawSolidRect(Vec2(46.f, -5.f), Vec2(54.f, 5.f), PZUI::c4f(Color3B(96, 100, 116)));            // muzzle brake
        _gunArt->drawSolidRect(Vec2(4.f, 7.f), Vec2(14.f, 11.f), PZUI::c4f(PZCfg::kColGun));                    // sight
        _gunArt->drawSolidRect(Vec2(4.f, -13.f), Vec2(12.f, -7.f), PZUI::c4f(Color3B(52, 56, 72)));             // grip
        _gunArt->drawCircle(Vec2(10.f, 0.f), 3.f, 0.f, 10, false, PZUI::c4f(PZCfg::kColGun), 1.5f);
        _scene->addChild(_gunArt, kZFx - 4);
        _gunFlash = DrawNode::create();
        _gunFlash->drawTriangle(Vec2(0.f, -9.f), Vec2(0.f, 9.f), Vec2(26.f, 0.f), PZUI::c4f(Color3B(255, 238, 170), 0.95f));
        _gunFlash->drawTriangle(Vec2(0.f, -4.f), Vec2(0.f, 4.f), Vec2(14.f, 0.f), PZUI::c4f(Color3B(255, 255, 255), 0.95f));
        _gunFlash->setVisible(false);
        _scene->addChild(_gunFlash, kZFx - 3);
    }
    if (_scene->character) {
        const Vec2 rp = _scene->character->loc;
        _gunArt->setVisible(true);
        _gunArt->setPosition(rp + Vec2(26.f, 4.f));
        if (_gunFlash) _gunFlash->setPosition(rp + Vec2(26.f + 54.f, 4.f));
    }
    if (!platform() || !_scene->character) return;
    _gunClock += dt;
    if (_gunClock < PZCfg::kGunInterval) return;
    _gunClock = 0.f;

    const Vec2 rp = riderPos();
    const float range = PZCfg::kGunRange * kDeviceScale();
    PZGObstacle* best = nullptr;
    float bestX = std::numeric_limits<float>::max();
    for (int i = 0; i < platform()->obstacleSlots(); ++i) {
        PZGObstacle* o = platform()->obstacleAt(i);
        if (!o || o->dead || !o->sprite || !o->destroyCharacter) continue;
        const float ox = o->getPosition().x;
        if (ox < rp.x + 30.f || ox > rp.x + range) continue;
        if (ox < bestX) {
            bestX = ox;
            best = o;
        }
    }
    if (best) fireAt(best);
}

void PZRunDirector::fireAt(PZGObstacle* target) {
    if (!target) return;
    const Vec2 from = riderPos() + Vec2(34.f, 8.f);
    const Rect box = target->worldBox();
    const Vec2 to(box.getMidX(), box.getMidY());
    PZAudio::play("fx_shot", 0.7f, 0.08f);

    auto* tracer = DrawNode::create();
    tracer->drawSegment(Vec2::ZERO, to - from, 2.4f, PZUI::c4f(Color3B(255, 248, 210), 0.95f));
    tracer->drawDot(to - from, 3.f, PZUI::c4f(PZCfg::kColGun, 0.9f));
    tracer->setPosition(from);
    _scene->addChild(tracer, kZFx + 10);
    tracer->runAction(Sequence::create(FadeOut::create(0.1f), RemoveSelf::create(), nullptr));

    if (_gunFlash) {
        _gunFlash->stopAllActions();
        _gunFlash->setVisible(true);
        _gunFlash->setScale(0.7f + 0.5f * CCRANDOM_0_1());
        _gunFlash->runAction(Sequence::create(DelayTime::create(0.05f), Hide::create(), nullptr));
    }
    if (_gunArt) {
        _gunArt->stopActionByTag(78);
        auto* kick = Sequence::create(MoveBy::create(0.03f, Vec2(-5.f, 0.f)), MoveBy::create(0.06f, Vec2(5.f, 0.f)), nullptr);
        kick->setTag(78);
        _gunArt->runAction(kick);
    }
    PZUI::burst(_scene, from, PZCfg::kColGun, 3, 22.f, 0.18f, kZFx);
    PZUI::burst(_scene, to, PZCfg::kColGun, 8, 48.f, 0.3f, kZFx);
    shakeScene(3.f, 0.08f);
    destroyObstacle(target, Style::Explode);
}

void PZRunDirector::endGun() {
    _gunT = 0.f;
    _gunClock = 0.f;
    if (_gunArt) _gunArt->setVisible(false);
    if (_gunFlash) _gunFlash->setVisible(false);
}

bool PZRunDirector::safeLandHere(float& outY) {
    PZGGamefieldPlatforms* p = platform();
    return p && p->groundYAt(riderPos().x, outY);
}

// True when the surface under the rider continues for `need` units ahead at the same height. Revive and the powered
// rides use it so the run never resumes on the last metres of a platform (the rider used to be dropped straight into
// the gap that followed as soon as the grace window ended).
bool PZRunDirector::runwayAhead(float need) {
    PZGGamefieldPlatforms* p = platform();
    if (!p || !_scene || !_scene->character) return false;
    const float x = riderPos().x;
    float gy = 0.f;
    if (!p->groundYAt(x, gy)) return false;
    const float step = 60.f;
    float reached = 0.f;
    for (float dx = 0.f; dx <= need; dx += step) {
        float g2 = 0.f;
        if (!p->groundYAt(x + dx, g2) || std::abs(g2 - gy) > 8.f) break;
        reached = dx;
    }
    return reached >= need;
}

// Revive used to teleport the rider onto the nearest platform edge. Now the world keeps scrolling while the rider
// hovers, and the run resumes the moment a real platform has slid underneath: no instant second crash.
void PZRunDirector::updateReviveWait(float dt) {
    if (!_reviveWait) return;
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) {
        _reviveWait = false;
        return;
    }
    _reviveWaitT += dt;

    // Wait for a platform that actually STARTS under the rider and keeps going, not for the last few units of one:
    // resuming there meant the next gap arrived before the player could react.
    float gy = 0.f;
    const bool solid = safeLandHere(gy);
    const bool runway = solid && runwayAhead(360.f * kDeviceScale());
    const Size win = Director::getInstance()->getWinSize();
    const float hover = std::max(210.f, win.height * 0.58f);
    const float targetY = runway ? gy + (_haveOffset ? _riderOffset : 30.f) + 10.f : hover;
    const float dy = targetY - ch->loc.y;

    ch->p_body->SetGravityScale(0.f);
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, dy * 7.f * kPhysicsWorldScale()));
    ch->jump_counter = 1;

    // After 2.5 s without a usable platform, stop waiting and slide the world instead (never hover forever).
    if ((runway && _reviveWaitT > 0.3f && std::abs(dy) < 34.f) || _reviveWaitT > 2.5f) {
        _reviveWait = false;
        rescueRider();

        float check = 0.f;
        if (!safeLandHere(check)) {
            // The rescue shifted the world out from under us: keep waiting.
            _reviveWait = true;
            _reviveWaitT = 0.f;
            return;
        }

        beginReadyCountdown(true);
    }
}

// After a revive the rider is parked on the platform and the world is held while "3 - 2 - 1" counts down, so the
// player's thumb is ready before the run resumes (an instant restart killed revived riders on the very next gap).
void PZRunDirector::beginReadyCountdown(bool revive) {
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) return;
    _readyCountdown = true;
    ch->p_body->SetGravityScale(1.f);
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
    ch->jump_counter = 1;
    const float readyLead = PZCfg::kReadyLeadSeconds;
    freezeFor(readyLead + PZCfg::kReadySeconds * 3.f + 0.9f); // held; released explicitly below

    const Size win = Director::getInstance()->getWinSize();
    const Rect safe = PZLayout::safeRect();
    const Vec2 centre(safe.getMidX(), win.height * 0.56f);

    // The countdown is a screen-space overlay: the camera is zoomed in during the intro, and anything parented to the
    // field layer would slide off centre with it.
    Node* fx = _scene->getParent() ? _scene->getParent() : _scene;
    auto tick = [this, centre, fx](const char* text, const Color3B& col, bool go) {
        auto* l = PZUI::heading(text, go ? 54.f : 84.f, col);
        l->setPosition(centre);
        l->setScale(2.4f);
        l->setOpacity(0);
        fx->addChild(l, kZFx + 40);
        l->runAction(Sequence::create(
            Spawn::create(EaseBackOut::create(ScaleTo::create(0.22f, 1.f)), FadeIn::create(0.12f), nullptr),
            DelayTime::create(go ? 0.25f : 0.2f),
            Spawn::create(ScaleTo::create(0.16f, 0.6f), FadeOut::create(0.16f), nullptr),
            RemoveSelf::create(), nullptr));
        auto* d = DrawNode::create();
        d->drawCircle(Vec2::ZERO, 30.f, 0.f, 36, false, PZUI::c4f(col, 0.95f), 4.f);
        d->setPosition(centre);
        d->setScale(0.3f);
        fx->addChild(d, kZFx + 39);
        d->runAction(Sequence::create(Spawn::create(EaseOut::create(ScaleTo::create(0.5f, 5.f), 2.f),
                                                    FadeOut::create(0.5f), nullptr),
                                      RemoveSelf::create(), nullptr));
        PZHaptics::play(go ? PZHaptics::Kind::Success : PZHaptics::Kind::Light);
        PZAudio::play(go ? "fx_go_hit" : "fx_tick", 1.f, 0.05f);
        if (go) PZAudio::say("vo_go", 2);
    };

    const float step = PZCfg::kReadySeconds;
    auto* countdown = Sequence::create(
        CallFunc::create([=]() {
            Node* fx = _scene->getParent() ? _scene->getParent() : _scene;
            auto* ready = PZUI::heading(PZCfg::kStrGetReady, 26.f, PZCfg::kColAccent);
            ready->setPosition(Vec2(safe.getMidX(), win.height * 0.72f));
            ready->setScale(0.5f);
            ready->setOpacity(0);
            fx->addChild(ready, kZFx + 30);
            ready->runAction(Sequence::create(
                Spawn::create(EaseBackOut::create(ScaleTo::create(0.22f, 1.f)), FadeIn::create(0.18f), nullptr),
                DelayTime::create(std::max(0.f, readyLead - 0.42f)),
                Spawn::create(MoveBy::create(0.2f, Vec2(0.f, 18.f)), FadeOut::create(0.2f), nullptr),
                RemoveSelf::create(), nullptr));
            PZAudio::say("vo_get_ready", 2);
        }),
        DelayTime::create(readyLead),
        CallFunc::create([=]() { tick("3", PZCfg::kColAccent, false); PZAudio::say("vo_3", 2); }),
        DelayTime::create(step), CallFunc::create([=]() {
            tick("2", PZCfg::kColAccent2, false);
            PZAudio::say("vo_2", 2);
            if (!revive) {
                cameraReset(0.8f);
                setHudVisible(true);
                animateHudIn();
            }
        }),
        DelayTime::create(step), CallFunc::create([=]() { tick("1", PZCfg::kColBad, false); PZAudio::say("vo_1", 2); }),
        DelayTime::create(step), CallFunc::create([=]() {
            tick(PZCfg::kStrGo, PZCfg::kColGood, true);
            _readyCountdown = false;
            _cinematic = false;
            _introActive = false;
            _freezeUntil = std::chrono::steady_clock::time_point{};   // release the world
            _graceT = std::max(_graceT, PZCfg::kHitGrace + 0.6f);
            closeCinematicFrame(0.28f);
            if (revive && PZCfg::kReviveTurbo > 0.f && PZCfg::kEnablePowerUps) {
                collect(PowerUp::Turbo, riderPos(), PZCfg::kReviveTurbo);
            } else {
                groundRiderNow();                                      // taps work on the very first frame
            }
            PZUI::burst(_scene, riderPos(), PZCfg::kColGood, 14, 90.f, 0.5f, kZFx);
            if (!revive) {
                // The crowd answers GO after the camera and HUD have finished their reveal.
                if (_scene->tip1 && _introTip1Visible) {
                    _scene->tip1->setVisible(true);
                    PZUI::enterIn(_scene->tip1, 0.18f, false, 14.f);
                }
                if (_scene->tip2 && _introTip2Visible) {
                    _scene->tip2->setVisible(true);
                    PZUI::enterIn(_scene->tip2, 0.28f, false, 14.f);
                }
                auto* cheer = Sequence::create(
                    DelayTime::create(0.36f),
                    CallFunc::create([]() { PZAudio::play("fx_crowd_cheer", 0.9f, 0.5f); }),
                    nullptr);
                cheer->setTag(kCrowdCueTag);
                runAction(cheer);
                flash(Color3B(255, 255, 255), 0.18f, 0.3f);
            }
        }),
        nullptr);
    countdown->setTag(kCountdownTag);
    runAction(countdown);
}

// Puts the rider flat on the platform under it and clears the "airborne" flag so the next tap is a real jump.
// Powered rides and revives end here: the player never has to wait for a landing they cannot see.
bool PZRunDirector::groundRiderNow() {
    PZGCharacter* ch = _scene ? _scene->character : nullptr;
    if (!ch || !ch->p_body) return false;
    float gy = 0.f;
    if (!safeLandHere(gy)) return false;
    const float offset = _haveOffset ? _riderOffset : 30.f;
    ch->setLoc(Vec2(ch->loc.x, gy + offset + 4.f));
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
    ch->p_body->SetGravityScale(1.f);
    ch->jump_counter = 0;
    ch->jump_enabled = false;
    return true;
}

// Death show: the world holds, the camera pushes in on the wreck with fire and the disappointed crowd, then pulls
// back out before the revive offer / game-over screen arrives.
void PZRunDirector::beginDeathCinematic(std::function<void()> completion) {
    if (!_scene || !_scene->character) {
        if (completion) completion();
        return;
    }
    stopActionByTag(kCrowdCueTag);
    if (_introActive || _readyCountdown || _cinematic) {
        stopActionByTag(kIntroDelayTag);
        stopActionByTag(kIntroRideTag);
        stopActionByTag(kCountdownTag);
        _scene->stopActionByTag(kCameraSnapTag);
        _introActive = false;
        _readyCountdown = false;
        _cinematic = false;
    }
    if (!PZCfg::kEnableCinematics) {
        if (completion) completion();
        return;
    }
    _cinematic = true;
    freezeFor(1.45f);
    setHudVisible(false);
    const Vec2 at = riderPos();
    openCinematicFrame();
    cameraTo(at, 1.62f, 0.42f, &at);
    PZAudio::play("fx_fire", 0.9f, 0.3f);
    PZAudio::play("fx_crowd_boo", 0.8f, 0.6f);
    flash(Color3B(255, 190, 110), 0.48f, 0.28f);
    ring(at, PZCfg::kColBad, 190.f, 0.42f);

    // Fire at the crash: orange licks + dark smoke puffs.
    for (int i = 0; i < 12; ++i) {
        auto* f = DrawNode::create();
        const float sz = rnd(7.f, 16.f);
        Vec2 tri[3] = {Vec2(-sz, 0.f), Vec2(0.f, sz * 2.2f), Vec2(sz, 0.f)};
        f->drawSolidPoly(tri, 3, PZUI::c4f(Color3B(255, 140, 40), 0.9f), 0.f, Color4F(0, 0, 0, 0), true);
        f->setBlendFunc(BlendFunc::ADDITIVE);
        f->setPosition(at + Vec2(rnd(-40.f, 40.f), rnd(-16.f, 16.f)));
        f->setScale(0.4f);
        _scene->addChild(f, kZFx + 20);
        f->runAction(Sequence::create(DelayTime::create(rnd(0.f, 0.18f)),
                                      Spawn::create(EaseOut::create(ScaleTo::create(0.42f, 1.2f, 1.8f), 2.f),
                                                    MoveBy::create(0.42f, Vec2(0.f, 30.f)), FadeOut::create(0.42f), nullptr),
                                      RemoveSelf::create(), nullptr));
    }
    for (int i = 0; i < 6; ++i) {
        auto* s = DrawNode::create();
        s->drawSolidCircle(Vec2::ZERO, rnd(8.f, 16.f), 0.f, 12, PZUI::c4f(Color3B(60, 56, 56), 0.55f));
        s->setPosition(at + Vec2(rnd(-30.f, 30.f), rnd(0.f, 24.f)));
        _scene->addChild(s, kZFx + 19);
        s->runAction(Sequence::create(Spawn::create(MoveBy::create(0.7f, Vec2(rnd(-20.f, 20.f), rnd(50.f, 90.f))),
                                                    ScaleTo::create(0.7f, 2.2f), FadeOut::create(0.7f), nullptr),
                                      RemoveSelf::create(), nullptr));
    }
    PZHaptics::play(PZHaptics::Kind::Error);

    auto* deathShow = Sequence::create(DelayTime::create(0.68f), CallFunc::create([this]() {
        cameraReset(0.48f);
        closeCinematicFrame(0.38f);
    }), DelayTime::create(0.52f), CallFunc::create([this, completion]() {
        setHudVisible(true);
        _cinematic = false;
        if (completion) completion();
    }), nullptr);
    deathShow->setTag(kDeathShowTag);
    runAction(deathShow);
}

void PZRunDirector::updateJetpackRide(float dt) {
    if (_jetpackT <= 0.f || !_scene->character || !_scene->character->p_body) return;
    PZGCharacter* ch = _scene->character;

    // Hover on the flight line with a light bob; the world scrolls underneath.
    const float bob = std::sin(_auraClock * 7.f) * 7.f;
    const float dy = (_jetpackLockY + bob) - ch->loc.y;
    ch->p_body->SetLinearVelocity(b2Vec2(0.f, dy * 8.f * kPhysicsWorldScale()));
    ch->jump_counter = 1;

    // Clear anything in the flight path (and pay smash coins through destroyObstacle).
    if (platform()) {
        const Rect rb = riderBox();
        for (int i = 0; i < platform()->obstacleSlots(); ++i) {
            PZGObstacle* o = platform()->obstacleAt(i);
            if (!o || o->dead || !o->sprite || !o->destroyCharacter) continue;
            if (o->worldBox().intersectsRect(rb)) destroyObstacle(o, Style::Explode);
        }
    }

    // Flame trail + afterimages.
    _trailClock += dt;
    if (_trailClock > 0.035f) {
        _trailClock = 0.f;
        PZUI::burst(_scene, ch->loc + Vec2(-30.f, -16.f), PZCfg::kColJetpack, 3, 34.f, 0.3f, kZFx - 12);
        PZUI::burst(_scene, ch->loc + Vec2(-30.f, -16.f), PZCfg::kColTurbo, 1, 22.f, 0.22f, kZFx - 12);
        riderAfterimage(PZCfg::kColJetpack);
    }

    // A ribbon of coins pays for the ride; they fly to the HUD so the count stays readable.
    _jetpackCoinClock += dt;
    if (_jetpackCoinClock >= std::max(0.05f, PZCfg::kJetpackCoinEvery)) {
        _jetpackCoinClock = 0.f;
        const int n = std::max(1, PZCfg::kJetpackCoinsPerTick);
        awardBonus(n, ch->loc + Vec2(0.f, 30.f), "", PZCfg::kColAccent);
        if (PZCfg::kEnableFx) coinFly(ch->loc + Vec2(20.f, 12.f));
        PZHaptics::play(PZHaptics::Kind::Tick);
    }
}

void PZRunDirector::endJetpack() {
    PZGCharacter* ch = _scene->character;
    if (!ch || !ch->p_body) return;
    _jetpackT = 0.f;
    ch->p_body->SetGravityScale(1.f);
    _graceT = std::max(_graceT, 1.2f);

    // Land on the nearest solid ground, sliding the world if the rider is over a gap.
    float gy = 0.f;
    PZGGamefieldPlatforms* p = platform();
    if (p) {
        if (!p->groundYAt(ch->loc.x, gy)) {
            p->alignForRevive(ch->loc.x, gy);
            if (!p->groundYAt(ch->loc.x, gy)) gy = _jetpackLockY - (_haveOffset ? _riderOffset : 0.f);
        }
        ch->setLoc(Vec2(ch->loc.x, gy + (_haveOffset ? _riderOffset : 0.f) + 4.f));
        ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
    }
    // Same rule as turbo: taps work on the frame the flight ends.
    if (!groundRiderNow()) ch->jump_counter = 1;
    clearHazards(120.f * kDeviceScale(), 520.f * kDeviceScale());
    PZUI::burst(_scene, ch->loc, PZCfg::kColJetpack, 18, 95.f, 0.55f, kZFx);
    ring(ch->loc, PZCfg::kColJetpack, 170.f, 0.45f);
    shakeScene(4.f, 0.2f);
    PZHaptics::play(PZHaptics::Kind::Medium);
}

// ---- camera + cinematics ---------------------------------------------------------------------

void PZRunDirector::shakeScene(float amplitude, float duration) {
    if (_cinematic || _camScale > 1.001f) return;   // the camera owns the scene transform during a cinematic / punch
    PZUI::shake(_scene, _camPos, amplitude, duration);
}

void PZRunDirector::setHudVisible(bool visible) {
    if (!visible) {
        auto hide = [this](Node* n) {
            if (n && n->isVisible()) {
                _hiddenHud.push_back({n, true});
                n->setVisible(false);
            }
        };
        hide(_hud);
        hide(_scene->distance_label);
        hide(_scene->coins_label);
        hide(_scene->coins_add_label);
        hide(_scene->getChildByTag(kBaseMenuItemTag));
        hide(_gadgetStrip);
    } else {
        for (auto& e : _hiddenHud) {
            if (e.first) e.first->setVisible(e.second);
        }
        _hiddenHud.clear();
    }
}

// Eased camera move: scales the whole scene layer and slides it so `focus` (a point in scene coordinates) lands in the
// middle of the screen. `focus` = screen centre and scale 1 is the neutral camera.
void PZRunDirector::cameraTo(const Vec2& focus, float scale, float duration, const Vec2* screenAnchor) {
    const Size win = Director::getInstance()->getWinSize();
    const Vec2 centre(win.width * 0.5f, win.height * 0.5f);
    const Vec2 anchor = screenAnchor ? *screenAnchor : centre;   // where on screen `focus` should end up
    const Vec2 f0 = _scene->convertToNodeSpace(centre);          // scene point currently under the screen centre
    const float s0 = _camScale;
    _scene->stopActionByTag(kCameraActionTag);
    auto* act = ActionFloat::create(std::max(0.01f, duration), 0.f, 1.f, [this, f0, focus, s0, scale, centre, anchor](float t) {
        const float e = t * t * (3.f - 2.f * t);                 // smoothstep
        _camScale = s0 + (scale - s0) * e;
        const Vec2 f = f0 + (focus - f0) * e;
        const Vec2 screenPt = centre + (anchor - centre) * e;
        _scene->setScale(_camScale);
        _scene->setPosition(Vec2::ZERO);
        const Vec2 w = _scene->convertToWorldSpace(f);
        _camPos = screenPt - w;
        _scene->setPosition(_camPos);
    });
    act->setTag(kCameraActionTag);
    _scene->runAction(act);
}

// Short zoom pulse that keeps the rider where they are on screen (stage change, power-up pickup, landing slam).
void PZRunDirector::cameraPunch(float scale, float inSeconds, float outSeconds) {
    if (_cinematic || !PZCfg::kEnableFx || scale <= 1.001f) return;
    constexpr int kPunchTag = 7733;
    stopActionByTag(kPunchTag);
    const Vec2 rp = riderPos();
    cameraTo(rp, scale, inSeconds, &rp);
    auto* seq = Sequence::create(DelayTime::create(inSeconds), CallFunc::create([this, outSeconds]() { cameraReset(outSeconds); }), nullptr);
    seq->setTag(kPunchTag);
    runAction(seq);
}

void PZRunDirector::cameraReset(float duration) {
    const Size win = Director::getInstance()->getWinSize();
    cameraTo(Vec2(win.width * 0.5f, win.height * 0.5f), 1.f, duration);
    // Snap to the exact neutral transform when the move completes (no sub-pixel drift).
    _scene->stopActionByTag(kCameraSnapTag);
    auto* snap = Sequence::create(DelayTime::create(duration + 0.02f), CallFunc::create([this]() {
        _camScale = 1.f;
        _camPos = Vec2::ZERO;
        _scene->setScale(1.f);
        _scene->setPosition(Vec2::ZERO);
    }), nullptr);
    snap->setTag(kCameraSnapTag);
    _scene->runAction(snap);
}

// "Destroy everything" pickup. The world holds still, the HUD steps aside, and the camera pushes in on each hazard in
// turn: a lock-on reticle, a punchy explosion, then on to the next. When the screen is clear the camera pulls back out
// and the run carries on. Hazards beyond the screen are cleaned up by the usual shockwave afterwards.
void PZRunDirector::beginBlastCinematic() {
    if (_scene->gameOver || _cinematic) return;      // never over the game-over screen, never nested
    const Size win = Director::getInstance()->getWinSize();
    const Color3B col = colorFor(PowerUp::Blast);

    std::vector<PZGObstacle*> targets;
    if (platform() && PZCfg::kEnableFx) {
        const float left = riderPos().x - 20.f, right = win.width + 80.f;
        for (int i = 0; i < platform()->obstacleSlots(); ++i) {
            PZGObstacle* o = platform()->obstacleAt(i);
            if (!o || o->dead || !o->sprite || !o->destroyCharacter) continue;
            const Rect b = o->worldBox();
            if (b.getMaxX() > left && b.getMinX() < right) targets.push_back(o);
        }
        std::sort(targets.begin(), targets.end(), [](PZGObstacle* a, PZGObstacle* b) {
            return a->worldBox().getMinX() < b->worldBox().getMinX();
        });
        if (targets.size() > static_cast<size_t>(PZCfg::kBlastMaxTargets)) targets.resize(PZCfg::kBlastMaxTargets);
    }

    if (targets.empty()) {
        // Nothing on screen to showcase: the classic shockwave.
        startWave(riderPos(), PZCfg::kBlastRadius, PZCfg::kBlastWaveSpeed, col);
        flash(Color3B(255, 255, 255), 0.55f, 0.35f);
        shakeScene(14.f, 0.5f);
        freezeFor(PZCfg::kHitStop * 1.2f);
        PZUI::banner(_scene, PZCfg::kStrBlast, "", col, win.height * 0.6f, 0.6f);
        runAction(Sequence::create(DelayTime::create(0.14f), CallFunc::create([]() { PZHaptics::play(PZHaptics::Kind::Medium); }), nullptr));
        return;
    }

    _cinematic = true;
    const float perTarget = PZCfg::kBlastZoomIn + PZCfg::kBlastHold + PZCfg::kBlastGap;
    freezeFor(perTarget * static_cast<float>(targets.size()) + 1.6f);      // safety net; released explicitly below
    setHudVisible(false);
    flash(Color3B(255, 255, 255), 0.5f, 0.3f);
    PZHaptics::play(PZHaptics::Kind::Heavy);
    PZUI::banner(_scene, PZCfg::kStrBlast, "", col, win.height * 0.72f, perTarget * 2.f, kZFx + 40);

    Vector<FiniteTimeAction*> seq;
    seq.pushBack(DelayTime::create(0.25f));
    for (PZGObstacle* target : targets) {
        // Push in on the hazard.
        seq.pushBack(CallFunc::create([this, target]() {
            if (!obstacleAlive(target)) return;
            const Rect b = target->worldBox();
            cameraTo(Vec2(b.getMidX(), b.getMidY()), PZCfg::kBlastZoom, PZCfg::kBlastZoomIn);
        }));
        seq.pushBack(DelayTime::create(PZCfg::kBlastZoomIn));
        // Lock on, then blow it apart.
        seq.pushBack(CallFunc::create([this, target, col]() {
            if (!obstacleAlive(target)) return;
            const Rect b = target->worldBox();
            const Vec2 c(b.getMidX(), b.getMidY());
            auto* reticle = DrawNode::create();
            const float r = std::max(34.f, std::max(b.size.width, b.size.height) * 0.62f);
            reticle->drawCircle(Vec2::ZERO, r, 0.f, 32, false, PZUI::c4f(col, 0.95f), 3.f);
            reticle->drawCircle(Vec2::ZERO, r * 0.55f, 0.f, 24, false, PZUI::c4f(Color3B(255, 255, 255), 0.8f), 1.5f);
            for (int k2 = 0; k2 < 4; ++k2) {
                const float a = CC_DEGREES_TO_RADIANS(90.f * k2);
                const Vec2 d(std::cos(a), std::sin(a));
                reticle->drawSegment(d * (r * 0.7f), d * (r * 1.3f), 2.5f, PZUI::c4f(col, 0.95f));
            }
            reticle->setPosition(c);
            reticle->setScale(2.2f);
            reticle->setRotation(40.f);
            _scene->addChild(reticle, kZFx + 20);
            reticle->runAction(Sequence::create(
                Spawn::create(EaseOut::create(ScaleTo::create(0.16f, 1.f), 2.f), RotateTo::create(0.16f, 0.f), nullptr),
                DelayTime::create(PZCfg::kBlastHold * 0.5f), FadeOut::create(0.08f), RemoveSelf::create(), nullptr));
            PZHaptics::play(PZHaptics::Kind::Light);
        }));
        seq.pushBack(DelayTime::create(PZCfg::kBlastHold * 0.6f));
        seq.pushBack(CallFunc::create([this, target, col]() {
            if (!obstacleAlive(target)) return;
            const Rect b = target->worldBox();
            destroyObstacle(target, Style::Explode);
            ring(Vec2(b.getMidX(), b.getMidY()), col, 170.f, 0.4f);
            flash(Color3B(255, 255, 255), 0.35f, 0.18f);
        }));
        seq.pushBack(DelayTime::create(PZCfg::kBlastHold * 0.4f + PZCfg::kBlastGap));
    }
    // Pull back out and hand control back.
    seq.pushBack(CallFunc::create([this]() { cameraReset(PZCfg::kBlastZoomOut); }));
    seq.pushBack(DelayTime::create(PZCfg::kBlastZoomOut + 0.05f));
    seq.pushBack(CallFunc::create([this, col]() {
        setHudVisible(true);
        _cinematic = false;
        _freezeUntil = std::chrono::steady_clock::time_point{};
        _graceT = std::max(_graceT, 1.0f);
        // Anything beyond the screen still gets wiped by a normal shockwave.
        startWave(riderPos(), PZCfg::kBlastRadius, PZCfg::kBlastWaveSpeed, col);
        PZHaptics::play(PZHaptics::Kind::Success);
    }));
    runAction(Sequence::create(seq));
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

void PZRunDirector::destroyObstacle(PZGObstacle* o, Style style, bool reward) {
    if (!o || o->dead) return;
    spawnGhost(o, style);
    o->dead = true;
    o->deleteReady = true;
    o->setVisible(false);     // the husk is replaced by the ghost animation; matters while the world is frozen
    PZAudio::play(style == Style::Launch ? "fx_smash" : "fx_explosion", 0.9f, 0.05f);
    if (!reward) return;      // rescue clean-up: the same animation, but no coins, stats, hit-stop or shake

    ++_stats.smashed;
    const Rect b = o->worldBox();
    if (PZCfg::kSmashCoins > 0) {
        _scene->addBonusCoins(PZCfg::kSmashCoins);
        _stats.coins += PZCfg::kSmashCoins;
    }
    PZUI::floatText(_scene, PZCfg::kStrSmash, Vec2(b.getMidX(), b.getMaxY() + 18.f), PZCfg::kColTurbo, 10.f, kZFx);
    shakeScene(4.f, 0.2f);
    PZHaptics::play(PZHaptics::Kind::Heavy);
    if (!frozen() && _time - _lastHitStop > 0.5f) {      // rate-limited so ploughing through traffic doesn't stutter
        freezeFor(PZCfg::kHitStop);
        _lastHitStop = _time;
    }
}

// Safe-restart rule: whenever the rider is put back in play (revive, mercy, shield / helmet save, landing after a powered
// ride) every hazard around them is exploded first, in view, so the thing that just killed them can never be waiting
// when control returns. Window is measured from the rider: `behind` covers the killer itself, `ahead` buys reaction time.
int PZRunDirector::clearHazards(float behind, float ahead) {
    if (!platform() || !_scene->character) return 0;
    const float rx = riderPos().x;
    int n = 0;
    for (int i = 0; i < platform()->obstacleSlots(); ++i) {
        PZGObstacle* o = platform()->obstacleAt(i);
        if (!o || o->dead || !o->sprite || !o->destroyCharacter) continue;
        const Rect b = o->worldBox();
        if (b.getMaxX() < rx - behind || b.getMinX() > rx + ahead) continue;
        destroyObstacle(o, Style::Explode, false);
        ++n;
    }
    _kills.clear();
    if (n > 0) {
        flash(Color3B(255, 255, 255), 0.25f, 0.25f);
        PZHaptics::play(PZHaptics::Kind::Medium);
    }
    return n;
}

bool PZRunDirector::mercyAvailable() const {
    return PZCfg::kEnableMercy && _mercyUsed < PZCfg::kMercySaves && PZProgress::shared()->bestScore() < PZCfg::kMercyBestBelow;
}

bool PZRunDirector::onObstacleHit(PZGObstacle* o) {
    if (!o) return false;
    if (_reviveWait || _readyCountdown || _deathPending) {      // untouchable while being put back in play
        destroyObstacle(o, Style::Explode, false);
        return true;
    }
    if (_jetpackT > 0.f) {
        destroyObstacle(o, Style::Explode);
        return true;
    }
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
        PZAudio::play("fx_shield_break");
        destroyObstacle(o, Style::Explode);
        startWave(riderPos(), PZCfg::kShieldBreakRadius, 1600.f, PZCfg::kColShield);
        PZUI::floatText(_scene, PZCfg::kStrShieldBreak, riderPos() + Vec2(0, 56), PZCfg::kColShield, 12.f, kZFx);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 16, 100.f, 0.55f, kZFx);
        flash(PZCfg::kColShield, 0.3f, 0.3f);
        shakeScene(9.f, 0.3f);
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
    // The flash must cover the whole screen even while the camera is zoomed, so it lives on the Scene, not on the layer.
    if (Node* top = _scene->getParent()) {
        if (_flash->getParent() != top) {
            _flash->retain();
            _flash->removeFromParent();
            top->addChild(_flash, 5000);
            _flash->setPosition(Vec2::ZERO);
            _flash->release();
        }
    }
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

// Landing: the impact happens at the tyres, so everything is drawn on the road under the bike (not a ring around it):
// a flat shock ring that races along the road, sparks thrown off both wheels, a short flame lick at each tyre on hard
// landings, and a warm additive flare on the surface.
void PZRunDirector::landingFx(float groundY, float strength) {
    if (!PZCfg::kEnableFx) return;
    strength = std::max(0.f, std::min(1.f, strength));
    const float x = riderPos().x;
    const float wheelDX = 36.f;
    const Color3B hot(255, 196, 64), warm(255, 224, 150), flame(255, 120, 30);

    // 1. flat shock ring along the road
    auto* wave = DrawNode::create();
    wave->setPosition(Vec2(x, groundY + 3.f));
    _scene->addChild(wave, kZAura + 3);
    const float maxR = 80.f + 90.f * strength;
    wave->runAction(ActionFloat::create(0.34f, 0.f, 1.f, [wave, maxR, warm](float t) {
        wave->clear();
        const float r = 16.f + (maxR - 16.f) * (1.f - (1.f - t) * (1.f - t));
        const float a = std::max(0.f, 1.f - t);
        Vec2 prev(r, 0.f);
        for (int i = 1; i <= 24; ++i) {
            const float ang = static_cast<float>(M_PI) * i / 24.f;
            const Vec2 p(std::cos(ang) * r, std::sin(ang) * r * 0.22f);
            wave->drawSegment(prev, p, 1.5f + 2.5f * a, PZUI::c4f(warm, 0.85f * a));
            prev = p;
        }
    }));
    wave->runAction(Sequence::create(DelayTime::create(0.36f), RemoveSelf::create(), nullptr));

    // 2. warm flare on the surface under the bike
    auto* flare = DrawNode::create();
    Vec2 diamond[4] = {Vec2(-70.f, 0.f), Vec2(0.f, 7.f), Vec2(70.f, 0.f), Vec2(0.f, -3.f)};
    flare->drawSolidPoly(diamond, 4, PZUI::c4f(hot, 0.55f + 0.3f * strength), 0.f, PZUI::c4f(hot, 0.f));
    flare->setBlendFunc(BlendFunc::ADDITIVE);
    flare->setPosition(Vec2(x, groundY + 4.f));
    flare->setScale(0.4f, 1.f);
    _scene->addChild(flare, kZAura + 4);
    flare->runAction(Sequence::create(Spawn::create(EaseOut::create(ScaleTo::create(0.3f, 1.4f, 0.6f), 2.f), FadeOut::create(0.3f), nullptr),
                                      RemoveSelf::create(), nullptr));

    // 3. sparks off both tyres (ballistic arcs)
    const int perWheel = 3 + static_cast<int>(strength * 5.f);
    for (int w = 0; w < 2; ++w) {
        const float wx = x + (w == 0 ? -wheelDX : wheelDX);
        for (int i = 0; i < perWheel; ++i) {
            auto* sp = DrawNode::create();
            sp->drawDot(Vec2::ZERO, rnd(1.6f, 2.8f), PZUI::c4f(rnd(0.f, 1.f) < 0.5f ? hot : warm, 1.f));
            const Vec2 start(wx, groundY + 5.f);
            sp->setPosition(start);
            _scene->addChild(sp, kZAura + 5);
            const float dir = (w == 0 ? -1.f : 1.f) * rnd(0.3f, 1.f);
            const Vec2 vel(dir * rnd(60.f, 150.f) * (0.6f + strength), rnd(90.f, 210.f) * (0.6f + 0.6f * strength));
            const float dur = rnd(0.28f, 0.5f);
            sp->runAction(ActionFloat::create(dur, 0.f, dur, [sp, start, vel](float tt) {
                sp->setPosition(start + vel * tt + Vec2(0.f, -520.f * 0.5f * tt * tt));
            }));
            sp->runAction(Sequence::create(Spawn::create(FadeOut::create(dur), ScaleTo::create(dur, 0.3f), nullptr), RemoveSelf::create(), nullptr));
        }
    }

    // 4. flame lick at each tyre on hard landings
    if (strength > 0.35f) {
        for (int w = 0; w < 2; ++w) {
            auto* f = DrawNode::create();
            Vec2 outer[3] = {Vec2(-7.f, 0.f), Vec2(0.f, 26.f), Vec2(7.f, 0.f)};
            Vec2 inner[3] = {Vec2(-3.5f, 0.f), Vec2(0.f, 14.f), Vec2(3.5f, 0.f)};
            f->drawSolidPoly(outer, 3, PZUI::c4f(flame, 0.9f), 0.f, PZUI::c4f(flame, 0.f));
            f->drawSolidPoly(inner, 3, PZUI::c4f(Color3B(255, 236, 120), 0.95f), 0.f, PZUI::c4f(hot, 0.f));
            f->setBlendFunc(BlendFunc::ADDITIVE);
            f->setPosition(Vec2(x + (w == 0 ? -wheelDX : wheelDX), groundY + 3.f));
            f->setScale(0.3f, 0.2f);
            _scene->addChild(f, kZAura + 5);
            f->runAction(Sequence::create(Spawn::create(EaseOut::create(ScaleTo::create(0.26f, 1.0f + 0.5f * strength, 0.9f + 0.7f * strength), 2.f),
                                                        FadeOut::create(0.26f), nullptr),
                                          RemoveSelf::create(), nullptr));
        }
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
                // Absolute scaling: a second coin arriving mid-pulse used to leave the label bigger every time.
                Label* lbl = _scene->coins_label;
                if (_coinLabelBase.lengthSquared() < 0.0001f) _coinLabelBase = Vec2(lbl->getScaleX(), lbl->getScaleY());
                lbl->stopActionByTag(77);
                lbl->setScale(_coinLabelBase.x * 1.28f, _coinLabelBase.y * 1.28f);
                auto* a = Sequence::create(ScaleTo::create(0.12f, _coinLabelBase.x, _coinLabelBase.y), nullptr);
                a->setTag(77);
                lbl->runAction(a);
                // Glitter: a couple of gold sparks popping off the new total.
                const Vec2 at = lbl->getParent()->convertToWorldSpace(lbl->getPosition()) + Vec2(10.f, 0.f);
                PZUI::burst(_scene, at, PZCfg::kColAccent, 4, 26.f, 0.32f, kZFx + 4);
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
    // Blink the rider while untouchable (after a save / revive) so the protection is visible.
    if (_scene->character && _scene->character->s_body) {
        const bool protectedNow = _graceT > 0.f;
        const uint8_t op = protectedNow && std::fmod(_auraClock, 0.22f) < 0.11f ? 110 : 255;
        _scene->character->s_body->setOpacity(op);
    }
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
    _jetpackAura->setVisible(_jetpackT > 0.f);
    if (_jetpackT > 0.f) {
        _jetpackAura->setPosition(rp);
        _jetpackAura->setScale(1.f + 0.06f * std::sin(_auraClock * 11.f));
        _jetpackAura->setOpacity(_jetpackT < 1.5f && std::fmod(_auraClock, 0.25f) < 0.12f ? 110 : 255);
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
    if (!turboActive() && _jetpackT <= 0.f) {
        if (_prevJump == 0 && jc != 0 && ch->p_body && ch->p_body->GetLinearVelocity().y > 0.f) {
            dustPuff(Vec2(riderPos().x - 30.f, groundY + 6.f), 4, 70.f);
            PZHaptics::play(PZHaptics::Kind::Light);
            PZAudio::play("fx_jump", 0.7f, 0.12f);
            _airTime = 0.f;
        } else if (_prevJump != 0 && jc == 0) {
            const float strength = std::min(1.f, _airTime / 0.7f);
            dustPuff(Vec2(riderPos().x, groundY + 6.f), 4 + static_cast<int>(strength * 5.f), 110.f);
            if (_airTime > 0.25f) {
                PZAudio::play("fx_land", 0.5f + 0.5f * strength, 0.1f);
                landingFx(groundY, strength);
                shakeScene(1.5f + 2.5f * strength, 0.12f);
                PZHaptics::play(strength > 0.55f ? PZHaptics::Kind::Medium : PZHaptics::Kind::Light);
            }
            if (PZCfg::kEnableBigAir && _airTime >= PZCfg::kBigAirMin) {
                char buf[40];
                std::snprintf(buf, sizeof(buf), PZCfg::kStrBigAir, PZCfg::kBigAirCoins);
                awardBonus(PZCfg::kBigAirCoins, Vec2(riderPos().x, riderPos().y + 58.f), buf, PZCfg::kColAccent2);
                PZUI::burst(_scene, riderPos(), PZCfg::kColAccent, 16, 100.f, 0.55f, kZFx);
                PZUI::glitter(_scene, riderPos(), PZCfg::kColAccent, 12, 90.f, 0.8f, kZFx + 5);
                PZAudio::play("fx_bigair");
                PZAudio::say("vo_big_air", 0);
                cameraPunch(1.0f + (PZCfg::kPunchZoom - 1.0f) * 0.6f, 0.1f, 0.3f);
                PZHaptics::play(PZHaptics::Kind::Success);
            }
        }
        _airTime = (jc != 0) ? _airTime + dt : 0.f;
    }
    _prevJump = jc;

    // Running dust at the rear wheel while grounded.
    if (jc == 0 && !turboActive() && _jetpackT <= 0.f) {
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
    if (_skipDeathOnce) {              // the deferred decision below already ran and chose a real game over
        _skipDeathOnce = false;
        return false;
    }
    if (_deathDeferred) return true;
    // Death signals come from the contact listener, i.e. in the middle of b2World::Step. Every rescue moves the rider's
    // body (SetTransform), which Box2D forbids while the world is locked, so take the decision one frame later.
    if (_scene->world && _scene->world->IsLocked()) {
        _deathDeferred = true;
        retain();
        Director::getInstance()->getScheduler()->runOnAxmolThread([this]() {
            _deathDeferred = false;
            if (_scene && !_scene->gameOver) {
                if (!onDeath()) {
                    _skipDeathOnce = true;
                    _scene->gameOverCallBack(true);
                }
            }
            release();
        });
        return true;
    }
    // A second death signal (hit + fall in the same frame, two obstacles at once) must not stack a game-over screen on
    // top of a revive in progress.
    if (_deathPending) return true;
    if (_reviveWait || _readyCountdown) {
        rescueRider();
        return true;
    }
    if (_graceT > 0.f) {
        rescueRider();
        return true;
    }
    // An active shield saves the rider from one fall (hits are already absorbed in onObstacleHit).
    if (_shieldT > 0.f) {
        _shieldT = 0.f;
        PZAudio::play("fx_shield_break");
        PZAudio::play("fx_revive");
        rescueRider();
        PZUI::floatText(_scene, PZCfg::kStrShieldBreak, riderPos() + Vec2(0, 56), PZCfg::kColShield, 12.f, kZFx);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 14, 90.f, 0.5f, kZFx);
        flash(PZCfg::kColShield, 0.3f, 0.3f);
        PZHaptics::play(PZHaptics::Kind::Warning);
        return true;
    }
    // Lucky Helmet gadget: one free hit/fall per run.
    if (_helmetSaves > 0) {
        --_helmetSaves;
        PZAudio::play("fx_helmet");
        PZAudio::play("fx_revive");
        refreshGadgetStrip();
        rescueRider();
        PZUI::banner(_scene, PZCfg::kStrLuckyHelmet, "", PZCfg::kColShield,
                     Director::getInstance()->getWinSize().height * 0.6f, 0.8f);
        PZUI::burst(_scene, riderPos(), PZCfg::kColShield, 14, 90.f, 0.5f, kZFx);
        flash(PZCfg::kColShield, 0.3f, 0.35f);
        PZHaptics::play(PZHaptics::Kind::Warning);
        return true;
    }
    // Beginners get a free second chance so early runs don't end on the first mistake.
    if (mercyAvailable()) {
        ++_mercyUsed;
        PZAudio::play("fx_revive");
        rescueRider();
        PZUI::banner(_scene, PZCfg::kStrSaved, "", PZCfg::kColGood, Director::getInstance()->getWinSize().height * 0.6f, 0.8f);
        PZUI::burst(_scene, riderPos(), PZCfg::kColGood, 14, 90.f, 0.5f, kZFx);
        flash(PZCfg::kColGood, 0.3f, 0.35f);
        PZHaptics::play(PZHaptics::Kind::Warning);
        return true;
    }

    flash(PZCfg::kColBad, 0.4f, 0.5f);
    PZHaptics::play(PZHaptics::Kind::Error);
    PZAudio::play("fx_crash");
    auto* sc = PZSettingsController::shared();
    const bool video = PZCfg::kEnableReviveVideo;
    const bool offerRevive = PZCfg::kEnableRevive && !_reviveOffered &&
                             (sc->coins >= PZCfg::kReviveCost || video);
    if (offerRevive) {
        if (video) PZ::AdsController::shared()->requestRewarded();
        _reviveOffered = true;
    }
    _deathPending = offerRevive;

    _scene->gameOver = true;     // freeze the world while the offer is up
    auto afterDeathShow = [this, video, offerRevive]() {
        if (!_scene) return;
        if (offerRevive) {
            PZPopups::showRevive(_scene, PZCfg::kReviveCost, PZSettingsController::shared()->coins, video,
                                 [this]() { reviveAccepted(true); }, [this]() { reviveFromVideo(); },
                                 [this]() { reviveDeclined(); });
        } else {
            _scene->gameOverCallBack(true);
        }
    };
    beginDeathCinematic(afterDeathShow);
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
        ch->update(0.f);   // the world is held during the countdown: draw the rider where it was parked
        ch->jump_counter = 1;
    }
    clearHazards(240.f * kDeviceScale(), 950.f * kDeviceScale());
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
    _deathPending = false;
    _readyCountdown = false;

    _shieldT = 0.f;
    _graceT = std::max(_graceT, 6.f);                         // untouchable until the countdown has finished
    clearHazards(280.f * kDeviceScale(), 1100.f * kDeviceScale());   // the killer goes first, visibly
    _scene->gameOver = false;
    _scene->gamePause = false;

    // Hold the rider until a platform slides underneath (see updateReviveWait), instead of dropping them back onto
    // the same gap.
    if (PZGCharacter* ch = _scene->character) {
        if (ch->p_body) {
            ch->p_body->SetGravityScale(0.f);
            ch->p_body->SetLinearVelocity(b2Vec2(0.f, 0.f));
        }
        ch->jump_counter = 1;
    }
    _reviveWait = true;
    _reviveWaitT = 0.f;
    PZAudio::play("fx_revive");

    flash(Color3B(255, 255, 255), 0.4f, 0.35f);
    PZHaptics::play(PZHaptics::Kind::Success);
}

void PZRunDirector::reviveDeclined() {
    _deathPending = false;
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
    _jetpackAura->setVisible(false);
    if (_gadgetStrip) _gadgetStrip->setVisible(false);
    _comboLabel->setVisible(false);
    _magnetT = _shieldT = _turboT = _jetpackT = 0.f;
    PZProgress::shared()->snapshot();
    const PZRunResult result = PZProgress::shared()->commitRun(_stats);
    _lastResult = result;
    if (!result.newBest) PZAudio::say("vo_game_over", 1);
    refreshHud();
    auto claimDoubleCoins = [this]() -> int {
        const int extra = static_cast<int>(std::lround(_stats.coins * (PZCfg::kDoubleCoinsMultiplier - 1.f)));
        if (extra <= 0) return 0;
        if (_scene) _scene->addBonusCoins(extra);
        PZSettingsController::shared()->save();
        return extra;
    };
    std::function<void()> onCoins, onVideo;
    if (PZCfg::kEnableRevive && !_stats.revived) {
        onCoins = [this]() { reviveFromGameOver(true); };
        onVideo = [this]() { reviveFromGameOver(false); };
    }
    PZPopups::attachResults(gameOverLayer, distanceLabel, _stats, result, claimDoubleCoins, onCoins, onVideo);
}

void PZRunDirector::reviveFromGameOver(bool withCoins) {
    if (_gameOverBusy || !_committed || _stats.revived || !_scene) return;
    PZ::AdsController::shared()->cancelPendingInterstitial();
    auto* sc = PZSettingsController::shared();
    const Size win = Director::getInstance()->getWinSize();
    if (withCoins) {
        if (sc->coins < PZCfg::kReviveCost) {
            PZUI::banner(_scene, PZCfg::kStrReviveNoCoinsShort, "", PZCfg::kColBad, win.height * 0.6f, 0.9f, 1600);
            PZHaptics::play(PZHaptics::Kind::Error);
            return;
        }
        resumeFromGameOver(true);
        return;
    }
    _gameOverBusy = true;
    PZ::AdsController::shared()->showRewardedWithCallback([this](bool earned) {
        _gameOverBusy = false;
        if (earned) {
            resumeFromGameOver(false);
        } else if (_scene) {
            PZUI::banner(_scene, PZCfg::kStrVideoNoAd, "", PZCfg::kColBad,
                         Director::getInstance()->getWinSize().height * 0.6f, 1.0f, 1600);
        }
    });
}

void PZRunDirector::resumeFromGameOver(bool paidWithCoins) {
    if (!_committed || !_scene) return;
    PZ::AdsController::shared()->cancelPendingInterstitial();
    // Roll the committed run back: best/xp/missions return to the pre-run state, and the level-up coins are taken out.
    PZProgress::shared()->restoreSnapshot();
    auto* sc = PZSettingsController::shared();
    if (_lastResult.levelUpCoins > 0) sc->coins = std::max(0, static_cast<int>(sc->coins) - _lastResult.levelUpCoins);
    sc->save();
    _lastResult = PZRunResult();
    _committed = false;
    _reviveOffered = true;     // one revive per run

    // Take the results overlay down and give the field back.
    if (Node* overlay = _scene->getChildByTag(10)) overlay->removeFromParent();
    if (Menu* menu = dynamic_cast<Menu*>(_scene->getChildByTag(kBaseMenuItemTag))) {
        for (int tag : {6, 7, 8}) {
            if (auto* item = dynamic_cast<MenuItem*>(menu->getChildByTag(tag))) item->setVisible(true);
        }
        if (auto* pause = dynamic_cast<MenuItem*>(menu->getChildByTag(kUIIDGameFieldPauseButton))) pause->setEnabled(true);
    }
    if (_gadgetStrip) _gadgetStrip->setVisible(true);
    if (_comboLabel) _comboLabel->setVisible(true);
    if (_tracker) _tracker->setVisible(true);
    if (_scene->character) _scene->character->reset();

    reviveAccepted(paidWithCoins);
}
