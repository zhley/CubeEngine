#pragma once

#include <stdint.h>
#include <string>

namespace Cube {

    enum class ResourceType : uint8_t{
        Unknown = 0x0,
        Texture,
        Sprite,
        AnimationClip,
        Font,
        Script,
        NodeTree
    };

    inline ResourceType getResType(const std::string& identifier) {
        size_t pos = identifier.find(':');
        std::string type = identifier.substr(0, pos);
        if(type == "tex") return ResourceType::Texture;
        if(type == "spr") return ResourceType::Sprite;
        if(type == "anim") return ResourceType::AnimationClip;
        if(type == "font") return ResourceType::Font;
        if(type == "script") return ResourceType::Script;
        if(type == "node") return ResourceType::NodeTree;
        return ResourceType::Unknown;
    }
    
}
