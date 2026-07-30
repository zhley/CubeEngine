#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Cube {

    // Camera2D::Camera2D(float viewportWidth, float viewportHeight, const glm::vec2& pos, float zoom, float rotation) : position(pos), zoom(zoom), rotation(rotation) {
    //     projectMatrix = glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight, -0.1f, 1.0f);
    //     recalculatePVMatrix();
    // }
    //
    // void Camera2D::setZoom(float zoom) {
    //     this->zoom = zoom;
    //     recalculatePVMatrix();
    // }
    //
    // void Camera2D::setRotation(float rotation) {
    //     this->rotation = rotation;
    //     recalculatePVMatrix();
    // }
    //
    // void Camera2D::setPosition(const glm::vec2& pos) {
    //     this->position = pos;
    //     recalculatePVMatrix();
    // }
    //
    // const glm::mat4& Camera2D::getPVMatrix() const {
    //     return projectViewMatrix;
    // }
    //
    // void Camera2D::recalculatePVMatrix() {
    //     glm::mat4 viewMatrix = glm::inverse( glm::rotate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)), glm::vec3(zoom, zoom, 1.0f)), glm::radians(rotation), glm::vec3(0,0,1)));
    //     projectViewMatrix = projectMatrix * viewMatrix;
    // }

}  // namespace Cube