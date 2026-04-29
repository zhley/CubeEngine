#include "pch.h"
#include "Camera2D.h"

#include "Entity.h"
#include "Scene.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Cube {

    Camera2D::Camera2D() {
        entity->getScene()->addCamera(entity);
    }

    Camera2D::~Camera2D() {
        entity->getScene()->removeCamera(entity);
    }

    glm::mat4 Camera2D::getPVMatrix() const {
        float halfW = viewport.x * 0.5f / zoom;
        float halfH = viewport.y * 0.5f / zoom;
        glm::mat4 proj = glm::ortho(-halfW, halfW, -halfH, halfH, -0.1f, 1.0f);
        glm::mat4 view = glm::inverse(entity->getTransform().getWorldMatrix());
        return proj * view;
    }

}