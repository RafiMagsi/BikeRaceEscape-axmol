#pragma once
#include "axmol.h"
#include "audio/AudioEngine.h"
#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#ifndef LAYER_CREATE_FUNC
#define LAYER_CREATE_FUNC(__TYPE__) CREATE_FUNC(__TYPE__)
#endif

#ifndef ccp
#define ccp(__X__, __Y__) ax::Vec2((float)(__X__), (float)(__Y__))
#endif
#ifndef ccpAdd
#define ccpAdd(__V1__, __V2__) ((__V1__) + (__V2__))
#endif
#ifndef ccpSub
#define ccpSub(__V1__, __V2__) ((__V1__) - (__V2__))
#endif
#ifndef ccpMult
#define ccpMult(__V__, __S__) ((__V__) * (__S__))
#endif
#ifndef ccpCompMult
#define ccpCompMult(__V1__, __V2__) ax::Vec2((__V1__).x * (__V2__).x, (__V1__).y * (__V2__).y)
#endif
#ifndef ccpLength
#define ccpLength(__V__) ((__V__).length())
#endif
#ifndef ccpNormalize
#define ccpNormalize(__V__) ((__V__).getNormalized())
#endif
#ifndef ccpLerp
#define ccpLerp(__A__, __B__, __ALPHA__) ((__A__) + (((__B__) - (__A__)) * (__ALPHA__)))
#endif

#ifndef CCPointZero
#define CCPointZero ax::Vec2::ZERO
#endif
#ifndef CCRectMake
#define CCRectMake(__X__, __Y__, __W__, __H__) ax::Rect((float)(__X__), (float)(__Y__), (float)(__W__), (float)(__H__))
#endif
#ifndef CCSizeMake
#define CCSizeMake(__W__, __H__) ax::Size((float)(__W__), (float)(__H__))
#endif
#ifndef ccc3
#define ccc3(__R__, __G__, __B__) ax::Color3B((unsigned char)(__R__), (unsigned char)(__G__), (unsigned char)(__B__))
#endif
#ifndef ccc4
#define ccc4(__R__, __G__, __B__, __A__) ax::Color4B((unsigned char)(__R__), (unsigned char)(__G__), (unsigned char)(__B__), (unsigned char)(__A__))
#endif

#ifndef CC_TARGET_PLATFORM
#define CC_TARGET_PLATFORM AX_TARGET_PLATFORM
#endif
#ifndef CC_PLATFORM_IOS
#define CC_PLATFORM_IOS AX_PLATFORM_IOS
#endif
#ifndef CC_PLATFORM_ANDROID
#define CC_PLATFORM_ANDROID AX_PLATFORM_ANDROID
#endif
#ifndef CC_PLATFORM_MAC
#define CC_PLATFORM_MAC AX_PLATFORM_MAC
#endif
#ifndef CC_PLATFORM_WIN32
#define CC_PLATFORM_WIN32 AX_PLATFORM_WIN32
#endif
#ifndef CC_PLATFORM_LINUX
#define CC_PLATFORM_LINUX AX_PLATFORM_LINUX
#endif

#ifndef CC_CONTENT_SCALE_FACTOR
#define CC_CONTENT_SCALE_FACTOR() ax::Director::getInstance()->getContentScaleFactor()
#endif
#ifndef CC_DEGREES_TO_RADIANS
#define CC_DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) * 0.01745329252f)
#endif
#ifndef CC_RADIANS_TO_DEGREES
#define CC_RADIANS_TO_DEGREES(__ANGLE__) ((__ANGLE__) * 57.29577951f)
#endif
#ifndef CCRANDOM_0_1
#define CCRANDOM_0_1() ((float)std::rand() / (float)RAND_MAX)
#endif
#ifndef CCRANDOM_MINUS1_1
#define CCRANDOM_MINUS1_1() (CCRANDOM_0_1() * 2.0f - 1.0f)
#endif
#ifndef CCLog
#define CCLog(...) AXLOGD(__VA_ARGS__)
#endif
#ifndef kResolutionExactFit
#define kResolutionExactFit ax::ResolutionPolicy::EXACT_FIT
#endif

// Legacy touch-set adapter retained during migration.
// Cocos2d-x 2.x touch callbacks used CCSet/__Set. Axmol 2.11.x uses
// std::vector<Touch*> callbacks instead, so we provide a tiny transient set
// wrapper used only by PZLegacyInputLayer when forwarding touch events.
//
// Axmol 2.11.1+ renamed Object -> Object. This adapter intentionally does not
// inherit from Object because it is a short-lived stack/local bridge, not an
// autoreleased engine object.
namespace ax {
class __Set {
public:
    using Container = std::vector<Object*>;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

    void addObject(Object* object) {
        if (object != nullptr) {
            _objects.push_back(object);
        }
    }

    Object* anyObject() const {
        return _objects.empty() ? nullptr : _objects.front();
    }

    ssize_t count() const {
        return static_cast<ssize_t>(_objects.size());
    }

    iterator begin() { return _objects.begin(); }
    iterator end() { return _objects.end(); }
    const_iterator begin() const { return _objects.begin(); }
    const_iterator end() const { return _objects.end(); }

private:
    Container _objects;
};
} // namespace ax

using __Set = ax::__Set;
using CCSet = ax::__Set;
using __SetIterator = ax::__Set::iterator;
using CCSetIterator = __SetIterator;
using CCAnimate = ax::Animate;
using CCAction = ax::Action;
using CCRepeatForever = ax::RepeatForever;
using CCLayerColor = ax::LayerColor;
using ccBlendFunc = ax::BlendFunc;

namespace CocosDenshion {
class SimpleAudioEngine {
public:
    static SimpleAudioEngine* sharedEngine() {
        static SimpleAudioEngine instance;
        return &instance;
    }
    unsigned int playEffect(const char* file, bool loop = false) { return ax::AudioEngine::play2d(file ? file : "", loop); }
    void stopEffect(unsigned int audioID) { ax::AudioEngine::stop((int)audioID); }
    void playBackgroundMusic(const char* file, bool loop = false) {
        stopBackgroundMusic();
        _bgmId = ax::AudioEngine::play2d(file ? file : "", loop);
    }
    void stopBackgroundMusic(bool release = false) { (void)release; if (_bgmId >= 0) { ax::AudioEngine::stop(_bgmId); _bgmId = -1; } }
    void pauseBackgroundMusic() { if (_bgmId >= 0) ax::AudioEngine::pause(_bgmId); }
    void resumeBackgroundMusic() { if (_bgmId >= 0) ax::AudioEngine::resume(_bgmId); }
    void stopAllEffects() { ax::AudioEngine::stopAll(); }
    void preloadEffect(const char*) {}
    void preloadBackgroundMusic(const char*) {}
private:
    int _bgmId = -1;
};
}
