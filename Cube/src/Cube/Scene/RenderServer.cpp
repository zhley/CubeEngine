#include "RenderServer.h"

#include <algorithm>

#include "Camera2D.h"
#include "Node.h"
#include "SpriteRender.h"
#include "Cube/Renderer/Renderer.h"

namespace Cube {

    void RenderServer::renderNodeTree(Node* root) {
        Camera2D* camera = nullptr;
        for(Node* node : root->getAllNodes()) {
            if(!node->isAlive()) {
                continue;
            }
            if(Camera2D* cam = node->getComponent<Camera2D>(); cam && cam->available) {
                camera = cam;
                break;
            }
        }
        if(camera == nullptr) {
            CB_CORE_ERROR("RenderServer::renderNodeTree(): no available Camera2D found in the node tree");
            return;
        }
        std::vector<Node*> renderableNodes;
        for(Node* node : root->getAllNodes()) {
            if(node->isAlive() && node->getComponent<SpriteRender>()) {
                renderableNodes.push_back(node);
            }
        }
        std::sort(renderableNodes.begin(), renderableNodes.end(), [](const Node* a, const Node* b) {
            SpriteRender* spriteA = a->getComponent<SpriteRender>();
            SpriteRender* spriteB = b->getComponent<SpriteRender>();
            if(spriteA->order != spriteB->order) {
                return spriteA->order < spriteB->order;
            }
            return (spriteA->sprite && spriteA->sprite->getTexture() ? spriteA->sprite->getTexture()->getId() : -1) < (spriteB->sprite && spriteB->sprite->getTexture() ? spriteB->sprite->getTexture()->getId() : -1);
        });

        Renderer2D::beginFrame(camera->getPVMatrix());
        for(Node* node : renderableNodes) {
            SpriteRender* sprite = node->getComponent<SpriteRender>();
            Renderer2D::drawQuad(node->getTransform().getWorldMatrix(), sprite->tintColor, sprite->sprite->getTexture(), sprite->sprite->getTexRegion().getUVCoord());
        }
        Renderer2D::endFrame();
    }
}  // namespace Cube
