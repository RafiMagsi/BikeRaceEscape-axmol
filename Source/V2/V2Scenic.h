//
//  V2Scenic.h
//  Decorative background dinosaur beats (PRODUCT_PLAN 21.5): a distant walker crossing the far plane and a wall-peeking T-Rex.
//  Purely cosmetic: its own RNG (never the run seed), no physics, no scoring, nothing the replay can see. Beats yield to road
//  readability: never during events, the climax, control locks (intro / death / revive) or while a ride power-up owns the rider.
//  Beat art, timing and layer come from CompleteGameInfo.plist (v2.presentation.scenicBeats); Spectacle Off / Reduced and
//  Reduced Motion are honoured (reduced = a still fade instead of a pan).
//
#pragma once

#include "PZLegacyCompat.h"
#include "V2/V2Core.h"

class PZGGameFieldScene;

namespace V2 {

class ScenicDirector : public ax::Node {
public:
    static ScenicDirector* create(PZGGameFieldScene* scene);

    void reset();                                   // new run
    void clear();                                   // remove any live beat (death, restart, scene exit)
    // dt: simulated seconds this tick (0 while frozen). `safe` is false whenever decoration must stay out of the way.
    void update(float dt, bool safe);
    bool active() const { return _node != nullptr; }
    const std::string& lastBeat() const { return _lastId; }

private:
    bool init(PZGGameFieldScene* scene);
    void startBeat(int index);
    void finishBeat();
    int pickBeat();

    PZGGameFieldScene* _scene = nullptr;
    Rng _rng;                                       // cosmetic stream, seeded from the clock
    ax::Node* _node = nullptr;
    ax::Node* _clip = nullptr;                      // optional clipping parent (owns _node when set)
    ax::Sprite* _spr = nullptr;
    int _beat = -1;
    std::string _lastId;
    float _t = 0.f, _duration = 0.f, _runClock = 0.f, _cooldown = 0.f, _x0 = 0.f, _x1 = 0.f, _y0 = 0.f, _y1 = 0.f;
    bool _reduced = false;
    std::vector<float> _recent;                     // run-clock start times of recent beats (rate limit)
};

} // namespace V2
