//
//  V2Cinematic.h
//  Single owner of "who may drive the screen" (PRODUCT_PLAN 21.7 / 21.13): pre-run dialogue, intro, T-Rex climax, blast, death,
//  revive. Only one owner at a time; legal transitions follow the plan's state diagram; Death and scene exit always win.
//  A denied request means "skip the cosmetic": callers fall back to their non-cinematic path (no zoom, no letterbox), so a
//  blast pickup during the T-Rex climax or a camera punch during the intro can never fight for the camera.
//  Pure logic (no nodes): testable with selfTest().
//
#pragma once

#include <string>

namespace V2 {

class Cinematic {
public:
    enum class Owner { None, PreRunDialogue, Intro, Gameplay, Climax, Blast, Death, Revive };

    static Cinematic& shared();

    bool request(Owner o);                 // true = granted (the caller now owns the camera / input / HUD)
    void release(Owner o);                 // only the current owner can release; others are ignored
    void forceReset();                     // new run, scene exit, app interruption: back to None, always
    Owner owner() const { return _owner; }
    bool allowsGameplayCamera() const { return _owner == Owner::None || _owner == Owner::Gameplay; }   // landing punches, scenic reveals
    static const char* name(Owner o);

    int denied() const { return _denied; }
    std::string selfTest();                // "RESULT: PASS" / "RESULT: FAIL" report

private:
    Owner _owner = Owner::None;
    int _denied = 0;
};

} // namespace V2
