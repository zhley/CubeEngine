#include "SceneManager.h"

#include "Scene.h"
#include "Cube/Core/Log.h"

namespace Cube {

    void SceneManager::registerScene(const std::string& name, const SceneFactory& sceneFactory) {
        if(scenes.find(name) != scenes.end()) {
            CB_CORE_WARN("SceneManager::registerScene(): Scene '{}' is already registered. Overwriting.", name);
        }
        scenes[name] = sceneFactory;
    }

    void SceneManager::registerScene(const std::string& name, const std::string& sceneFilePath) {
        if(scenes.find(name) != scenes.end()) {
            CB_CORE_WARN("SceneManager::registerScene(): Scene '{}' is already registered. Overwriting.", name);
        }
        scenes[name] = [sceneFilePath] {
            return new Scene(sceneFilePath);
        };
    }

    Scene* SceneManager::load(const std::string& name) {
        auto it = loadedScenes.find(name);
        if(it != loadedScenes.end()) {
            CB_CORE_WARN("SceneManager::load(): Scene {} is already loaded.");
            return it->second.get();
        }
        auto factoryIt = scenes.find(name);
        if(factoryIt == scenes.end()) {
            CB_CORE_ERROR("SceneManager::load(): Scene '{}' is not registered.", name);
            return nullptr;
        }
        Scene* scene = factoryIt->second();
        loadedScenes[name] = std::unique_ptr<Scene>(scene);
        return scene;
    }

    void SceneManager::unload(const std::string& name) {
        auto it = loadedScenes.find(name);
        if(it == loadedScenes.end()) {
            CB_CORE_WARN("SceneManager::unload(): Scene '{}' is not loaded.", name);
            return;
        }
        if(activeScene == it->second.get()) {
            CB_CORE_WARN("SceneManager::unload(): An active scene is unloaded.");
            activeScene = nullptr;
        }
        loadedScenes.erase(it);
    }

    void SceneManager::setActive(const std::string& name) {
        auto it = loadedScenes.find(name);
        if(it == loadedScenes.end()) {
            CB_CORE_ERROR("SceneManager::setActive(): Scene '{}' is not loaded.", name);
            return;
        }
        activeScene = it->second.get();
    }

    Scene* SceneManager::getActive() const {
        return activeScene;
    }
}  // namespace Cube