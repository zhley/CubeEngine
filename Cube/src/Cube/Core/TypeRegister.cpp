#include "TypeRegister.h"

#include "glm/glm.hpp"

#include "Cube/Animation/Animation.h"
#include "Cube/Reflection/ClassBuilder.h"
#include "Cube/Renderer/Font.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/Component.h"
#include "Cube/Scene/SpriteRender.h"

namespace Cube {

	void registerTypes() {
		// glm
		ClassBuilder<glm::vec1>("vec1")
			.property("x", &glm::vec1::x)
            .serializer();
		ClassBuilder<glm::vec2>("vec2")
			.property("x", &glm::vec2::x)
			.property("y", &glm::vec2::y)
			.serializer();
		ClassBuilder<glm::vec3>("vec3")
			.property("x", &glm::vec3::x)
			.property("y", &glm::vec3::y)
			.property("z", &glm::vec3::z)
			.serializer();
		ClassBuilder<glm::vec4>("vec4")
			.property("x", &glm::vec4::x)
			.property("y", &glm::vec4::y)
			.property("z", &glm::vec4::z)
			.property("w", &glm::vec4::w)
			.serializer();

		// TextureRegion
		ClassBuilder<TextureRegion>("TextureRegion")
			.property("uvMin", &TextureRegion::uvMin)
			.property("uvMax", &TextureRegion::uvMax)
			.serializer();

		// Color
		ClassBuilder<Color>("Color")
			.property("r", &Color::r)
			.property("g", &Color::g)
			.property("b", &Color::b)
			.property("a", &Color::a)
			.serializer();

		// Component
	    // ClassBuilder<Component>("Component").serializer(); // Component is abstract, no need to register
		ClassBuilder<SpriteRender>("SpriteRender")
			.base<Component>()
			.property("sprite", &SpriteRender::sprite)
			.property("tintColor", &SpriteRender::tintColor)
			.property("order", &SpriteRender::order)
			.serializer();
		ClassBuilder<Camera2D>("Camera2D")
			.base<Component>()
			.property("viewport", &Camera2D::viewport)
			.property("zoom", &Camera2D::zoom)
		    .property("available", &Camera2D::available)
			.serializer();
		ClassBuilder<Animation>("Animation")
			.base<Component>()
			.property("clips", &Animation::clips)
			.serializer();

		// register serializer
		registerBasicSerializers();
		registerSerializer<std::unordered_map<std::string, ResPtr<AnimationClip>>>();
		registerResPtrSerializer<Sprite>();
		registerResPtrSerializer<Texture2D>();
		registerResPtrSerializer<AnimationClip>();
		registerResPtrSerializer<Font>();
	}
}