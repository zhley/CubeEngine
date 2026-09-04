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

    Engine::setApp(new Application({1920, 1080, "test02_script"}, {"D:/mycode/vsProject/CubeEngine/Cube/scripts"}));
    Application* app = Engine::getApp();
    // 注册脚本资源路径, 使 ResPtr<Script>("script:player") 可被加载
    std::unordered_map<std::string, nlohmann::json> pathMap;
    pathMap["script:player"] = {{"path", "D:/mycode/vsProject/CubeEngine/Test/CubeTest/scripts/Player.zt"}};
    pathMap["tex:icon.png"] = {{"path", "D:/mycode/vsProject/CubeEngine/Test/CubeEditorProject/Test01/Assets/icon.png"}};
    app->getResourceManager().init(pathMap);
    app->getSceneManager().registerScene("scene", [&app]() {
        Scene* scene = new Scene("scene");

        auto cameraEntity = scene->createEntity("camera");
        cameraEntity->addComponent<Camera2D>();

        auto entity = scene->createEntity("player");
        ScriptComponent* component = entity->addComponent<ScriptComponent>();
        component->script.reset("script:player");
        component->name = "Player";
        return scene;
    });
    app->getSceneManager().load("scene");
    app->getSceneManager().setActive("scene");
    app->run();

    Engine::shutdown();
}

int main() {
    testScriptIntegration();
    return 0;
}
