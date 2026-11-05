//
//  PZRunDirector.h
//  Per-run "action layer" bolted onto the runner: stages, combos, near-miss bonuses, power-ups
//  (magnet / shield / turbo), revive, screen juice and run statistics.
//
//  It only observes the existing game through a few narrow hooks (see PZGGameFieldScene), draws
//  everything in code, and reads all tuning/text/colours from PZGameConfig.h, so a reskin never
//  has to touch it.
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZGameConfig.h"
#include "PZProgress.h"
#include <vector>

class PZGGameFieldScene;
class PZGGamefieldPlatforms;
class PZGObstacle;
class PZGCoin;

class PZRunDirector : public ax::Node {
public:
    enum class PowerUp { Magnet = 0, Shield = 1, Turbo = 2 };

    static PZRunDirector* create(PZGGameFieldScene* scene);

    // Lifecycle -------------------------------------------------------------------------------
    void onLoaded();            // HUD anchors (labels) exist now
    void startRun();            // called for the first run and on every restart
    void update(float dt) override;   // driven by the owning scene while the run is live

    // Hooks from the game ---------------------------------------------------------------------
    void onCoin(PZGCoin* coin);
    bool onObstacleHit(PZGObstacle* obstacle);    // true = hit absorbed (shield / turbo / grace)
    bool onDeath();                                // true = a revive offer is up, defer game over
    void presentResults(ax::Node* gameOverLayer, ax::Label* distanceLabel);

    // Queries ---------------------------------------------------------------------------------
    float speedMultiplier() const { return _boost; }
    bool  turboActive() const { return _turboT > 0.f; }

    // Gives the rider a power-up right now (also usable for rewards).
    void grant(PowerUp kind);

    void reviveAccepted();
    void reviveDeclined();

private:
    struct Orb {
        ax::Node* node = nullptr;
        PowerUp kind = PowerUp::Magnet;
    };

    bool initDirector(PZGGameFieldScene* scene);
    PZGGamefieldPlatforms* platform() const;
    ax::Vec2 riderPos() const;
    ax::Rect riderBox() const;

    void clearRunVisuals();
    void layoutHud();
    void refreshHud();
    void updateStage();
    void updateCombo(float dt);
    void updatePowerUps(float dt);
    void updateOrbs(float dt, float scroll);
    void updateMagnet(float dt);
    void updateNearMiss();
    void updateTurboRide(float dt);
    void updateSpeedLines(float dt);
    void updateAuras(float dt);

    void spawnOrb();
    void collect(PowerUp kind, const ax::Vec2& at);
    void endTurbo();
    void rescueRider();      // put the rider back on solid ground (revive / shield save)
    void smash(PZGObstacle* o);
    void awardBonus(int coins, const ax::Vec2& at, const std::string& text, const ax::Color3B& color);
    ax::Node* makeOrbVisual(PowerUp kind) const;
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
    int   _combo = 0;
    float _comboT = 0.f;
    bool  _reviveOffered = false;
    bool  _committed = false;
    float _riderOffset = 0.f;     // rider origin above the platform surface while grounded
    bool  _haveOffset = false;
    float _turboLockY = 0.f;
    PZRunStats _stats;
    std::vector<Orb> _orbs;

    // visuals
    ax::LayerColor* _tint = nullptr;
    ax::Label* _bestLabel = nullptr;
    ax::Label* _comboLabel = nullptr;
    ax::Node* _hud = nullptr;
    ax::DrawNode* _hudBars[3] = {nullptr, nullptr, nullptr};
    ax::Label* _hudNames[3] = {nullptr, nullptr, nullptr};
    ax::DrawNode* _shieldAura = nullptr;
    ax::DrawNode* _magnetAura = nullptr;
    ax::DrawNode* _turboAura = nullptr;
    std::vector<ax::DrawNode*> _speedLines;
    float _auraClock = 0.f;
    float _trailClock = 0.f;
};
