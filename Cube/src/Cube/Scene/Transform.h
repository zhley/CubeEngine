#pragma once

#include "glm/glm.hpp"

#include "Component.h"

namespace Cube {

	class Transform : public Component {
	public:
		glm::vec2 pos = {0.0f, 0.0f};
		float rotation = 0.0f; // degrees
		glm::vec2 scale = {1.0f, 1.0f};

		Transform() = default;
		explicit Transform(Node* node) : Component(node) {}
		~Transform() override = default;
		TypeID getType() const override { return getTypeID<Transform>(); }

		glm::mat4 getLocalMatrix() const;
		glm::mat4 getWorldMatrix() const;
    };
}
