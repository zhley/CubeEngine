#include "RigidBody.h"

#include "PhysicsServer.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Node.h"

namespace Cube {

RigidBody::~RigidBody() {
    if (B2_IS_NULL(bodyId)) {
        return;
    }
    if (PhysicsServer* server = getPhysicsServer()) {
        server->unregisterBody(this);
    }
    if (b2Body_IsValid(bodyId)) {
        b2DestroyBody(bodyId);
    }
    bodyId = b2_nullBodyId;
}

void RigidBody::start() {
    ensureBody();
}

void RigidBody::update(float delta) {
    if (B2_IS_NULL(bodyId) || !node) {
        return;
    }
    // Static and kinematic bodies are driven by the node transform; dynamic
    // bodies are driven by the simulation and synced back in PhysicsServer::step.
    if (bodyType == b2_staticBody || bodyType == b2_kinematicBody) {
        PhysicsServer* server = getPhysicsServer();
        if (!server) {
            return;
        }
        b2Body_SetTransform(bodyId, server->toMeters(node->pos), b2MakeRot(glm::radians(node->rotation)));
    }
}

void RigidBody::ensureBody() {
    if (B2_IS_NON_NULL(bodyId)) {
        return;
    }
    if (!node) {
        CB_CORE_ERROR("RigidBody::ensureBody(): component is not attached to a node");
        return;
    }
    PhysicsServer* server = getPhysicsServer();
    if (!server) {
        CB_CORE_ERROR("RigidBody::ensureBody(): no physics server available");
        return;
    }
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = static_cast<b2BodyType>(bodyType);
    bodyDef.position = server->toMeters(node->pos);
    bodyDef.rotation = b2MakeRot(glm::radians(node->rotation));
    bodyDef.linearVelocity = server->toMeters(linearVelocity);
    bodyDef.angularVelocity = glm::radians(angularVelocity);
    bodyDef.gravityScale = gravityScale;
    bodyDef.linearDamping = linearDamping;
    bodyDef.angularDamping = angularDamping;
    bodyDef.fixedRotation = fixedRotation;
    bodyDef.isBullet = isBullet;
    bodyDef.isEnabled = enabled;
    bodyId = b2CreateBody(server->getWorldId(), &bodyDef);
    if (B2_IS_NULL(bodyId)) {
        CB_CORE_ERROR("RigidBody::ensureBody(): failed to create body for node '{}'", node->getName());
        return;
    }
    server->registerBody(this);
}

void RigidBody::syncTransformFromBody() {
    if (B2_IS_NULL(bodyId) || !node || bodyType != b2_dynamicBody) {
        return;
    }
    PhysicsServer* server = getPhysicsServer();
    if (!server) {
        return;
    }
    b2Transform transform = b2Body_GetTransform(bodyId);
    node->pos = server->toPixels(transform.p);
    node->rotation = glm::degrees(b2Rot_GetAngle(transform.q));
}

}  // namespace Cube
