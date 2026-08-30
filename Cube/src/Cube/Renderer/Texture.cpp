#include "Texture.h"

#include "stb_image.h"

#include "Cube/Core/Log.h"

namespace Cube {

    Texture2D::Texture2D(const Cube::Path& filePath){
        stbi_set_flip_vertically_on_load(1);
        int channels;
        uint8_t* originalData = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        if(!originalData) {
            CB_CORE_ERROR("Failed to load image: {}", filePath);
            CB_ASSERT(0);
        }
        GLenum internalFormat = 0, dataFormat = 0;
        int pixelSize = 3;
        switch(channels) {
        case 3:
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
            pixelSize = 3;
            break;
        case 4:
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
            pixelSize = 4;
            break;
        default:
            CB_CORE_ERROR("Unsupported image format");
            CB_ASSERT(true);
        }

        // alignment
        int rowSize = width * pixelSize;
        int alignedSize = (rowSize + 3) / 4 * 4;
        uint8_t* data = originalData;
        if(rowSize != alignedSize) {
            data = new uint8_t[(size_t)(alignedSize * height)];
            for(int i = 0; i < height; i++) {
                memcpy(data + i * alignedSize, originalData + i * rowSize, rowSize);
                memset(data + i * alignedSize + rowSize, 0, alignedSize - rowSize);
            }
        }

        glGenTextures(1, &id);
        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(originalData);
        if(rowSize != alignedSize) {
            delete[] data;
        }
    }
    
    Texture2D::Texture2D(int width, int height, uint8_t* originalData) : width(width), height(height){
        // alignment
        int rowSize = width * 4;
        int alignedSize = (rowSize + 3) / 4 * 4;
        uint8_t* data = originalData;
        if(rowSize != alignedSize) {
            data = new uint8_t[(size_t)(alignedSize * height)];
            for(int i = 0; i < height; i++) {
                memcpy(data + i * alignedSize, originalData + i * rowSize, rowSize);
                memset(data + i * alignedSize + rowSize, 0, alignedSize - rowSize);
            }
        }

        glGenTextures(1, &id);
        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if(rowSize != alignedSize) {
            delete[] data;
        }
    }

    Texture2D::~Texture2D() {
        glDeleteTextures(1, &id);
    }

    void Texture2D::bind(unsigned int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void Texture2D::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

    // only for RGBA
    void Texture2D::updateData(int x, int y, int w, int h, const unsigned char* data) {
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

}  // namespace Cube