#include "PZLegacyCompat.h"
#include "PZLegacyInputLayer.h"
USING_NS_AX;

bool PZLegacyLayer::init() {
    return Layer::init();
}

static __Set buildLegacyTouchSet(const std::vector<Touch*>& touches) {
    __Set legacy;
    for (auto* touch : touches) legacy.addObject(touch);
    return legacy;
}

void PZLegacyLayer::setTouchEnabled(bool enabled) {
    if (enabled && !_legacyTouchListener) {
        _legacyTouchListener = EventListenerTouchAllAtOnce::create();
        _legacyTouchListener->onTouchesBegan = [this](const std::vector<Touch*>& touches, Event* event) {
            auto legacy = buildLegacyTouchSet(touches);
            this->ccTouchesBegan(&legacy, event);
        };
        _legacyTouchListener->onTouchesMoved = [this](const std::vector<Touch*>& touches, Event* event) {
            auto legacy = buildLegacyTouchSet(touches);
            this->ccTouchesMoved(&legacy, event);
        };
        _legacyTouchListener->onTouchesEnded = [this](const std::vector<Touch*>& touches, Event* event) {
            auto legacy = buildLegacyTouchSet(touches);
            this->ccTouchesEnded(&legacy, event);
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(_legacyTouchListener, this);
    } else if (!enabled && _legacyTouchListener) {
        _eventDispatcher->removeEventListener(_legacyTouchListener);
        _legacyTouchListener = nullptr;
    }
}

void PZLegacyLayer::setAccelerometerEnabled(bool enabled) {
    Device::setAccelerometerEnabled(enabled);
    if (enabled && !_legacyAccelerationListener) {
        _legacyAccelerationListener = EventListenerAcceleration::create([this](Acceleration* acceleration, Event* event) {
            (void)event;
            this->didAccelerate(acceleration);
        });
        _eventDispatcher->addEventListenerWithSceneGraphPriority(_legacyAccelerationListener, this);
    } else if (!enabled && _legacyAccelerationListener) {
        _eventDispatcher->removeEventListener(_legacyAccelerationListener);
        _legacyAccelerationListener = nullptr;
    }
}
