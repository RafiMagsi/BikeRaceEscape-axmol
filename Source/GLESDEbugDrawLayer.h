#pragma once
#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"

class GLESDEbugDrawLayer : public ax::Layer {
public:
    CREATE_FUNC(GLESDEbugDrawLayer);
    static ax::Scene* scene();

    b2World* world{nullptr};
    ax::Rect playArea{ax::Rect::ZERO};

    bool init() override;

    void update(float dt) override;

private:
    ax::DrawNode* _drawNode{nullptr};
    void drawWorld();
};
