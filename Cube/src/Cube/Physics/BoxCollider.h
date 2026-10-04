#pragma once

#include "box2d/box2d.h"
#include "glm/glm.hpp"

#include "Cube/Scene/Component.h"

namespace Cube {

// A rectangular collision shape created on the node's RigidBody component.
class BoxCollider : public Component {
public:
    glm::vec2 size = {100.0f, 100.0f}; // pixels
    glm::vec2 offset = {0.0f, 0.0f};   // pixels, relative to the body origin
    float density = 1.0f;
    float friction = 0.2f;
    float restitution = 0.0f;
    bool isSensor = false;

    BoxCollider() = default;
    ~BoxCollider() override;

    void start() override;
    TypeID getType() const override { return getTypeID<BoxCollider>(); }

private:
    b2ShapeId shapeId = b2_nullShapeId;
};

}  // namespace Cube
