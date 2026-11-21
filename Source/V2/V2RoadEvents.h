//
//  V2RoadEvents.h
//  Version 2 "the road tells jokes": a deterministic (seeded) director that runs short, readable road events on top of the
//  existing runner. See versions/v2/PRODUCT_PLAN.md sections 5 and 6.
//
//  Design rules (from the plan, enforced here):
//   * one event at a time, shuffle bag + breathers between events, first event only after a quiet opening;
//   * every hazard is telegraphed (sign / bark) and is spawned only on flat, clear road at the right screen edge, so an
//     entry is never blind and never over a gap;
//   * hazards and rivals are KINEMATIC actors (no Box2D bodies): they follow the road height from the platform layer, hop
//     gaps on a ballistic arc, and hit-test with simple boxes;
//   * the director never decides what a hit means: it asks the host (PZRunDirector) whether shield / turbo / grace absorbed
//     it, and only then reports a lethal hit;
//   * everything is removed by clear() (death, restart, scene exit) and shifted by shiftWorld() (revive glide).
//
#pragma once

#include "PZLegacyCompat.h"
#include "V2/V2Core.h"
#include "V2/V2Config.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class PZGGameFieldScene;
class PZGGamefieldPlatforms;

namespace V2 {

enum class EventId { DinoCrossing = 0, PizzaPanic = 1, CoolKidRace = 2, ConstructionChaos = 3, MeteorShower = 4, GrandmaTurbo = 5, TrexChase = 6 };
constexpr int kEventCount = 6;      // events in the shuffle bag
constexpr int kEventTotal = 7;      // + the T-Rex chase: a chapter climax, scheduled by success count, never drawn from the bag
const char* eventName(EventId id);

// Why an event ended. Technical skips and run endings are NOT player failures (they would poison the analytics).
enum class Outcome { Success, Failed, SkippedNoRoad, AbortedDeath, CancelledScene };
const char* outcomeName(Outcome o);

// Variant difficulty, chosen from distance (tuning: v2/tierStandardAt, v2/tierExpertAt).
enum class Tier { Intro = 0, Standard = 1, Expert = 2 };
const char* tierName(Tier t);

struct Context {
    float dt = 0.f;
    float scroll = 0.f;                 // world units scrolled this frame (what the platforms moved)
    ax::Vec2 rider;
    ax::Rect riderBox;
    bool airborne = false;
    float apex = 160.f;                 // rider jump apex above the road, scene units
    float distance = 0.f;               // run distance (metres) - picks the difficulty tier
    bool ridesActive = false;           // jetpack / turbo / rocket owns the rider: no new events
    bool controlLocked = false;         // countdown / cinematic / revive glide: actors hold still, no hits
};

struct Host {
    std::function<bool(const ax::Vec2&)> absorbHit;                                   // true = shield / turbo / grace took it
    std::function<void()> lethal;                                                     // the hit kills the rider
    std::function<void(int, const ax::Vec2&, const std::string&)> award;              // coins + floating text
    std::function<void(float, float)> shake;
};

class RoadEventDirector : public ax::Node {
public:
    static RoadEventDirector* create(PZGGameFieldScene* scene, ax::Node* hud);

    void setHost(Host h) { _host = std::move(h); }
    void reset(uint32_t seed);                    // new run (also clears)
    void clear(Outcome reason = Outcome::CancelledScene);   // remove every actor, bubble and marker; an unfinished event is logged with `reason`
    void update(const Context& ctx);
    void shiftWorld(float dx);                    // the world slid left by dx (revive / ride landing)
    void clearNear(float behind, float ahead);    // revive / blast: hazards around the rider fly away (no hit)
    void notifyBoost(float amount);               // coin / near miss / long jump: helps in a rival race
    void setEnabled(bool on) { _enabled = on; if (!on) clear(); }
    bool busy() const { return _active || !_actors.empty(); }
    bool forceStart(const std::string& name);     // debug / QA: start a specific event now
    uint32_t seed() const { return _seed; }
    int successCount() const { return _successes; }   // events finished without a hit this run (daily score)
    struct RunStats { int eventCoins = 0, smooth = 0, boxesCaught = 0, boxesMissed = 0, objectsCollected = 0, microDone = 0, closeDodges = 0; bool trexEscaped = false, rivalBeaten = false; };
    const RunStats& runStats() const { return _rs; }
    void notifySmoothLanding();                   // the rider landed smoothly after crossing a gap
    void notifyPowerUp();                         // a power-up was picked up
    void setPizzaBoost(bool on) { _pizzaBoost = on; }   // PIZZA EXPRESS: pizza events three times as common
    bool eventActive() const { return _active; }
    void setBreatherScale(float s) { _breatherScale = s; }
    void layoutHud();
    // Distance from x to the nearest live hazard actor ahead (left edge of its box), or -1. Used by the QA bot and by a
    // future assist/tutorial hint; never by gameplay.
    float hazardAhead(float x) const;

private:
    enum class Kind { Sign, DinoSmall, DinoBig, Rider, Box, Ambient, Cone, Barrel, Worker, Meteor, Rock, Coin, Shadow, BonusCoin };
    struct Actor {
        int id = 0;
        Kind kind = Kind::DinoSmall;
        ax::Node* root = nullptr;
        ax::Sprite* spr = nullptr;
        std::string base;                         // semantic animation id (V2Content)
        std::vector<std::string> frameNames;      // resolved once from the registry
        int frames = 1;
        float fps = 8.f, t = 0.f;
        float x = 0.f, y = 0.f;                   // feet / wheel contact, scene space
        float vx = 0.f, vy = 0.f;                 // extra velocity relative to the world
        float h = 60.f, w = 60.f;                 // collision size (forgiving: smaller than the sprite)
        bool hazard = false, launched = false, grounded = true, stopped = false;
        bool inGap = false;
        float gapX0 = 0.f, gapX1 = 0.f, gapY0 = 0.f, gapY1 = 0.f;
        float laneLift = 0.f;                     // ambient riders ride on a back lane
        float spin = 0.f, life = 0.f;
        bool collected = false;
        bool facingRight = false;
        ax::Sprite* extra = nullptr;              // e.g. the rocket flame on Grandma's bike
        float targetY = 0.f, fallT = 0.f, fallDur = 1.f;   // meteor / shadow timing
        bool dodged = false, counted = false;     // close-dodge bonus paid / passed-the-rider counted
        int group = 0;                            // risk-line coins share a group id
        ax::DrawNode* lamp = nullptr;             // the worker's STOP / GO paddle
    };
    struct Bubble { ax::Node* node = nullptr; int actorId = 0; float t = 0.f, life = 2.f; ax::Vec2 off; };

    bool init(PZGGameFieldScene* scene, ax::Node* hud);
    PZGGamefieldPlatforms* platform() const;
    bool ground(float x, float& y) const;
    bool roadClear(float x0, float x1, float apex) const;     // flat, continuous, free of lethal obstacles
    bool obstacleNear(float x, float range) const;
    Actor* spawn(Kind kind, const std::string& base, int frames, float x, float y, float targetH, bool faceRight);
    Actor* find(int id);
    void removeActor(Actor& a);
    void animate(Actor& a, float dt);
    void followRoad(Actor& a, float dt, bool hop);
    void launch(Actor& a, float sx);
    void say(const std::string& tag, int actorId);
    void bubble(const std::string& text, int actorId, float life);

    void startEvent(EventId id);
    void finishEvent(bool success) { finishEvent(success ? Outcome::Success : Outcome::Failed); }
    void finishEvent(Outcome o);
    void updateScheduler(const Context& ctx);
    int chapterIndexAt(float distance) const;                 // index into cfg().chapters (-1 = none)
    bool chapterAllows(int chapter, EventId id) const;
    void announceChapter(const std::string& title);
    void updateDino(const Context& ctx);
    void updatePizza(const Context& ctx);
    void updateRace(const Context& ctx);
    void updateConstruction(const Context& ctx);
    void updateMeteor(const Context& ctx);
    void updateGrandma(const Context& ctx);
    void updateTrex(const Context& ctx);
    void updateMicro(const Context& ctx);
    void settleRisk(const Context& ctx);          // pays the risk line when every coin was taken, then resets it
    void updateObjectiveHud(float dt);
    void reward(int coins, const ax::Vec2& at, const std::string& text);   // coins from an event (tracked: not counted twice in the daily score)
    void setObjective(const std::string& text);
    void flashResult(const std::string& text, const ax::Color3B& colour);
    void hideObjective();
    ax::Node* makePlate(float w, const ax::Color3B& edge);
    void spawnRiskLine(float x, float gy, float apex, int count, float heightFrac, float spacing, int group);
    int  countGroup(int group) const;
    float telegraphLead(float base) const;        // seconds between a warning and the first hazard, never below the floor
    float rivalScoreWins() const;
    void impact(Actor& meteor, const Context& ctx);
    void updateAmbient(const Context& ctx);
    void updateActors(const Context& ctx);
    void updateBubbles(float dt, const Context& ctx);
    void hitTest(const Context& ctx);
    void buildRaceMarker();
    void updateRaceMarker();

    PZGGameFieldScene* _scene = nullptr;
    ax::Node* _hud = nullptr;
    Host _host;
    Rng _rng;
    Rng _fxRng;                         // cosmetic only (barks): never feeds gameplay
    float _dodgeCool = 0.f;
    int _dodgesEv = 0, _lastCountedGroup = 0;
    BarkBook _barks;
    ShuffleBag<int> _bag;
    uint32_t _seed = 1;
    int _successes = 0;
    float _breatherScale = 1.f;
    bool _enabled = true;

    std::vector<std::shared_ptr<Actor>> _actors;
    std::vector<Bubble> _bubbles;
    int _nextId = 1;
    float _lastRiderX = 0.f;
    float _clock = 0.f;                 // run time
    float _cool = 9.f;                  // seconds until the next event may start
    float _ambientCool = 14.f;
    float _speed = 300.f;               // smoothed world speed, units / second
    bool _active = false;
    EventId _cur = EventId::DinoCrossing;
    float _t = 0.f;                     // time inside the event
    int _step = 0;                      // spawn script progress
    float _wait = 0.f;
    float _waited = 0.f;
    int _score = 0, _total = 0;         // pizzas collected / dropped, dinos passed, ...
    int _passed = 0, _target = 0;       // objective progress: hazards passed / objects to collect
    int _riskGot = 0, _riskNeed = 0, _riskReward = 0;   // optional risk line of the running event
    std::string _riskLabel;
    float _lastDistance = 0.f;
    bool _hitThisEvent = false;
    Tier _tier = Tier::Intro;
    int _variant = 0;
    ShuffleBag<int> _variantBag[kEventTotal];
    int _chapter = -1;                  // current Night Run chapter
    float _nextClimaxM = 1200.f;        // distance of the next T-Rex chase window
    ax::Node* _chapterTitle = nullptr;
    float _draft = 0.f;                 // Grandma variant 1: seconds spent in her slipstream
    bool _drafted = false;
    int _streak = 0;                    // consecutive cleared events
    bool _lastLucky = false;
    int _gagState = 0;                  // ambient gag payoff: 0 none, 1 a prop was lost (a later rider returns it fixed)
    RunStats _rs;
    int _evCoins = 0;                    // coins paid by the running event (for the summary)
    int _nextGroup = 1;
    bool _pizzaBoost = false;
    bool _powerUpInEvent = false;

    // objective HUD (left side, under the coin counter) + micro challenge plate
    ax::Node* _objPlate = nullptr;
    ax::Label* _objLabel = nullptr;
    std::string _objText;
    float _objResultT = 0.f;
    ax::Node* _microPlate = nullptr;
    ax::Label* _microLabel = nullptr;
    struct Micro { int type = -1; int progress = 0, target = 0; float timeLeft = 0.f, cooldown = 0.f; bool active = false; bool armedNoPowerUp = false; };
    Micro _micro;
    int _microNext = 0;

    // rival memory (persisted)
    int rivalWins() const;
    int rivalLosses() const;
    void noteRivalResult(bool playerWon);

    // rival race
    int _rivalId = 0;
    float _lead = 0.f, _pendingGain = 0.f, _raceT = 0.f;
    int _raceState = 0;                 // 0 intro pass, 1 racing, 2 won, 3 lost
    ax::Node* _marker = nullptr;
    ax::DrawNode* _markerBar = nullptr;
    ax::Node* _markerRival = nullptr;
    ax::Node* _markerPlayer = nullptr;

    // all tuning lives in V2Config (plist `v2` section): see V2::cfg()
};

} // namespace V2
