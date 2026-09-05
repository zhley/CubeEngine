#include "ResourceManager.h"

#include <fstream>

#include "Script.h"
#include "Sprite.h"
#include "Cube/Animation/AnimationClip.h"
#include "Cube/Renderer/Font.h"
#include "NodeTree.h"

namespace Cube {

    void ResourceManager::init(const Cube::Path& pathMapFilePath) {
        std::ifstream file(pathMapFilePath.string());
        if(!file.is_open()) {
            CB_CORE_ERROR("Failed to open resource path map file: {}", pathMapFilePath);
        }
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        pathMap = jsonData.get<std::unordered_map<std::string, nlohmann::json>>();
    }

    void ResourceManager::init(const std::unordered_map<std::string, nlohmann::json>& pathMap) {
        this->pathMap = pathMap;
    }

    void ResourceManager::release(ResourceBase* resource) {
        if(!resource)
            return;
        resource->refCount--;
        if(resource->refCount == 0) {
            markRelease(resource);
        }
    }

    void ResourceManager::release(const std::string& identifier) {
        auto it = resourcesCache.find(identifier);
        if(it != resourcesCache.end()) {
            if((--it->second->refCount) == 0) {
                markRelease(it->second.get());
            }
        }
    }

    // releaseAll不应该被随意调用，因为可能有资源正在被使用，直接删除会导致悬空指针
    void ResourceManager::releaseAll() {
        resourcesCache.clear();
        toRelease.clear();
        toReleaseMap.clear();
    }

    void ResourceManager::reset(const std::unordered_map<std::string, nlohmann::json>& pathMap) {
        this->pathMap = pathMap;
    } 

    void ResourceManager::markRelease(ResourceBase* resource) {
        if(toReleaseMap.count(resource)) {
            return;
        }
        toRelease.push_back(resource);
        toReleaseMap[resource] = std::prev(toRelease.end());
        if(toRelease.size() > maxDelayedRelease) {
            ResourceBase* toSave = toRelease.front();
            saveFromRelease(toSave);
            toRelease.pop_front();
            toReleaseMap.erase(toSave);
            resourcesCache.erase(toSave->identifier);
        }
    }

    void ResourceManager::saveFromRelease(ResourceBase* resource) {
        auto it = toReleaseMap.find(resource);
        if(it != toReleaseMap.end()) {
            toRelease.erase(it->second);
            toReleaseMap.erase(it);
        }
    }
        

    Texture2D* ResourceManager::loadTexture2D(const nlohmann::json& path) {
        return new Texture2D(Cube::Path(path["path"].get<std::string>()));
    }

    Sprite* ResourceManager::loadSprite(const std::string& identifier) {
        size_t pos1 = identifier.find(':');
        size_t pos2 = identifier.find('#');
        if(pos2 == std::string::npos) {
            // spr:tex:abc.png
            nlohmann::json data;
            data["texture"] = identifier.substr(pos1 + 1);
            data["texRegion"] = {0.0f, 0.0f, 1.0f, 1.0f};
            return new Sprite(data);
        }else {
            // spr:tex:123#a
            nlohmann::json data;
            data["texture"] = identifier.substr(pos1 + 1, pos2 - pos1 - 1);
            auto it = pathMap.find(identifier.substr(pos1 + 1, pos2 - pos1 - 1));
            CB_ASSERT(it != pathMap.end());
            data["texRegion"] = it->second["sprites"][identifier.substr(pos2 + 1)];
            return new Sprite(data);
        }
    }

    Script* ResourceManager::loadScript(const nlohmann::json& path) {
        return new Script(Cube::Path(path["path"].get<std::string>()));
    }

    AnimationClip* ResourceManager::loadAnimationClip(const nlohmann::json& path) {
        return new AnimationClip(Cube::Path(path["path"].get<std::string>()));
    }

    Font* ResourceManager::loadFont(const nlohmann::json& path) {
        return new Font(Cube::Path(path["fontFilePath"].get<std::string>()), path.value("fontSize", 16));
    }

    NodeTree* ResourceManager::loadNodeTree(const nlohmann::json& path) {
        return new NodeTree(Cube::Path(path["path"].get<std::string>()));
    }
    
}  // namespace Cube
