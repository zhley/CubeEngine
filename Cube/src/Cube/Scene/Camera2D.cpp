#include "Camera2D.h"

#include "glm/ext/matrix_clip_space.hpp"

#include "Node.h"

namespace Cube {

    glm::mat4 Camera2D::getPVMatrix() const {
        glm::mat4 proj = glm::ortho(0.0f, viewport.x / zoom, 0.0f, viewport.y / zoom, -0.1f, 1.0f);
        glm::mat4 view = glm::inverse(node->getWorldMatrix());
        return proj * view;
    }

}
