#pragma once
#include "PZLegacyCompat.h"
#include "GLESDebugDraw.h"

class GLESDEbugDrawLayer : public ax::Layer {
public:
    CREATE_FUNC(GLESDEbugDrawLayer);
    bool init() override { return ax::Layer::init(); }
};
