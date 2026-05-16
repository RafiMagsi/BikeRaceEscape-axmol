#include "PZLegacyCompat.h"
#include "GLESDEbugDrawLayer.h"

ax::Scene* GLESDEbugDrawLayer::scene() {
    auto* scene = ax::Scene::create();
    auto* layer = GLESDEbugDrawLayer::create();
    scene->addChild(layer);
    return scene;
}
