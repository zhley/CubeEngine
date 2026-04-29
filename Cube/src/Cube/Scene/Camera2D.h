#pragma once

#include "Component.h"

#include <glm/glm.hpp>

namespace Cube {

    class Camera2D : public Component{
    public:
        glm::vec2 viewport = {1280.0f, 720.0f};
        float zoom = 1.0f;
        bool available = true;

        Camera2D();
        ~Camera2D() override;
        TypeID getType() const override { return getTypeID<Camera2D>(); }

        glm::mat4 getPVMatrix() const;
    };
}