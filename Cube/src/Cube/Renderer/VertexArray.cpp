#include "VertexArray.h"

namespace Cube {

    VertexArray::VertexArray() : vertexBufferIndex(0){
        glGenVertexArrays(1, &id);
    }

    VertexArray::~VertexArray() {
        glDeleteBuffers(1, &id);
    }

    void VertexArray::bind() {
        glBindVertexArray(id);
    }

    void VertexArray::unbind() { glBindVertexArray(0); }

    void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vb) {
        bind();
        vb->bind();
        const auto& layout = vb->getLayout();
        for(const auto& e : layout.getElements()) {
            glEnableVertexAttribArray(vertexBufferIndex);
            glVertexAttribPointer(vertexBufferIndex, shaderDataTypeCount(e.type), shaderDataTypeToOpenGlBaseType(e.type), e.normalized ? GL_TRUE : GL_FALSE, layout.getStride(), (void*)e.offset);
            ++vertexBufferIndex;
        }
        vbs.push_back(vb);
    }

    void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& ib) {
        bind();
        ib->bind();
        this->ib = ib;
    }

    const std::shared_ptr<IndexBuffer>& VertexArray::getIndexBuffer() const {
        return ib;
    }

    const std::vector<std::shared_ptr<VertexBuffer>>& VertexArray::getVertexBuffers() const {
        return vbs;
    }

}  // namespace Cube