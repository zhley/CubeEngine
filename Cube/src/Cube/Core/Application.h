#pragma once

#include "Window.h"
#include "Cube/Event/Event.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Script/ScriptRuntime.h"
#include "Cube/Scene/RenderServer.h"
#include <memory>
#include <vector>

namespace Cube {

class Node;

class IGameController {
public: 
    virtual ~IGameController() = default;
    virtual void init() {};
    virtual void update(float deltaTime) = 0;
};

// TODO: 有些错误处理太复杂, 将来可能还是要加上异常

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
    EventDispatcher& getEventDispatcher() {
        return eventDispatcher;
    }
    ResourceManager& getResourceManager() {
        return resourceManager;
    }
    ScriptRuntime& getScriptRuntime() {
        return scriptRuntime;
    }
    Node* getRootNode() {
        return rootNode.get();
    }

    bool onWindowClose(const Event& e);

protected:
    Window* mainWindow;
    std::unique_ptr<IGameController> gameController;
    bool running;
    ResourceManager resourceManager;
    RenderServer renderServer;
    EventDispatcher eventDispatcher;
    ScriptRuntime scriptRuntime;
    std::unique_ptr<Node> rootNode;
};

}
