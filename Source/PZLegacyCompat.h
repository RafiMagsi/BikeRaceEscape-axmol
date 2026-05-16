#pragma once
#include "axmol.h"
#include "audio/AudioEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <map>
#include <sstream>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace cocos2d = ax;

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

#ifndef SizeMake
#define SizeMake(__W__, __H__) ax::Size((float)(__W__), (float)(__H__))
#endif
#ifndef RectMake
#define RectMake(__X__, __Y__, __W__, __H__) ax::Rect((float)(__X__), (float)(__Y__), (float)(__W__), (float)(__H__))
#endif

#ifndef CC_SYNTHESIZE
#define CC_SYNTHESIZE(varType, varName, funName) \
protected: varType varName; \
public: virtual varType get##funName(void) const { return varName; } \
public: virtual void set##funName(varType var) { varName = var; }
#endif

#ifndef CCARRAY_FOREACH
#define CCARRAY_FOREACH(__array__, __object__) \
for (ssize_t __legacy_index = 0; \
     (__array__) != nullptr && __legacy_index < (__array__)->count() && \
     (((__object__) = (__array__)->objectAtIndex(__legacy_index)), true); \
     ++__legacy_index)
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


// -----------------------------------------------------------------------------
// Legacy value/container adapters.
// Cocos2d-x 2.x code in this project relies heavily on CCString/CCArray/
// CCDictionary, renamed in later Cocos versions and removed from Axmol.
// Axmol's FAQ recommends STL/ValueMap for new code, but keeping a compact
// adapter here is the fastest safe bridge while porting this legacy project.
// -----------------------------------------------------------------------------
namespace ax {

class __String final : public Object {
public:
    std::string m_sString;

    static __String* create(const std::string& value) {
        auto* obj = new __String(value);
        obj->autorelease();
        return obj;
    }

    static __String* create(const char* value) {
        return create(std::string(value ? value : ""));
    }

    static __String* createWithFormat(const char* format, ...) {
        if (!format) {
            return create("");
        }

        va_list args;
        va_start(args, format);
        va_list copy;
        va_copy(copy, args);
        const int needed = std::vsnprintf(nullptr, 0, format, copy);
        va_end(copy);

        if (needed <= 0) {
            va_end(args);
            return create("");
        }

        std::string buffer(static_cast<size_t>(needed), '\0');
        std::vsnprintf(buffer.data(), static_cast<size_t>(needed) + 1, format, args);
        va_end(args);
        return create(buffer);
    }

    const char* getCString() const { return m_sString.c_str(); }

    int compare(const char* rhs) const {
        return m_sString.compare(rhs ? rhs : "");
    }

    bool isEqual(const Object* other) const {
        const auto* rhs = dynamic_cast<const __String*>(other);
        return rhs != nullptr && m_sString == rhs->m_sString;
    }

    int intValue() const {
        try { return std::stoi(m_sString); } catch (...) { return 0; }
    }

    float floatValue() const {
        try { return std::stof(m_sString); } catch (...) { return 0.0f; }
    }

    bool boolValue() const {
        if (m_sString == "1" || m_sString == "true" || m_sString == "TRUE" ||
            m_sString == "yes" || m_sString == "YES") {
            return true;
        }
        return false;
    }

    Vec2 pointValue() const {
        float x = 0.0f;
        float y = 0.0f;
        const char* raw = m_sString.c_str();
        // Accept common plist point formats: "{x,y}", "x,y", "{x, y}".
        if (std::sscanf(raw, "{%f,%f}", &x, &y) == 2 ||
            std::sscanf(raw, "{%f, %f}", &x, &y) == 2 ||
            std::sscanf(raw, "%f,%f", &x, &y) == 2 ||
            std::sscanf(raw, "%f, %f", &x, &y) == 2) {
            return Vec2(x, y);
        }
        return Vec2::ZERO;
    }

private:
    explicit __String(std::string value) : m_sString(std::move(value)) {}
};

class __Array final : public Object {
public:
    static __Array* create() {
        auto* obj = new __Array();
        obj->autorelease();
        return obj;
    }

    ~__Array() override {
        for (auto* object : _objects) {
            AX_SAFE_RELEASE(object);
        }
        _objects.clear();
    }

    void addObject(Object* object) {
        if (!object) {
            return;
        }
        object->retain();
        _objects.push_back(object);
    }

    Object* objectAtIndex(ssize_t index) const {
        if (index < 0 || static_cast<size_t>(index) >= _objects.size()) {
            return nullptr;
        }
        return _objects[static_cast<size_t>(index)];
    }

    ssize_t count() const {
        return static_cast<ssize_t>(_objects.size());
    }

    void removeObjectsInArray(const __Array* array) {
        if (!array) {
            return;
        }
        for (const auto* target : array->_objects) {
            auto it = std::find(_objects.begin(), _objects.end(), target);
            if (it != _objects.end()) {
                AX_SAFE_RELEASE(*it);
                _objects.erase(it);
            }
        }
    }

private:
    std::vector<Object*> _objects;
};

class __Dictionary final : public Object {
public:
    static __Dictionary* create() {
        auto* obj = new __Dictionary();
        obj->autorelease();
        return obj;
    }

    static __Dictionary* createWithContentsOfFile(const char* path);

    ~__Dictionary() override {
        for (auto& item : _objects) {
            AX_SAFE_RELEASE(item.second);
        }
        _objects.clear();
    }

    void setObject(Object* object, const std::string& key) {
        if (!object) {
            return;
        }

        auto it = _objects.find(key);
        if (it != _objects.end()) {
            AX_SAFE_RELEASE(it->second);
        }

        object->retain();
        _objects[key] = object;
    }

    void setObject(Object* object, const char* key) {
        setObject(object, std::string(key ? key : ""));
    }

    Object* objectForKey(const std::string& key) const {
        const auto it = _objects.find(key);
        return it == _objects.end() ? nullptr : it->second;
    }

    Object* objectForKey(const char* key) const {
        return objectForKey(std::string(key ? key : ""));
    }

    __String* valueForKey(const std::string& key) const {
        return dynamic_cast<__String*>(objectForKey(key));
    }

    __String* valueForKey(const char* key) const {
        return valueForKey(std::string(key ? key : ""));
    }

private:
    std::map<std::string, Object*> _objects;
};

inline Object* __legacyObjectFromValue(const Value& value);

inline __Array* __legacyArrayFromValueVector(const ValueVector& vector) {
    auto* array = __Array::create();
    for (const auto& item : vector) {
        auto* object = __legacyObjectFromValue(item);
        if (object) {
            array->addObject(object);
        }
    }
    return array;
}

inline __Dictionary* __legacyDictionaryFromValueMap(const ValueMap& map) {
    auto* dictionary = __Dictionary::create();
    for (const auto& item : map) {
        auto* object = __legacyObjectFromValue(item.second);
        if (object) {
            dictionary->setObject(object, item.first);
        }
    }
    return dictionary;
}

inline Object* __legacyObjectFromValue(const Value& value) {
    switch (value.getType()) {
        case Value::Type::MAP:
            return __legacyDictionaryFromValueMap(value.asValueMap());
        case Value::Type::VECTOR:
            return __legacyArrayFromValueVector(value.asValueVector());
        default:
            return __String::create(value.asString());
    }
}

inline __Dictionary* __Dictionary::createWithContentsOfFile(const char* path) {
    const auto map = FileUtils::getInstance()->getValueMapFromFile(path ? path : "");
    return __legacyDictionaryFromValueMap(map);
}

} // namespace ax

using __String = ax::__String;
using __Array = ax::__Array;
using __Dictionary = ax::__Dictionary;

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
