#include "Sprite.h"

#include "json.hpp"

namespace Cube {

    Sprite::Sprite(const nlohmann::json& data) {
        texture = ResPtr<Texture2D>(data["texture"]);
        texRegion = {
            { data["texRegion"][0], data["texRegion"][1] },
            { data["texRegion"][2], data["texRegion"][3] }
        };
    }

}  // namespace Cube