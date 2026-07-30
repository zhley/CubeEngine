#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "glad/glad.h"

namespace Cube {

    enum class ShaderDataType {
        None = 0,
        Float, Float2, Float3, Float4,
        Int, Int2, Int3, Int4,
        Mat3, Mat4,
        Bool
    };

    uint32_t shaderDataTypeSize(ShaderDataType type);

    GLenum shaderDataTypeToOpenGlBaseType(ShaderDataType type);

    uint32_t shaderDataTypeCount(ShaderDataType type);

    struct BufferElement {
        std::string name;
        ShaderDataType type;
        uint32_t size;
        uint32_t offset;
        bool normalized;

        BufferElement(ShaderDataType type, const std::string name, bool normalized = false):name(name), type(type), size(shaderDataTypeSize(type)), offset(0), normalized(normalized){}

    };

    class BufferLayout {
    public:
        BufferLayout() = default;
        BufferLayout(const std::initializer_list<BufferElement>& elements);
        ~BufferLayout() = default;

        void addElement(const BufferElement& element);
        const std::vector<BufferElement>& getElements() const;
        uint32_t getStride() const;
    private:
        std::vector<BufferElement> elements;
        uint32_t stride;
    };

}