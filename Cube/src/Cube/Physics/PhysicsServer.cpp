#include "PhysicsServer.h"

#include <algorithm>

#include "RigidBody.h"
#include "Cube/Core/Application.h"
#include "Cube/Core/Engine.h"
#include "Cube/Core/Log.h"

namespace Cube {

PhysicsServer::PhysicsServer() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {gravity.x, gravity.y};
    worldId = b2CreateWorld(&worldDef);
}

PhysicsServer::~PhysicsServer() {
    if (B2_IS_NON_NULL(worldId)) {
        b2DestroyWorld(worldId);
        worldId = b2_nullWorldId;
    }
}

void PhysicsServer::step(float delta) {
    if (delta <= 0.0f) {
        return;
    }
    if (B2_IS_NULL(worldId)) {
        CB_CORE_ERROR("PhysicsServer::step(): world is not valid");
        return;
    }
    b2World_Step(worldId, delta, 4);
    for (RigidBody* body : bodies) {
        body->syncTransformFromBody();
    }
}

void PhysicsServer::registerBody(RigidBody* body) {
    if (!body) {
        return;
    }
    if (std::find(bodies.begin(), bodies.end(), body) == bodies.end()) {
        bodies.push_back(body);
    }
}

void PhysicsServer::unregisterBody(RigidBody* body) {
    auto it = std::find(bodies.begin(), bodies.end(), body);
    if (it != bodies.end()) {
        bodies.erase(it);
    }
}

void PhysicsServer::setPixelsPerMeter(float value) {
    if (value <= 0.0f) {
        CB_CORE_ERROR("PhysicsServer::setPixelsPerMeter(): value must be positive, got {}", value);
        return;
    }
    pixelsPerMeter = value;
}

void PhysicsServer::setGravity(const glm::vec2& gravityMeters) {
    gravity = gravityMeters;
    if (B2_IS_NON_NULL(worldId)) {
        b2World_SetGravity(worldId, {gravity.x, gravity.y});
    }
}

PhysicsServer* getPhysicsServer() {
    Application* app = Engine::getApp();
    return app ? &app->getPhysicsServer() : nullptr;
}

}  // namespace Cube
