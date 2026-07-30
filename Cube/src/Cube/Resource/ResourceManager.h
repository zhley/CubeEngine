#pragma once

#include <json.hpp>
#include <list>
#include <unordered_map>

#include "Cube/Core/Log.h"
#include "Cube/Resource/Resource.h"
#include "Resource.h"

namespace Cube {
    class Texture2D;
    class Context;
    class Sprite;
    class AnimationClip;
    class Font;

    class ResourceManager {
    public:
        friend Context;

        // TODO: 改成全局单例
        // thread-local singleton
        static ResourceManager& get();
        static void init(const std::string& pathMapFilePath);
        static void init(const std::unordered_map<std::string, nlohmann::json>& pathMap);

        // Delete copy and move constructors and assignment operators
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        // TODO: 将加载器解耦，支持用户从外部通过脚本注册加载器，自定义类型
        // load
        template<typename T>
        T* load(const std::string& identifier) {
            static_assert(std::is_base_of_v<ResourceBase, T>);
            auto it = resourcesCache.find(identifier);
            if(it != resourcesCache.end()) {
                ResourceBase* resource = it->second.get();
                if(resource->refCount == 0) {
                    saveFromRelease(resource);
                }
                resource->refCount++;
                return static_cast<T*>(resource);
            }
            ResourceBase* newRes = nullptr;
            if constexpr (std::is_same_v<Sprite, T>) {
                newRes = loadSprite(identifier);
            } else {
                auto it2 = pathMap.find(identifier);
                if(it2 == pathMap.end()) {
                    CB_CORE_ERROR("Resource path not found: {}", identifier);
                    return nullptr;
                }
                if constexpr (std::is_same_v<Texture2D, T>) {
                    newRes = loadTexture2D(it2->second);
                }else if constexpr (std::is_same_v<AnimationClip, T>) {
                    newRes = loadAnimationClip(it2->second);
                }else if constexpr (std::is_same_v<Font, T>) {
                    newRes = loadFont(it2->second);
                }else {
                    static_assert(false);
                }
            }
            CB_ASSERT(newRes);
            newRes->refCount = 1;
            newRes->identifier = identifier;
            resourcesCache[identifier] = std::unique_ptr<ResourceBase>(newRes);
            return static_cast<T*>(newRes);
        }

        void release(ResourceBase* resource);
        void release(const std::string& identifier);
        void releaseAll();

        // for CubeEditor
        void reset(const std::unordered_map<std::string, nlohmann::json>& pathMap);

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        std::unordered_map<std::string, std::unique_ptr<ResourceBase>> resourcesCache;
        std::unordered_map<std::string, nlohmann::json> pathMap;

        static constexpr int maxDelayedRelease = 16;
        std::list<ResourceBase*> toRelease; // for delayed release
        std::unordered_map<ResourceBase*, std::list<ResourceBase*>::iterator> toReleaseMap;

        void markRelease(ResourceBase* resource);
        void saveFromRelease(ResourceBase* resource);

        Texture2D* loadTexture2D(const nlohmann::json& path);
        Sprite* loadSprite(const std::string& identifier);
        AnimationClip* loadAnimationClip(const nlohmann::json& path);
        Font* loadFont(const nlohmann::json& path);

    };
}  // namespace Cube