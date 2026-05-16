#pragma once
#include "PZLegacyCompat.h"
#include "PZLegacyInputLayer.h"

class PZLegacyLayer : public PZLegacyLayer {
public:
    bool init() override;
    void setTouchEnabled(bool enabled);
    void setAccelerometerEnabled(bool enabled);

    virtual void ccTouchesBegan(ax::__Set* touches, ax::Event* event) { (void)touches; (void)event; }
    virtual void ccTouchesMoved(ax::__Set* touches, ax::Event* event) { (void)touches; (void)event; }
    virtual void ccTouchesEnded(ax::__Set* touches, ax::Event* event) { (void)touches; (void)event; }
    virtual void didAccelerate(ax::Acceleration* acceleration) { (void)acceleration; }

protected:
    ax::EventListenerTouchAllAtOnce* _legacyTouchListener = nullptr;
    ax::EventListenerAcceleration* _legacyAccelerationListener = nullptr;
};
