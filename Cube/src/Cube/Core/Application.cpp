#include "Application.h"

#include <chrono>

#include "Window.h"
#include "Cube/Event/ApplicationEvent.h"
#include "Cube/Renderer/Renderer.h"
#include "Cube/Scene/Scene.h"

namespace Cube {

    Application::Application() : Application({1920, 1080, "Cube Engine"}){}

    Application::Application(const WindowPros& windowPros) : mainWindow(nullptr), running(true){
        mainWindow = new Window(windowPros, &eventDispatcher);
        eventDispatcher.subscribe<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
    }

    Application::~Application() {
        delete mainWindow;
    }

    void Application::run() {
        running = true;
        CB_CORE_INFO("Application run");

        std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();

        while(running) {
            std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<float> frameDuration = currentTime - lastTime;
            lastTime = currentTime;
            float deltaTime = frameDuration.count();

            Renderer::clearBuffer();

            if(Scene* scene = sceneManager.getActive()) {
                scene->update(deltaTime);
                renderServer.renderScene(scene);
            }

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