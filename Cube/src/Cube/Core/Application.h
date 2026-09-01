#pragma once

#include "Window.h"
#include "Cube/Event/Event.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Script/ScriptRuntime.h"
#include "Cube/Scene/RenderServer.h"
#include "Cube/Scene/SceneManager.h"
#include <memory>
#include <vector>

namespace Cube {

class IGameController {
public: 
    virtual ~IGameController() = default;
    virtual void init() {};
    virtual void update(float deltaTime) = 0;
};

class Application {
public:
    Application(const WindowPros& windowPros, const std::vector<std::string>& moduleSearchPaths, std::unique_ptr<IGameController> gameController = nullptr);
    virtual ~Application();

    virtual void run();

    void stop() {
        running = false;
    }
    Window* getWindow() {
        return mainWindow;
    }
    RenderServer& getRenderServer() {
        return renderServer;
    }
    SceneManager& getSceneManager() {
        return sceneManager;
    }
    EventDispatcher& getEventDispatcher() {
        return eventDispatcher;
    }
    ResourceManager& getResourceManager() {
        return resourceManager;
    }
    ScriptRuntime& getScriptRuntime() {
        return scriptRuntime;
    }

    bool onWindowClose(const Event& e);

protected:
    Window* mainWindow;
    std::unique_ptr<IGameController> gameController;
    bool running;
    ResourceManager resourceManager;
    RenderServer renderServer;
    SceneManager sceneManager;
    EventDispatcher eventDispatcher;
    ScriptRuntime scriptRuntime;
};

}
