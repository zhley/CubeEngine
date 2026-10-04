#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Cube/Core/Application.h"
#include "Cube/Core/Engine.h"
#include "Cube/Physics/BoxCollider.h"
#include "Cube/Physics/CircleCollider.h"
#include "Cube/Physics/RigidBody.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/Node.h"
#include "Cube/Scene/SpriteRender.h"

using namespace Cube;

// Absolute path to the single-white-pixel texture shipped with the tests.
constexpr const char* kPixelTexturePath = "D:/mycode/vsProject/CubeEngine/Test/CubeTest/assets/pixel.png";
constexpr const char* kPixelSpriteIdentifier = "spr:tex:pixel.png";
constexpr const char* kPhysicsScriptPath = "D:/mycode/vsProject/CubeEngine/Cube/scripts";

// The renderer draws a sprite quad with its origin at the node and scales it by
// the node scale, while a Box2D shape is centered on the body origin. Matching
// node scale to the shape size and offsetting the shape by half its size keeps
// the visual block and the collider aligned.
Node* addBox(Node* parent, const std::string& name, const glm::vec2& position, const glm::vec2& size, const Color& color, int bodyType) {
    Node* node = parent->addChild(name);
    node->pos = position;
    node->scale = size;

    RigidBody* rigidBody = node->addComponent<RigidBody>();
    rigidBody->bodyType = bodyType;

    BoxCollider* collider = node->addComponent<BoxCollider>();
    collider->size = size;
    collider->offset = {size.x * 0.5f, size.y * 0.5f};
    collider->friction = 0.6f;

    SpriteRender* sprite = node->addComponent<SpriteRender>();
    sprite->sprite.reset(kPixelSpriteIdentifier);
    sprite->tintColor = color;
    return node;
}

Node* addCircle(Node* parent, const std::string& name, const glm::vec2& position, float radius, const Color& color, int bodyType) {
    Node* node = parent->addChild(name);
    node->pos = position;
    node->scale = {radius * 2.0f, radius * 2.0f};

    RigidBody* rigidBody = node->addComponent<RigidBody>();
    rigidBody->bodyType = bodyType;

    CircleCollider* collider = node->addComponent<CircleCollider>();
    collider->radius = radius;
    collider->offset = {radius, radius};
    collider->friction = 0.6f;

    SpriteRender* sprite = node->addComponent<SpriteRender>();
    sprite->sprite.reset(kPixelSpriteIdentifier);
    sprite->tintColor = color;
    return node;
}

// Walks the dynamic bodies every second and prints a simple verdict at the end:
// every observed body must fall and must not sink through the ground.
class PhysicsSmokeController : public IGameController {
public:
    struct Observed {
        Node* node;
        float initialY;
    };

    std::vector<Observed> observed;
    float duration = 10.0f;
    float reportAt = 6.0f;

    void update(float deltaTime) override {
        elapsed += deltaTime;
        logTimer += deltaTime;
        if(logTimer >= 1.0f) {
            logTimer = 0.0f;
            logPositions();
        }
        if(!reported && elapsed >= reportAt) {
            reported = true;
            report();
        }
        if(elapsed >= duration) {
            Engine::getApp()->stop();
        }
    }

private:
    float elapsed = 0.0f;
    float logTimer = 0.0f;
    bool reported = false;

    void logPositions() {
        std::cout << "[03_physics] t=" << elapsed << "s:";
        for(const Observed& item : observed) {
            std::cout << ' ' << item.node->getName() << "=(" << item.node->pos.x << ", " << item.node->pos.y << ')';
        }
        std::cout << std::endl;
    }

    void report() {
        bool passed = true;
        for(const Observed& item : observed) {
            const float finalY = item.node->pos.y;
            const bool fell = finalY < item.initialY - 1.0f;
            const bool stayedAboveGround = finalY >= -1.0f;
            if(!fell || !stayedAboveGround) {
                passed = false;
            }
            std::cout << "[03_physics] " << item.node->getName() << ": initialY=" << item.initialY << " finalY=" << finalY
                      << " fell=" << (fell ? "yes" : "no") << " aboveGround=" << (stayedAboveGround ? "yes" : "no") << std::endl;
        }
        std::cout << "[03_physics] RESULT: " << (passed ? "PASS" : "FAIL") << std::endl;
    }
};

int main() {
    Engine::init();

    auto controller = std::make_unique<PhysicsSmokeController>();
    PhysicsSmokeController* controllerPtr = controller.get();

    Engine::setApp(new Application({1280, 720, "03_physics - Box2D smoke test"}, {kPhysicsScriptPath}, std::move(controller)));
    Application* app = Engine::getApp();

    std::unordered_map<std::string, nlohmann::json> pathMap;
    pathMap["tex:pixel.png"] = {{"path", kPixelTexturePath}};
    app->getResourceManager().init(pathMap);

    Node* root = app->getRootNode();
    Node* camera = root->addChild("camera");
    camera->addComponent<Camera2D>();

    // Static ground covering the bottom of the visible area (world Y points up).
    addBox(root, "ground", {0.0f, 0.0f}, {1280.0f, 40.0f}, Color(0.35f, 0.35f, 0.35f, 1.0f), b2_staticBody);

    // Dynamic bodies dropped from above.
    Node* boxRed = addBox(root, "box_red", {560.0f, 420.0f}, {80.0f, 80.0f}, RED, b2_dynamicBody);
    Node* boxGreen = addBox(root, "box_green", {660.0f, 520.0f}, {70.0f, 70.0f}, GREEN, b2_dynamicBody);
    Node* boxBlue = addBox(root, "box_blue", {840.0f, 640.0f}, {110.0f, 40.0f}, BLUE, b2_dynamicBody);
    Node* boxCyan = addBox(root, "box_cyan", {560.0f, 620.0f}, {50.0f, 50.0f}, CYAN, b2_dynamicBody);
    Node* ballYellow = addCircle(root, "ball_yellow", {900.0f, 560.0f}, 40.0f, YELLOW, b2_dynamicBody);

    controllerPtr->observed.push_back({boxRed, boxRed->pos.y});
    controllerPtr->observed.push_back({boxGreen, boxGreen->pos.y});
    controllerPtr->observed.push_back({boxBlue, boxBlue->pos.y});
    controllerPtr->observed.push_back({boxCyan, boxCyan->pos.y});
    controllerPtr->observed.push_back({ballYellow, ballYellow->pos.y});

    std::cout << "==== 03_physics: Box2D smoke test ====" << std::endl;
    std::cout << "Expect: a grey ground bar stays at the bottom, and the white blocks / square fall under gravity," << std::endl;
    std::cout << "collide with the ground and each other, then settle. Every observed body moves down and stays above y=0." << std::endl;
    std::cout << "(The yellow body uses a circle collider but is drawn as a square because the texture is 1x1.)" << std::endl;

    app->run();

    Engine::shutdown();
    return 0;
}
