#include "Transform.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"

#include "Entity.h"


namespace Cube {

    glm::mat4 Transform::getLocalMatrix() const {
        glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
        localMatrix = glm::rotate(localMatrix, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        localMatrix = glm::scale(localMatrix, glm::vec3(scale, 1.0f));
        return localMatrix;
    }

    glm::mat4 Transform::getWorldMatrix() const {
        glm::mat4 worldMatrix = getLocalMatrix();
        if(Entity* parent = entity->getParent()) {
            worldMatrix = parent->getTransform().getWorldMatrix() * worldMatrix;
        }
        return worldMatrix;
    }
}  // namespace Cube