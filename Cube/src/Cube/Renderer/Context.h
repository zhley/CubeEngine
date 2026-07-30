#pragma once

#include <vector>

#include "glm/glm.hpp"


#include "Buffer.h"
#include "Color.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"

namespace Cube {

    struct QuadData {
		glm::mat4 modelMatrix;
		Color color;
		glm::vec4 textureCoord;
	};

	// each Window owns a Context object
	class Context {
	public:
		friend class Renderer2D;

		Context();
		~Context();

	private:
		std::shared_ptr<VertexArray> vao;
		std::shared_ptr<VertexBuffer> vbo; // TODO: use raw ptr
		std::vector<QuadData> batchData;
		unsigned int batchCnt = 0;
		Shader* shader = nullptr;
		Shader* defaultShader = nullptr;
		Texture2D* currentTex = nullptr;
		Texture2D* whiteTex = nullptr;
	};

}