//
//  PZRunDirector.h
//  Per-run "action layer" bolted onto the runner: stages, combos, near-miss bonuses, power-ups
//  (magnet / shield / turbo / blast), obstacle destruction, revive & mercy saves, haptics, screen juice and
//  run statistics.
//
//  It only observes the existing game through a few narrow hooks (see PZGGameFieldScene), draws
//  everything in code, and reads all tuning/text/colours from PZGameConfig.h (overridable from the
//  `metaFeatures` section of CompleteGameInfo.plist), so a reskin never has to touch it.
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZGameConfig.h"
#include "PZProgress.h"
#include <chrono>
#include <string>
#include <vector>

class PZGGameFieldScene;
class PZGGamefieldPlatforms;
class PZGObstacle;
class PZGCoin;

class PZRunDirector : public ax::Node {
public:
    enum class PowerUp { Magnet = 0, Shield = 1, Turbo = 2, Blast = 3, Jetpack = 4 };

    static PZRunDirector* create(PZGGameFieldScene* scene);

    // Lifecycle -------------------------------------------------------------------------------
    void onLoaded();            // HUD anchors (labels) exist now
    void startRun();            // called for the first run and on every restart
    void update(float dt) override;   // driven by the owning scene while the run is live

    // Hooks from the game ---------------------------------------------------------------------
    void onCoin(PZGCoin* coin);
    bool onObstacleHit(PZGObstacle* obstacle);    // true = hit absorbed (shield / turbo / grace / mercy)
    bool onDeath();                                // true = rescued or a revive offer is up, defer game over
    void presentResults(ax::Node* gameOverLayer, ax::Label* distanceLabel);

    // Queries ---------------------------------------------------------------------------------
    float speedMultiplier() const { return _boost; }
    bool  turboActive() const { return _turboT > 0.f; }
    bool  frozen() const;      // brief hit-stop: the owning scene skips its simulation step while true

    // Gives the rider a power-up right now (also usable for rewards).
    void grant(PowerUp kind);
    void grantByName(const std::string& name);
    int  coinsEarned() const { return _stats.coins; }

    void reviveAccepted(bool paidWithCoins);
    void reviveFromVideo();
    void reviveDeclined();

private:
    enum class Style { Explode, Launch };

    struct Orb {
        ax::Node* node = nullptr;
        PowerUp kind = PowerUp::Magnet;
    };
    struct Wave {                      // expanding shockwave that destroys obstacles it reaches
        ax::Vec2 origin;
        float radius = 0.f;
        float maxRadius = 0.f;
        float speed = 0.f;
        ax::Color3B colour;
        ax::DrawNode* ring = nullptr;
    };
    struct PendingKill {               // obstacle queued to die (staggered chain explosions)
        PZGObstacle* obstacle = nullptr;
        float delay = 0.f;
        Style style = Style::Explode;
    };

    bool initDirector(PZGGameFieldScene* scene);
    PZGGamefieldPlatforms* platform() const;
    ax::Vec2 riderPos() const;
    ax::Rect riderBox() const;
    float apex() const;

    void clearRunVisuals();
    void layoutHud();
    void refreshHud();
    void refreshGadgetStrip();
    void updateMissionTracker(float dt);
    void updateStage();
    void updateCombo(float dt);
    void updatePowerUps(float dt);
    void updateOrbs(float dt, float scroll);
    void updateMagnet(float dt);
    void updateNearMiss();
    void updateTurboRide(float dt);
    void updateJetpackRide(float dt);
    void applyGadgets();
    void endJetpack();
    void updateWaves(float dt);
    void updateKills(float dt);
    void updateSpeedLines(float dt);
    void updateAuras(float dt);
    void updateFx(float dt, float scroll);

    void spawnOrb();
    void collect(PowerUp kind, const ax::Vec2& at, float durationOverride = 0.f);
    void endTurbo();
    void rescueRider();      // put the rider back on solid ground (revive / shield save / mercy)
    bool mercyAvailable() const;

    // Obstacle destruction
    void destroyObstacle(PZGObstacle* o, Style style);
    void queueKill(PZGObstacle* o, float delay, Style style);
    bool obstacleAlive(PZGObstacle* o) const;
    void startWave(const ax::Vec2& origin, float maxRadius, float speed, const ax::Color3B& colour);
    void spawnGhost(PZGObstacle* o, Style style);

    void awardBonus(int coins, const ax::Vec2& at, const std::string& text, const ax::Color3B& color);

    // FX helpers
    void flash(const ax::Color3B& colour, float alpha, float duration);
    void ring(const ax::Vec2& at, const ax::Color3B& colour, float radius, float duration);
    void dustPuff(const ax::Vec2& at, int count, float spread);
    void coinFly(const ax::Vec2& from);
    void freezeFor(float seconds);
    void riderAfterimage(const ax::Color3B& tint);

    ax::Node* makeOrbVisual(PowerUp kind, float scale = 1.f) const;
    static ax::Color3B colorFor(PowerUp kind);
    static const char* nameFor(PowerUp kind);
    void playPickupSound();

    PZGGameFieldScene* _scene = nullptr;

    // run state
    float _time = 0.f;
    float _nextOrbAt = 0.f;
    int   _stage = 1;
    float _boost = 1.f;
    float _magnetT = 0.f, _shieldT = 0.f, _turboT = 0.f, _graceT = 0.f;
    float _magnetMax = 1.f, _shieldMax = 1.f, _turboMax = 1.f;
    float _jetpackT = 0.f, _jetpackMax = 1.f, _jetpackLockY = 0.f;
    float _jetpackCoinClock = 0.f;
    int   _helmetSaves = 0;
    int   _combo = 0;
    float _comboT = 0.f;
    bool  _reviveOffered = false;
    bool  _committed = false;
    int   _mercyUsed = 0;
    float _riderOffset = 0.f;     // rider origin above the platform surface while grounded
    bool  _haveOffset = false;
    float _turboLockY = 0.f;
    int   _prevJump = 0;
    float _airTime = 0.f;
    float _dustClock = 0.f;
    float _trailClock = 0.f;
    float _auraClock = 0.f;
    float _coinTickClock = 0.f;
    int   _flying = 0;            // coins currently flying to the HUD
    std::chrono::steady_clock::time_point _freezeUntil{};
    PZRunStats _stats;
    std::vector<Orb> _orbs;
    std::vector<Wave> _waves;
    std::vector<PendingKill> _kills;

    // visuals
    ax::LayerColor* _tint = nullptr;
    ax::LayerColor* _flash = nullptr;
    ax::Label* _bestLabel = nullptr;
    ax::Label* _comboLabel = nullptr;
    ax::Node* _hud = nullptr;
    struct HudRow { ax::Node* root = nullptr; ax::DrawNode* bar = nullptr; ax::Node* icon = nullptr; ax::Label* name = nullptr; };
    HudRow _rows[5];
    ax::Node* _gadgetStrip = nullptr;
    ax::Node* _tracker = nullptr;
    ax::Label* _trackerText = nullptr;
    ax::Label* _trackerCount = nullptr;
    ax::DrawNode* _trackerBar = nullptr;
    float _trackerClock = 0.f;
    int   _trackerSlot = -1;
    bool  _trackerDone[PZCfg::kMissionsPerDay] = {false, false, false};
    ax::DrawNode* _shieldAura = nullptr;
    ax::DrawNode* _magnetAura = nullptr;
    ax::DrawNode* _turboAura = nullptr;
    ax::DrawNode* _jetpackAura = nullptr;
    std::vector<ax::DrawNode*> _speedLines;
    std::vector<ax::DrawNode*> _motes;
};
