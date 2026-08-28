#include "Engine.h"

#include "TypeRegister.h"
#include "Log.h"

namespace Cube {

    Application* Engine::application = nullptr;

    void Engine::init() {
        Log::init();
        registerTypes();
    }

    void Engine::setApp(Application* app) {
        if (application) {
            CB_CORE_WARN("Engine::setApp: application already set, will delete the old one.");
            delete application;
        }
        application = app;
    }

    Application* Engine::getApp() {
        return application;
    }

    void Engine::shutdown() {
        delete application;
        application = nullptr;
    }

}  // namespace Cube