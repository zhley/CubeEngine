#include "pch.h"
#include "RenderServer.h"

#include "Camera2D.h"
#include "Cube/Renderer/Renderer.h"
#include "Scene.h"
#include "SpriteRender.h"

namespace Cube {

    void RenderServer::renderScene(const Scene* scene) {
        Camera2D* camera = nullptr;
        for(auto& entity : scene->getCameras()) {
            Camera2D* cam = entity->getComponent<Camera2D>();
            if(cam != nullptr && cam->available) {
                camera = cam;
                break;
            }
        }
        if(camera == nullptr) {
            CB_CORE_ERROR("RenderServer::renderScene(): no available Camera2D found in the scene");
            return;
        }
        const auto& entities = scene->getRenderableEntities();
        Renderer2D::beginFrame(camera->getPVMatrix());
        for(auto& entity : entities) {
            SpriteRender* sprite = entity->getComponent<SpriteRender>();
            Renderer2D::drawQuad(entity->getTransform().getWorldMatrix(), sprite->tintColor, sprite->sprite->getTexture(), sprite->sprite->getTexRegion().getUVCoord());
        }
        Renderer2D::endFrame();
    }
}  // namespace Cube