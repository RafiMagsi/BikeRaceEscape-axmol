#pragma once
#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"

// Legacy OpenGL immediate/debug drawing disabled during the Axmol core-port.
class GLESDebugDraw : public b2Draw {
public:
    explicit GLESDebugDraw(float ratio) : mRatio(ratio) {}
    void DrawPolygon(const b2Vec2*, int32, const b2Color&) override {}
    void DrawSolidPolygon(const b2Vec2*, int32, const b2Color&) override {}
    void DrawCircle(const b2Vec2&, float, const b2Color&) override {}
    void DrawSolidCircle(const b2Vec2&, float, const b2Vec2&, const b2Color&) override {}
    void DrawSegment(const b2Vec2&, const b2Vec2&, const b2Color&) override {}
    void DrawTransform(const b2Transform&) override {}
    void DrawPoint(const b2Vec2&, float, const b2Color&) {}
private:
    float mRatio;
};
