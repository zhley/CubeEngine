#include "Application.h"

#include "Window.h"
#include "Cube/Core/Timer.h"
#include "Cube/Event/ApplicationEvent.h"
#include "Cube/Renderer/Renderer.h"
#include "Cube/Scene/Node.h"

namespace Cube {

Application::Application(const WindowPros& windowPros, const std::vector<std::string>& moduleSearchPaths, std::unique_ptr<IGameController> gameController) : mainWindow(nullptr), gameController(std::move(gameController)), running(true), scriptRuntime(moduleSearchPaths) {
    mainWindow = new Window(windowPros, &eventDispatcher);
    eventDispatcher.subscribe<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
    rootNode = std::make_unique<Node>("Root");
}

Application::~Application() {
    delete mainWindow;
}

void Application::run() {
    // init
    if (gameController) gameController->init();

    running = true;
    CB_CORE_INFO("Application run");
    Timer timer;
    while(running) {
        // TODO: 需要限制最大步长
        float deltaTime = static_cast<float>(timer.tick());

        Renderer::clearBuffer();

        if(gameController) gameController->update(deltaTime);

        // The whole game is one rooted node tree.
        rootNode->update(deltaTime);
        physicsServer.step(deltaTime);
        renderServer.renderNodeTree(rootNode.get());

        mainWindow->update();
    }
}

bool Application::onWindowClose(const Event& e) {
    if(static_cast<const WindowCloseEvent*>(&e)->window == mainWindow){
        running = false;
        CB_CORE_INFO("mainWindow close");
        return true;
    }
    return false;
}

}  // namespace Cube
