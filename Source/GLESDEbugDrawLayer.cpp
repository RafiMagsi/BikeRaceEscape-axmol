#include "PZLegacyCompat.h"
#include "GLESDEbugDrawLayer.h"
#include "constants.h"

ax::Scene* GLESDEbugDrawLayer::scene() {
    auto* scene = ax::Scene::create();
    auto* layer = GLESDEbugDrawLayer::create();
    scene->addChild(layer);
    return scene;
}

bool GLESDEbugDrawLayer::init() {
    if (!ax::Layer::init()) {
        return false;
    }

    _drawNode = ax::DrawNode::create();
    this->addChild(_drawNode, 1);

    this->scheduleUpdate();
    return true;
}

void GLESDEbugDrawLayer::update(float) {
    drawWorld();
}

static ax::Color4F pz_colorForBody(const b2Body* body) {
    if (!body) return {1, 1, 1, 1};
    switch (body->GetType()) {
        case b2_staticBody: return {0.2f, 1.0f, 0.2f, 0.9f};
        case b2_kinematicBody: return {0.2f, 0.6f, 1.0f, 0.9f};
        case b2_dynamicBody: return {1.0f, 0.2f, 0.2f, 0.9f};
        default: return {1.0f, 1.0f, 1.0f, 0.9f};
    }
}

void GLESDEbugDrawLayer::drawWorld() {
    if (!_drawNode) return;

    _drawNode->clear();

    // Draw play area (pixel coordinates already)
    if (!playArea.equals(ax::Rect::ZERO)) {
        _drawNode->drawRect(
            ax::Vec2(playArea.getMinX(), playArea.getMinY()),
            ax::Vec2(playArea.getMaxX(), playArea.getMaxY()),
            ax::Color4F(1.0f, 1.0f, 0.0f, 0.8f));
    }

    if (!world) return;

    const float invScale = 1.0f / kPhysicsWorldScale();

    for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
        const ax::Color4F color = pz_colorForBody(body);
        const b2Transform xf = body->GetTransform();

        for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
            const b2Shape* shape = fixture->GetShape();
            if (!shape) continue;

            switch (shape->GetType()) {
                case b2Shape::e_polygon: {
                    const auto* poly = static_cast<const b2PolygonShape*>(shape);
                    std::vector<ax::Vec2> verts;
                    const int count = poly->GetVertexCount();
                    verts.reserve(static_cast<size_t>(count));
                    for (int i = 0; i < count; ++i) {
                        const b2Vec2 v = b2Mul(xf, poly->GetVertex(i));
                        verts.emplace_back(v.x * invScale, v.y * invScale);
                    }
                    if (verts.size() >= 2) {
                        _drawNode->drawPoly(verts.data(), static_cast<int>(verts.size()), true, color);
                    }
                } break;
                case b2Shape::e_circle: {
                    const auto* cir = static_cast<const b2CircleShape*>(shape);
                    const b2Vec2 c = b2Mul(xf, cir->m_p);
                    _drawNode->drawCircle(
                        ax::Vec2(c.x * invScale, c.y * invScale),
                        cir->m_radius * invScale,
                        0,
                        24,
                        false,
                        color);
                } break;
                case b2Shape::e_edge: {
                    const auto* edge = static_cast<const b2EdgeShape*>(shape);
                    const b2Vec2 v1 = b2Mul(xf, edge->m_vertex1);
                    const b2Vec2 v2 = b2Mul(xf, edge->m_vertex2);
                    _drawNode->drawLine(
                        ax::Vec2(v1.x * invScale, v1.y * invScale),
                        ax::Vec2(v2.x * invScale, v2.y * invScale),
                        color);
                } break;
                case b2Shape::e_chain: {
                    const auto* chain = static_cast<const b2ChainShape*>(shape);
                    for (int i = 0; i < chain->m_count - 1; ++i) {
                        const b2Vec2 v1 = b2Mul(xf, chain->m_vertices[i]);
                        const b2Vec2 v2 = b2Mul(xf, chain->m_vertices[i + 1]);
                        _drawNode->drawLine(
                            ax::Vec2(v1.x * invScale, v1.y * invScale),
                            ax::Vec2(v2.x * invScale, v2.y * invScale),
                            color);
                    }
                } break;
                default:
                    break;
            }
        }
    }
}
