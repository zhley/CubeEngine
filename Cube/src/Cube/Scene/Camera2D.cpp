#include "pch.h"
#include "Camera2D.h"

#include "Entity.h"
#include "Scene.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Cube {

    Camera2D::~Camera2D() {
        entity->getScene()->removeCamera(entity);
    }

    void Camera2D::start() {
        entity->getScene()->addCamera(entity);
    }

    glm::mat4 Camera2D::getPVMatrix() const {
        glm::mat4 proj = glm::ortho(0.0f, viewport.x / zoom, 0.0f, viewport.y / zoom, -0.1f, 1.0f);
        glm::mat4 view = glm::inverse(entity->getTransform().getWorldMatrix());
        return proj * view;
    }

}