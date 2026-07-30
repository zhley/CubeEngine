#pragma once

#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "Cube/Resource/Resource.h"

namespace Cube {

    class Texture2D: public ResourceBase{
    public:
        Texture2D(const std::string& filePath);
        Texture2D(int width, int height, uint8_t* data);
        virtual ~Texture2D() override;
        void bind(unsigned int slot = 0) const;
        void unbind();
        void updateData(int x, int y, int w, int h, const unsigned char* data);

        int getWidth() const { return width; }
        int getHeight() const { return height; }
        glm::vec2 getSize() const {return { static_cast<float>(width), static_cast<float>(height)}; }
        GLuint getId() const { return id; }
    private:
        int width;
        int height;
        GLuint id;
    };

    struct SubTexture {
        std::string name;
        glm::vec2 uvMin;
        glm::vec2 uvMax;
        glm::vec2 position;
        glm::vec2 size;
    };

}