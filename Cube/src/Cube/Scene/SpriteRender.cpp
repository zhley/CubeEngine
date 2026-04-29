#include "pch.h"
#include "SpriteRender.h"

#include "Entity.h"
#include "Scene.h"

namespace Cube {

    SpriteRender::SpriteRender() {
        entity->getScene()->addRenderableEntity(entity);
    }

    SpriteRender::SpriteRender(const std::string& sprite, const Color& tintColor) : sprite(sprite), tintColor(tintColor) {
        entity->getScene()->addRenderableEntity(entity);
    }

    SpriteRender::~SpriteRender() {
        entity->getScene()->removeRenderableEntity(entity);
    }

} // namespace Cube