#include "Engine.h"

#include "TypeRegister.h"

namespace Cube {

    std::unique_ptr<Application> Engine::application;

    void Engine::init() {
        Log::init();
        registerTypes();
    }

    void Engine::setApp(Application* app) {
        application.reset(app);
    }

    Application* Engine::getApp() {
        return application.get();
    }

    void Engine::shutdown() {
        application.reset();
    }

}  // namespace Cube