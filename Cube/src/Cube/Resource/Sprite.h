#pragma once

#include "ResPtr.h"
#include "Resource.h"
#include "Cube/Renderer/TextureRegion.h"
#include "Cube/Renderer/Texture.h"

namespace Cube {

    class Sprite : public ResourceBase{
    public:
        Sprite(const nlohmann::json& data);
        ~Sprite() = default;

        glm::vec2 getSize() const {
            if(texture) {
                // TODO：如果有旋转，计算方式要改
                return glm::vec2(texture->getWidth(), texture->getHeight()) * (texRegion.uvMax - texRegion.uvMin);
            } else {
                return glm::vec2(1.0f);
            }
        }

        Texture2D* getTexture() const { return texture.get(); }
        const TextureRegion& getTexRegion() const { return texRegion; }

    private:
        ResPtr<Texture2D> texture = nullptr;
        TextureRegion texRegion = {{0.0f, 0.0f}, {1.0f, 1.0f}};
    };

}