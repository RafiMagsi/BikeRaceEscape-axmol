#pragma once
#include "PZLegacyCompat.h"
#include "GLESDebugDraw.h"

class GLESDEbugDrawLayer : public ax::Layer {
public:
    CREATE_FUNC(GLESDEbugDrawLayer);
    static ax::Scene* scene();

    b2World* world{nullptr};
    ax::Rect playArea{ax::Rect::ZERO};

    bool init() override { return ax::Layer::init(); }
};
