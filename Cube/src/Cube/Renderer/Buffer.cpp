#include "Buffer.h"

#include "glad/glad.h"

namespace Cube {

	VertexBuffer::VertexBuffer(const std::vector<float>& vertices, uint32_t usage) {
        glGenBuffers(1, &id);
        bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), usage);
    }

    VertexBuffer::VertexBuffer() {
	    glGenBuffers(1, &id);
	}

    VertexBuffer::~VertexBuffer() {
	    glDeleteBuffers(1, &id);
	}

	void VertexBuffer::bind() {
	    glBindBuffer(GL_ARRAY_BUFFER, id);
	}

	void VertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    void VertexBuffer::uploadData(const std::vector<float>& vertices, uint32_t usage) {
        bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), usage);
    }

    void VertexBuffer::uploadData(float* data, uint32_t size, uint32_t usage) {
	    bind();
		glBufferData(GL_ARRAY_BUFFER, size, data, usage);
	}

    void VertexBuffer::setLayout(const BufferLayout& layout) {
		this->layout = layout;
	}

	const BufferLayout& VertexBuffer::getLayout() const {
	    return layout;
	}


	IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices, uint32_t usage) : count(indices.size()) {
        glGenBuffers(1, &id);
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), usage);
    }

    IndexBuffer::IndexBuffer(const uint32_t* data, uint32_t size, uint32_t usage) {
	    glGenBuffers(1, &id);
		bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
	}

    IndexBuffer::~IndexBuffer() {
		glDeleteBuffers(1, &id);
	}

	void IndexBuffer::bind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}

	void IndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

    uint32_t IndexBuffer::getCount() const {
	    return count;
	}

}  // namespace Cube