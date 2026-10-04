#include "BoxCollider.h"

#include "PhysicsServer.h"
#include "RigidBody.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Node.h"

namespace Cube {

BoxCollider::~BoxCollider() {
    if (B2_IS_NON_NULL(shapeId)) {
        if (b2Shape_IsValid(shapeId)) {
            b2DestroyShape(shapeId, true);
        }
        shapeId = b2_nullShapeId;
    }
}

void BoxCollider::start() {
    if (!node) {
        CB_CORE_ERROR("BoxCollider::start(): component is not attached to a node");
        return;
    }
    RigidBody* rigidBody = node->getComponent<RigidBody>();
    if (!rigidBody) {
        CB_CORE_ERROR("BoxCollider::start(): node '{}' has no RigidBody component", node->getName());
        return;
    }
    rigidBody->ensureBody();
    if (!rigidBody->hasBody()) {
        return;
    }
    PhysicsServer* server = getPhysicsServer();
    if (!server) {
        CB_CORE_ERROR("BoxCollider::start(): no physics server available");
        return;
    }
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor = isSensor;
    b2Polygon polygon = b2MakeOffsetBox(server->toMeters(size.x) * 0.5f, server->toMeters(size.y) * 0.5f, server->toMeters(offset), b2Rot_identity);
    shapeId = b2CreatePolygonShape(rigidBody->getBodyId(), &shapeDef, &polygon);
    if (B2_IS_NULL(shapeId)) {
        CB_CORE_ERROR("BoxCollider::start(): failed to create box shape on node '{}'", node->getName());
    }
}

}  // namespace Cube
