//
//  V2Ghost.h
//  Personal best ghost (PRODUCT_PLAN 20.10 #4): a translucent copy of your own best run on the same daily road, drawn from the
//  recorded checkpoints (tick, distance, height) and interpolated between them. Purely cosmetic: no physics body, no collision,
//  no scoring; it never touches the replay or the run RNG. The ghost belongs to ONE seed + rules version + modifier; any
//  mismatch disables it. Friend ghosts are NOT possible without a server (a token cannot carry a full replay).
//
#pragma once

#include "PZLegacyCompat.h"
#include "V2/V2Replay.h"

namespace V2 {

class GhostRider : public ax::Node {
public:
    static GhostRider* create();

    // true when `d` is a usable ghost for a run with this seed / rules / modifier (and has enough checkpoints)
    static bool compatible(const Replay::Data& d, uint32_t seed, int rules, int modifier);
    void arm(const Replay::Data& d);
    void clear();
    bool armed() const { return !_pts.empty(); }
    // tick: simulation ticks since GO. Positions are given relative to the live rider.
    void update(uint32_t tick, float myDistance, float myBodyY, const ax::Vec2& riderScreen, float metersPerPixel,
                ax::SpriteFrame* frame, float spriteScale, ax::Node* parent, int z);

private:
    struct Pt { uint32_t tick; float dist, y; };
    std::vector<Pt> _pts;
    ax::Sprite* _spr = nullptr;
    ax::Label* _tag = nullptr;
};

} // namespace V2
