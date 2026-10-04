#pragma once

#include <vector>

#include "box2d/box2d.h"
#include "glm/glm.hpp"

namespace Cube {

class RigidBody;

// Owns the Box2D world and drives the simulation. It follows the same service
// pattern as RenderServer: Application owns and steps it, while physics
// components register themselves so their transforms can be synced back.
//
// The engine works in pixels while Box2D works in meters, so every value that
// crosses the boundary is converted through this server using a configurable
// pixels-per-meter ratio.
class PhysicsServer {
public:
    PhysicsServer();
    ~PhysicsServer();

    PhysicsServer(const PhysicsServer&) = delete;
    PhysicsServer& operator=(const PhysicsServer&) = delete;

    // Advance the world by delta seconds and write the resulting body
    // transforms back to their nodes.
    void step(float delta);

    void registerBody(RigidBody* body);
    void unregisterBody(RigidBody* body);

    b2WorldId getWorldId() const { return worldId; }

    float getPixelsPerMeter() const { return pixelsPerMeter; }
    void setPixelsPerMeter(float value);

    glm::vec2 getGravity() const { return gravity; }
    // Gravity in m/s^2. The engine's Y axis points up, so the default is -9.8.
    void setGravity(const glm::vec2& gravityMeters);

    float toMeters(float pixels) const { return pixels / pixelsPerMeter; }
    float toPixels(float meters) const { return meters * pixelsPerMeter; }
    b2Vec2 toMeters(const glm::vec2& pixels) const { return {pixels.x / pixelsPerMeter, pixels.y / pixelsPerMeter}; }
    glm::vec2 toPixels(const b2Vec2& meters) const { return {meters.x * pixelsPerMeter, meters.y * pixelsPerMeter}; }

private:
    b2WorldId worldId = b2_nullWorldId;
    float pixelsPerMeter = 100.0f;
    glm::vec2 gravity = {0.0f, -9.8f};
    std::vector<RigidBody*> bodies;
};

// Returns the physics server of the active application, or nullptr if there is none.
PhysicsServer* getPhysicsServer();

}  // namespace Cube
