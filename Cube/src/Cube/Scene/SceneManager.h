#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

#include "Cube/Core/Path.h"

namespace Cube {
    class Scene;

    class SceneManager {
    public:
        using SceneFactory = std::function<Scene*()>;
        SceneManager() = default;
        ~SceneManager() = default;

        void registerScene(const std::string& name, const SceneFactory& sceneFactory);
        void registerScene(const std::string& name, const Cube::Path& sceneFilePath);
        Scene* load(const std::string& name);
        void unload(const std::string& name);
        void setActive(const std::string& name);
        Scene* getActive() const;

    private:
        std::unordered_map<std::string, SceneFactory> scenes;
        std::unordered_map<std::string, std::unique_ptr<Scene>> loadedScenes;
        Scene* activeScene = nullptr;
    };

}