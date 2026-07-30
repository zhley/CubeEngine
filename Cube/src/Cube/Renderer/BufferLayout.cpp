#include "BufferLayout.h"

#include "Cube/Core/Log.h"

// clang-format off
namespace Cube {

    uint32_t shaderDataTypeSize(ShaderDataType type) {
        switch (type) {
        case ShaderDataType::Float:    return 4;
        case ShaderDataType::Float2:   return 4 * 2;
        case ShaderDataType::Float3:   return 4 * 3;
        case ShaderDataType::Float4:   return 4 * 4;
        case ShaderDataType::Int:      return 4;
        case ShaderDataType::Int2:     return 4 * 2;
        case ShaderDataType::Int3:     return 4 * 3;
        case ShaderDataType::Int4:     return 4 * 4;
        case ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ShaderDataType::Bool:     return 1;
        default:
            CB_CORE_ERROR("shaderDataTypeSize : Unknown type!");
            return 0;
        }
    }

    GLenum shaderDataTypeToOpenGlBaseType(ShaderDataType type) {
        switch(type) {
        case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4:
        case ShaderDataType::Mat3:
        case ShaderDataType::Mat4: return GL_FLOAT;
        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4: return GL_INT;
        case ShaderDataType::Bool: return GL_BOOL;
        default:
            CB_CORE_ERROR("shaderDataTypeToOpenGlBaseType : Unknow type!");
            return 0;
        }
    }
    uint32_t shaderDataTypeCount(ShaderDataType type) {
        switch(type) {
        case ShaderDataType::Float:    return 1;
        case ShaderDataType::Float2:   return 2;
        case ShaderDataType::Float3:   return 3;
        case ShaderDataType::Float4:   return 4;
        case ShaderDataType::Int:      return 1;
        case ShaderDataType::Int2:     return 2;
        case ShaderDataType::Int3:     return 3;
        case ShaderDataType::Int4:     return 4;
        case ShaderDataType::Mat3:     return 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4;
        case ShaderDataType::Bool:     return 1;
        default:
            CB_CORE_ERROR("shaderDataTypeCount : Unknown type!");
            return 0;
        }
    }

    BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) : elements(elements) {
        stride = 0;
        uint32_t offset = 0;
        for(auto& e : this->elements) {
            stride += e.size;
            e.offset = offset;
            offset += e.size;
        }
    }

    void BufferLayout::addElement(const BufferElement& element) {
	    elements.push_back(element);
        elements.back().offset = stride;
        stride += element.size;
	}

    const std::vector<BufferElement>& BufferLayout::getElements() const {
	    return elements;
	}

    uint32_t BufferLayout::getStride() const {
        return stride;
    }

}  // namespace Cube
// clang-format on