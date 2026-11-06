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
#include <functional>
#include <string>
#include <vector>

class PZGGameFieldScene;
class PZGGamefieldPlatforms;
class PZGObstacle;
class PZGCoin;

class PZRunDirector : public ax::Node {
public:
    enum class PowerUp { Magnet = 0, Shield = 1, Turbo = 2, Blast = 3, Jetpack = 4, Rocket = 5, Gun = 6 };

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
    bool  deathPending() const { return _deathPending; }   // a revive offer / revive sequence owns the end of the run
    bool  frozen() const;      // brief hit-stop: the owning scene skips its simulation step while true
    bool  inputLocked() const { return _readyCountdown || _cinematic; }   // taps are ignored during countdown / cinematics

    // Gives the rider a power-up right now (also usable for rewards).
    void grant(PowerUp kind);
    void grantByName(const std::string& name);
    int  coinsEarned() const { return _stats.coins; }
    void extendGrace(float seconds) { _graceT = std::max(_graceT, seconds); }   // untouchable (blinking) for at least this long

    void reviveAccepted(bool paidWithCoins);
    void reviveFromVideo();
    void reviveDeclined();

    // Game-over screen: REVIVE for coins / WATCH AD. Either rolls the committed run back and resumes it.
    void reviveFromGameOver(bool withCoins);

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
    void updateRocketRide(float dt);
    void updateGun(float dt);
    void updateReviveWait(float dt);
    void updateGraceBridge(float dt);   // blinking (protected) rider never sinks into a pit
    bool _graceHover = false;
    float _graceRoadY = 0.f;            // rider height on the last road it rode (where the bridge holds it)
    bool  _haveRoadY = false;
    void beginBlastCinematic();          // freeze, zoom on every obstacle in turn, destroy, zoom back
    void cameraTo(const ax::Vec2& focus, float scale, float duration, const ax::Vec2* screenAnchor = nullptr);
    void cameraPunch(float scale, float inSeconds, float outSeconds);   // quick zoom pulse about the rider
    void cameraReset(float duration);
    void shakeScene(float amplitude, float duration);   // shake that respects the current camera position
    void setHudVisible(bool visible);
    // 3-2-1 with the world held, then the run resumes. `revive` also grants the revive turbo + clear.
    void beginReadyCountdown(bool revive = true);
    // Start-of-run show: the bike rides in from the left while the camera is pushed in on it, the HUD arrives with
    // the countdown and the crowd cheers on GO.
    void beginIntro();
    void startIntroNow();
    void animateHudIn();
    // Death show: hold, push in on the wreck, fire + disappointed crowd, pull back (then the revive offer).
    void beginDeathCinematic(std::function<void()> completion);
    void openCinematicFrame();
    void closeCinematicFrame(float duration);
    bool groundRiderNow();               // snap the rider onto the platform under it and re-enable taps
    void startRocket();
    void endRocket();
    void fireAt(PZGObstacle* target);
    bool safeLandHere(float& outY);      // true when there is platform under the rider
    float cruiseLine();                  // y a powered ride should hold (ground-relative, never in a pit)
    bool  nearestSurface(float& outY);   // surface height under the rider, else the closest platform ahead/behind
    bool  runwayAhead(float need);       // solid ground under the rider that keeps going for `need` units
    float _lastGroundY = 0.f;            // last known platform surface (fallback while over a gap)
    bool  _haveGroundY = false;
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
    void endGun();
    void resumeFromGameOver(bool paidWithCoins);
    void rescueRider();      // put the rider back on solid ground (revive / shield save / mercy)
    bool mercyAvailable() const;

    // Obstacle destruction
    void destroyObstacle(PZGObstacle* o, Style style, bool reward = true);
    int  clearHazards(float behind, float ahead);   // visibly blow up every hazard in a window around the rider (no reward)
    void queueKill(PZGObstacle* o, float delay, Style style);
    bool obstacleAlive(PZGObstacle* o) const;
    void startWave(const ax::Vec2& origin, float maxRadius, float speed, const ax::Color3B& colour);
    void spawnGhost(PZGObstacle* o, Style style);

    void awardBonus(int coins, const ax::Vec2& at, const std::string& text, const ax::Color3B& color);

    // FX helpers
    void flash(const ax::Color3B& colour, float alpha, float duration);
    void ring(const ax::Vec2& at, const ax::Color3B& colour, float radius, float duration);
    void dustPuff(const ax::Vec2& at, int count, float spread);
    void landingFx(float groundY, float strength);   // flat shock ring on the road, wheel sparks + flames, ground flare
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
    float _rocketT = 0.f, _rocketMax = 1.f;
    float _rocketLockY = 0.f;
    float _gunT = 0.f, _gunMax = 1.f;
    float _gunClock = 0.f;
    ax::Node* _rocketArt = nullptr;      // rocket pod strapped to the bike's side while it burns
    ax::DrawNode* _rocketFlame = nullptr;
    ax::DrawNode* _gunArt = nullptr;     // cannon mounted in front of the bike while gun mode is live
    ax::DrawNode* _gunFlash = nullptr;
    bool  _deathDeferred = false;        // a death signal arrived mid physics step: decide it at the next safe point
    bool  _skipDeathOnce = false;
    bool  _deathPending = false;         // revive popup / video / hover / countdown in progress: ignore further deaths
    bool  _reviveWait = false;           // holding the rider in the air until a platform arrives
    float _reviveWaitT = 0.f;
    int   _helmetSaves = 0;
    int   _combo = 0;
    float _comboT = 0.f;
    bool  _reviveOffered = false;
    bool  _committed = false;
    PZRunResult _lastResult;             // what commitRun granted (rolled back if the player revives from game over)
    bool  _gameOverBusy = false;         // a rewarded ad is showing for the game-over revive
    int   _mercyUsed = 0;
    bool  _readyCountdown = false;
    bool  _cinematic = false;
    bool  _introActive = false;          // start-of-run show is running
    bool  _introTip1Visible = false;
    bool  _introTip2Visible = false;
    ax::Node* _cinematicFrame = nullptr;
    ax::Vec2 _camPos = ax::Vec2::ZERO;
    float _camScale = 1.f;
    std::vector<std::pair<ax::Node*, bool>> _hiddenHud;   // nodes hidden during a cinematic (restored afterwards)
    float _lastHitStop = -10.f;
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
    ax::Vec2 _coinLabelBase;      // resting scale of the coin counter (pulses are absolute, never compounded)
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
    struct HudRow {
        ax::Node* root = nullptr;
        ax::DrawNode* bar = nullptr;
        ax::Node* icon = nullptr;
        ax::Label* name = nullptr;
        ax::Label* time = nullptr;      // remaining seconds, right of the name
    };
    // One chip per timed power-up, in this fixed order (HUD slot i always shows chipKind(i)).
    static constexpr int kChipCount = 6;
    static PowerUp chipKind(int i) {
        static const PowerUp kinds[kChipCount] = {PowerUp::Magnet, PowerUp::Shield, PowerUp::Turbo,
                                                  PowerUp::Jetpack, PowerUp::Rocket, PowerUp::Gun};
        return kinds[i];
    }
    HudRow _rows[kChipCount];
    ax::Node* _gadgetStrip = nullptr;
    // In-run gadget bar: big round tappable buttons along the top. Nothing is equipped at the start of a run; the
    // player taps a gadget when they want it (one stored charge per tap).
    struct GadgetBtn {
        std::string id;
        int pool = -1;
        ax::Vec2 pos;                 // centre, in the strip's space
        float r = 30.f;
        ax::DrawNode* ring = nullptr; // progress / ready ring
        ax::Label* count = nullptr;   // charges left
        ax::Node* root = nullptr;
    };
    std::vector<GadgetBtn> _gadgetBtns;
    int   _gadgetPress = -1;
    float _headstartFlash = 0.f;
    void  activateGadget(int btn);
    void  updateGadgetStrip(float dt);
    bool  gadgetState(const std::string& id, float& frac) const;   // true while the gadget's effect is running
    static constexpr int kGadgetStartTag = 7711;
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
