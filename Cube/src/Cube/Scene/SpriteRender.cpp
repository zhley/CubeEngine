#include "pch.h"
#include "SpriteRender.h"

#include "Entity.h"
#include "Scene.h"

namespace Cube {

    SpriteRender::~SpriteRender() {
        entity->getScene()->removeRenderableEntity(entity);
    }

    void SpriteRender::start() {
        entity->getScene()->addRenderableEntity(entity);
    }

} // namespace Cube