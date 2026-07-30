#pragma once

#include "json.hpp"

#include "Resource.h"

namespace Cube {

    class Atlas : public ResourceBase {
    public:
        Atlas(const std::string& atlasFilePath);
        ~Atlas() = default;

        const nlohmann::json& getSprite(const std::string& name) const;

    private:
        std::unordered_map<std::string, nlohmann::json> sprites;
    };

}
