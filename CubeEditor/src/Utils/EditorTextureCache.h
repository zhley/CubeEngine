#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Cube/Core/Path.h"
#include "Cube/Renderer/Texture.h"

class EditorTextureCache {
public:
    // TODO: 不用单例，改成 EditorPage 的成员
    static EditorTextureCache& get() {
        static EditorTextureCache instance;
        return instance;
    }

    Cube::Texture2D* request(const std::string& path) {
        if(path.empty()) {
            return nullptr;
        }
        auto it = textures.find(path);
        if(it == textures.end()) {
            textures[path] = std::make_unique<Cube::Texture2D>(Cube::Path(path));
            it = textures.find(path);
        }
        return it->second.get();
    }

    void clear() {
        textures.clear();
    }

private:
    EditorTextureCache() = default;
    ~EditorTextureCache() = default;

    EditorTextureCache(const EditorTextureCache&) = delete;
    EditorTextureCache& operator=(const EditorTextureCache&) = delete;
    EditorTextureCache(EditorTextureCache&&) = delete;
    EditorTextureCache& operator=(EditorTextureCache&&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Cube::Texture2D>> textures;
};
