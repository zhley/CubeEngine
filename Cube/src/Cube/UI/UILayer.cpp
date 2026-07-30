#include "UILayer.h"

#include "glm/ext/matrix_clip_space.hpp"

#include "Cube/Renderer/Renderer.h"

namespace Cube {
	void UILayer::onUpdate(float deltaTime) {
        glm::vec2 size = Renderer::getViewport();
		Renderer2D::beginFrame(glm::ortho(0.0f, size.x, size.y, 0.0f, -1.0f, 1.0f));
		for(auto& widget : widgets) {
			widget->render();
		}
		Renderer2D::endFrame();
	}
}  // namespace Cube