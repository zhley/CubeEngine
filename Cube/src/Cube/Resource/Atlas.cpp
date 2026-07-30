#include "Atlas.h"

#include <fstream>

#include "Cube/Core/Log.h"

namespace Cube {

    Atlas::Atlas(const std::string& atlasFilePath) {
        std::ifstream file(atlasFilePath);
        if(!file.is_open()) {
            CB_CORE_ERROR("Failed to open atlas file: {}", atlasFilePath);
            return;
        }
        nlohmann::json atlasData;
        file >> atlasData;
        file.close();
        for(auto& sprite : atlasData["sprites"].items()) {
            sprites[sprite.key()] = sprite.value();
        }
    }

    const nlohmann::json& Atlas::getSprite(const std::string& name) const {
        auto it = sprites.find(name);
        if(it == sprites.end()) {
            CB_CORE_ERROR("Sprite not found in atlas: {}", name);
            static nlohmann::json emptyJson;
            return emptyJson;
        }
        return it->second;
    }

}  // namespace Cube