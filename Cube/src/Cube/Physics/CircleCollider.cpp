#include "CircleCollider.h"

#include "PhysicsServer.h"
#include "RigidBody.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Node.h"

namespace Cube {

CircleCollider::~CircleCollider() {
    if (B2_IS_NON_NULL(shapeId)) {
        if (b2Shape_IsValid(shapeId)) {
            b2DestroyShape(shapeId, true);
        }
        shapeId = b2_nullShapeId;
    }
}

void CircleCollider::start() {
    if (!node) {
        CB_CORE_ERROR("CircleCollider::start(): component is not attached to a node");
        return;
    }
    RigidBody* rigidBody = node->getComponent<RigidBody>();
    if (!rigidBody) {
        CB_CORE_ERROR("CircleCollider::start(): node '{}' has no RigidBody component", node->getName());
        return;
    }
    rigidBody->ensureBody();
    if (!rigidBody->hasBody()) {
        return;
    }
    PhysicsServer* server = getPhysicsServer();
    if (!server) {
        CB_CORE_ERROR("CircleCollider::start(): no physics server available");
        return;
    }
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor = isSensor;
    b2Circle circle;
    circle.center = server->toMeters(offset);
    circle.radius = server->toMeters(radius);
    shapeId = b2CreateCircleShape(rigidBody->getBodyId(), &shapeDef, &circle);
    if (B2_IS_NULL(shapeId)) {
        CB_CORE_ERROR("CircleCollider::start(): failed to create circle shape on node '{}'", node->getName());
    }
}

}  // namespace Cube
