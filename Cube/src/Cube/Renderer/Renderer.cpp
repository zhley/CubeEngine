#include "Renderer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"

#include "Buffer.h"
#include "BufferLayout.h"
#include "VertexArray.h"
#include "Cube/Core/Log.h"

namespace Cube {

    bool Renderer::isGladInitialized = false;

    void Renderer::init() {
        if(!isGladInitialized){
            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                CB_CORE_ERROR("Failed to initialize GLAD!");
            } else {
                isGladInitialized = true;
            }
        }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }

    void Renderer::clearBuffer() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

    void Renderer::beginFrame() {}

    void Renderer::endFrame() {}

    void Renderer::setViewport(int width, int height) { glViewport(0, 0, width, height); }

    glm::vec2 Renderer::getViewport() {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        return {viewport[2], viewport[3]};
    }

    void Renderer::setClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }

    void Renderer::setVSync(bool val) { glfwSwapInterval(val); }

    //=========Renderer2D=================================

    thread_local Context* Renderer2D::currentContext = nullptr;

    void Renderer2D::init() {
        if(!isGladInitialized){
            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                CB_CORE_ERROR("Failed to initialize GLAD!");
            } else {
                isGladInitialized = true;
            }
        }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        // 启用混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        currentContext->defaultShader = new Shader(DEFAULT_2D_VERTEX_SHADER_SRC, DEFAULT_2D_FRAGMENT_SHADER_SRC);
        currentContext->shader = currentContext->defaultShader;

        currentContext->vbo = std::make_shared<VertexBuffer>();

        uint32_t* indices = new uint32_t[MAX_INDICES_PER_BATCH];
        uint32_t offset = 0;
        for(unsigned int i = 0; i < MAX_INDICES_PER_BATCH; i += 6) {
            indices[i] = 0 + offset;
            indices[i+1] = 1 + offset;
            indices[i+2] = 2 + offset;
            indices[i+3] = 0 + offset;
            indices[i+4] = 2 + offset;
            indices[i+5] = 3 + offset;
            offset += 4;
        }
        auto ibo = std::make_shared<IndexBuffer>(indices, MAX_INDICES_PER_BATCH * sizeof(uint32_t));
        delete[] indices;

        BufferLayout layout = {
            {ShaderDataType::Float4, "position"},
            {ShaderDataType::Float4, "color"},
            {ShaderDataType::Float2, "texCoord"}
        };
        currentContext->vbo->setLayout(layout);
        currentContext->vao = std::make_shared<VertexArray>();
        currentContext->vao->setIndexBuffer(ibo);
        currentContext->vao->addVertexBuffer(currentContext->vbo);

        uint8_t data[4]{0xFF, 0xFF, 0xFF, 0xFF};
        currentContext->whiteTex = new Texture2D(1, 1, data);
    }

    void Renderer2D::beginFrame(const glm::mat4& pvMatrix) {
        startNewBatch();

        currentContext->shader->bind();
        currentContext->shader->setMat4("u_ViewProjectMatrix", pvMatrix);
    }

    void Renderer2D::endFrame() {
        flushBatch();
    }

    void Renderer2D::shutdown() {
    }

    void Renderer2D::drawQuad(const glm::mat4& modelMatrix, const Color& tintColor, Texture2D* texture, const glm::vec4& texCoord) {
        if(texture == nullptr) {
            texture = currentContext->whiteTex;
        }
        if(texture != currentContext->currentTex) {
            flushBatch();
            startNewBatch();
            currentContext->currentTex = texture;
        }else if(currentContext->batchCnt >= MAX_QUADS_PER_BATCH) {
            flushBatch();
            startNewBatch();
        }
        currentContext->batchData.push_back({modelMatrix, tintColor, texCoord});
        currentContext->batchCnt++;
    }

    void Renderer2D::drawQuad(const glm::vec2& pos, const glm::vec2& size, Texture2D* texture, const Color& tintColor, float degree, const glm::vec4& texCoord) {
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(degree), {0.0f, 0.0f, 1.0f});
        modelMatrix = glm::scale(modelMatrix, glm::vec3(size, 1.0f));
        drawQuad(modelMatrix, tintColor, texture, texCoord);
    }

    void Renderer2D::drawQuad(const glm::vec2& pos, const glm::vec2& size, Texture2D* texture, const glm::vec4& texCoord, const Color& color, const glm::mat4& transform) {
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(size, 1.0f));
        modelMatrix = transform * modelMatrix;
        drawQuad(modelMatrix, color, texture, texCoord);
    }

    void Renderer2D::drawLine(const glm::vec2& p1, const glm::vec2& p2, const Color& color, float width) {
        glm::vec2 direction = p2 - p1;
        float length = glm::length(direction);
        
        float angle = std::atan2(direction.y, direction.x);
        
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(p1, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(length, width, 1.0f));
        
        drawQuad(modelMatrix, color, currentContext->whiteTex, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    }

    void Renderer2D::drawRect(const glm::mat4& modelMatrix, const glm::vec2& size, const Color& color, float width) {
        if(width < 0.01f) return;
        glm::vec2 vertices[4] = {
            {0.0f, 0.0f},
            {size.x, 0.0f},
            {size.x, size.y},
            {0.0f, size.y}
        };
        for(int i = 0; i < 4; ++i) {
            glm::vec4 p = modelMatrix * glm::vec4(vertices[i], 0.0f, 1.0f);
            vertices[i] = {p.x, p.y};
        }
        drawLine(vertices[0], vertices[1], color, width);
        drawLine(vertices[1], vertices[2], color, width);
        drawLine(vertices[2], vertices[3], color, width);
        drawLine(vertices[3], vertices[0], color, width);
    }

    void Renderer2D::startNewBatch() {
        currentContext->batchData.clear();
        currentContext->batchCnt = 0;
    }

    void Renderer2D::flushBatch() {
        std::vector<float> data;
        Texture2D* tex = currentContext->currentTex;
        for(const QuadData& qd : currentContext->batchData) {
            glm::vec2 size = {tex->getWidth() * (qd.textureCoord.z - qd.textureCoord.x), tex->getHeight() * (qd.textureCoord.w - qd.textureCoord.y)};
            glm::vec4 pos[4] = {
                {0.0f, 0.0f, 0.0f, 1.0f},
                {size.x, 0.0f, 0.0f, 1.0f},
                {size.x, size.y, 0.0f, 1.0f},
                {0.0f, size.y, 0.0f, 1.0f}
            };
            glm::vec2 texCoords[4] = {
                {qd.textureCoord.x, qd.textureCoord.y},
                {qd.textureCoord.z, qd.textureCoord.y},
                {qd.textureCoord.z, qd.textureCoord.w},
                {qd.textureCoord.x, qd.textureCoord.w}
            };
            for(int i = 0; i < 4; ++i) {
                glm::vec4 p = qd.modelMatrix * pos[i];
                data.push_back(p.x);
                data.push_back(p.y);
                data.push_back(p.z);
                data.push_back(p.w);
                data.push_back(qd.color.r);
                data.push_back(qd.color.g);
                data.push_back(qd.color.b);
                data.push_back(qd.color.a);
                data.push_back(texCoords[i].x);
                data.push_back(texCoords[i].y);
            }
        }
        currentContext->vbo->uploadData(data, GL_DYNAMIC_DRAW);
        if(currentContext->currentTex != nullptr){
            currentContext->currentTex->bind();
        }
        currentContext->shader->bind();
        currentContext->vao->bind();
        glDrawElements(GL_TRIANGLES, currentContext->batchData.size() * 6, GL_UNSIGNED_INT, nullptr);
    }

}  // namespace Cube
