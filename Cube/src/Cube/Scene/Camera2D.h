#pragma once

#include "glm/glm.hpp"

#include "Component.h"

namespace Cube {

    class Camera2D : public Component{
    public:
        glm::vec2 viewport = {1280.0f, 720.0f};
        float zoom = 1.0f;
        bool available = true;

        Camera2D() = default;
        ~Camera2D() override = default;
        TypeID getType() const override { return getTypeID<Camera2D>(); }

        glm::mat4 getPVMatrix() const;
    };
    
}
