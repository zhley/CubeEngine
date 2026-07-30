#pragma once

#include "glm/vec2.hpp"

#include "KeyCodes.h"
#include "MouseCodes.h"


namespace Cube {

    class Input {
    public:
        static bool isMouseButtonPressed(MouseCode code);
        static bool isKeyPressed(KeyCode code);

        static glm::vec2 getMousePos();
        static float getMousePosX();
        static float getMousePosY();
    };

}