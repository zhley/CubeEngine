#pragma once

#include "glm/glm.hpp"

namespace Cube {

    struct TextureRegion {
        glm::vec2 uvMin;
        glm::vec2 uvMax;

        glm::vec4 getUVCoord() const { return glm::vec4(uvMin.x, uvMin.y, uvMax.x, uvMax.y); }
    };

}