#pragma once

#include "box2d/box2d.h"
#include "glm/glm.hpp"

#include "Cube/Scene/Component.h"

namespace Cube {

// Attaches a Box2D rigid body to a node. The body type field stores a b2BodyType
// value (b2_staticBody / b2_kinematicBody / b2_dynamicBody) as an int because
// enums are not reflectable by the serialization system.
//
// NOTE: 物理节点应挂在变换接近单位阵的父节点下(通常是 root), 因为 Box2D 是世界空间
// 扁平结构, 而节点的 pos/rotation 是局部变换, 这里直接以局部变换当作世界变换使用.
class RigidBody : public Component {
public:
    int bodyType = b2_dynamicBody;
    glm::vec2 linearVelocity = {0.0f, 0.0f}; // pixels per second
    float angularVelocity = 0.0f;            // degrees per second
    float gravityScale = 1.0f;
    float linearDamping = 0.0f;
    float angularDamping = 0.0f;
    bool fixedRotation = false;
    bool isBullet = false;
    bool enabled = true;

    RigidBody() = default;
    ~RigidBody() override;

    void start() override;
    void update(float delta) override;
    TypeID getType() const override { return getTypeID<RigidBody>(); }

    // Create the underlying body on demand. Safe to call multiple times. It is
    // also invoked by colliders so their initialization does not depend on the
    // order in which components are added to the node.
    void ensureBody();

    b2BodyId getBodyId() const { return bodyId; }
    bool hasBody() const { return B2_IS_NON_NULL(bodyId); }

    // Called by PhysicsServer after a world step to write the body transform
    // back to the node.
    void syncTransformFromBody();

private:
    b2BodyId bodyId = b2_nullBodyId;
};

}  // namespace Cube
