#include "Cube/Core/Engine.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/Entity.h"
#include "Cube/Script/ScriptComponent.h"
#include "Cube/Scene/Scene.h"

using namespace Cube;

// 脚本集成冒烟测试: 验证 parseModule -> loadModule -> instantiate -> start/update 全链路
void testScriptIntegration() {
    Engine::init();

    Engine::setApp(new Application());
    Application* app = Engine::getApp();
    // 注册脚本资源路径, 使 ResPtr<Script>("script:player") 可被加载
    std::unordered_map<std::string, nlohmann::json> pathMap;
    pathMap["script:player"] = {{"path", "Test/CubeTest/scripts/Player.zt"}};
    app->getResourceManager().init(pathMap);
    app->getSceneManager().registerScene("scene", [&app]() {
        Scene* scene = new Scene("scene", true);
        auto entity = scene->createEntity("player");
        ScriptComponent* component = entity->addComponent<ScriptComponent>();
        component->script.reset("script:player");
        component->name = "Player";
        entity->addComponent<Camera2D>();
        return scene;
    });
    app->getSceneManager().load("scene");
    app->getSceneManager().setActive("scene");
    app->run();
}

int main() {
    testScriptIntegration();
    return 0;
}
