#include "RenderServer.h"

#include <algorithm>

#include "Camera2D.h"
#include "Node.h"
#include "SpriteRender.h"
#include "Cube/Renderer/Renderer.h"

namespace Cube {

void RenderServer::renderNodeTree(Node* root) {
    Camera2D* camera = nullptr;
    root->forEachNode([&camera](Node* node) {
        camera = node->getComponent<Camera2D>();
        if (camera && camera->available) return false;
        return true;
    });
    if(camera == nullptr || !camera->available) {
        CB_CORE_ERROR("RenderServer::renderNodeTree(): no available Camera2D found in the node tree");
        return;
    }
    std::vector<SpriteRender*> spriteRenders;
    root->forEachNode([&spriteRenders](Node* node) {
        SpriteRender* sprite = node->getComponent<SpriteRender>();
        if (sprite && sprite->sprite) {
            spriteRenders.push_back(sprite);
        }
        return true;
    });
    std::sort(spriteRenders.begin(), spriteRenders.end(), [](const SpriteRender* a, const SpriteRender* b) {
        if(a->order != b->order) {
            return a->order < b->order;
        }
        return (a->sprite->getTexture() ? a->sprite->getTexture()->getId() : -1) < (b->sprite->getTexture() ? b->sprite->getTexture()->getId() : -1);
    });

    Renderer2D::beginFrame(camera->getPVMatrix());
    for(SpriteRender* sprite : spriteRenders) {
        Renderer2D::drawQuad(sprite->getNode()->getWorldMatrix(), sprite->tintColor, sprite->sprite->getTexture(), sprite->sprite->getTexRegion().getUVCoord());
    }
    Renderer2D::endFrame();
}

}  // namespace Cube
