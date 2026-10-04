#include "TypeRegister.h"

#include "Cube/Reflection/Serializer.h"
#include "glm/glm.hpp"

#include "Cube/Animation/Animation.h"
#include "Cube/Physics/BoxCollider.h"
#include "Cube/Physics/CircleCollider.h"
#include "Cube/Physics/RigidBody.h"
#include "Cube/Reflection/ClassBuilder.h"
#include "Cube/Renderer/Font.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/Component.h"
#include "Cube/Scene/SpriteRender.h"
#include "Cube/Script/ScriptComponent.h"

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
        .method("play", &Animation::play)
        .method("stop", &Animation::stop)
        .method("addClip", &Animation::addClip)
        .serializer();
    ClassBuilder<ScriptComponent>("ScriptComponent")
        .base<Component>()
        .method("getScript", &ScriptComponent::getScript)
        .method("getName", &ScriptComponent::getName)
        .method("getInstance", &ScriptComponent::getInstance);
    ClassBuilder<RigidBody>("RigidBody")
        .base<Component>()
        .property("bodyType", &RigidBody::bodyType)
        .property("linearVelocity", &RigidBody::linearVelocity)
        .property("angularVelocity", &RigidBody::angularVelocity)
        .property("gravityScale", &RigidBody::gravityScale)
        .property("linearDamping", &RigidBody::linearDamping)
        .property("angularDamping", &RigidBody::angularDamping)
        .property("fixedRotation", &RigidBody::fixedRotation)
        .property("isBullet", &RigidBody::isBullet)
        .property("enabled", &RigidBody::enabled)
        .serializer();
    ClassBuilder<BoxCollider>("BoxCollider")
        .base<Component>()
        .property("size", &BoxCollider::size)
        .property("offset", &BoxCollider::offset)
        .property("density", &BoxCollider::density)
        .property("friction", &BoxCollider::friction)
        .property("restitution", &BoxCollider::restitution)
        .property("isSensor", &BoxCollider::isSensor)
        .serializer();
    ClassBuilder<CircleCollider>("CircleCollider")
        .base<Component>()
        .property("radius", &CircleCollider::radius)
        .property("offset", &CircleCollider::offset)
        .property("density", &CircleCollider::density)
        .property("friction", &CircleCollider::friction)
        .property("restitution", &CircleCollider::restitution)
        .property("isSensor", &CircleCollider::isSensor)
        .serializer();

    // register serializer
    registerBasicSerializers();
    registerSerializer<std::unordered_map<std::string, ResPtr<AnimationClip>>>();
    registerResPtrSerializer<Sprite>();
    registerResPtrSerializer<Texture2D>();
    registerResPtrSerializer<AnimationClip>();
    registerResPtrSerializer<Font>();
    registerResPtrSerializer<Script>();
}

}