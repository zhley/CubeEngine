#pragma once

#include "Color.h"
#include "Context.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexArray.h"

#include <glm/glm.hpp>
#include <memory>

namespace Cube {

    class Renderer {
    public:
        static void init();
        static void clearBuffer();
        static void beginFrame();
        static void endFrame();
        static void setViewport(int width, int height);
        static glm::vec2 getViewport();
        static void setClearColor(float r, float g, float b, float a);
        static void setVSync(bool val);
    protected:
        static bool isGladInitialized;
    };

    class Renderer2D : public Renderer {
    public:
        static void init();
        static void beginFrame(const glm::mat4& pvMatrix);
        static void endFrame();
        static void shutdown();
        static void drawQuad(const glm::mat4& modelMatrix, const Color& color, Texture2D* texture, const glm::vec4& texCoord);
        static void drawQuad(const glm::vec2& pos, const glm::vec2& size, Texture2D* texture, const Color& tintColor = WHITE, float degree = 0.0f, const glm::vec4& texCoord = {0.0f, 0.0f, 1.0f, 1.0f});
        static void drawQuad(const glm::vec2& pos, const glm::vec2& size, Texture2D* texture, const glm::vec4& texCoord, const Color& color, const glm::mat4& transform);
        static void drawLine(const glm::vec2& p1, const glm::vec2& p2, const Color& color, float width = 1.0f);
        static void drawRect(const glm::mat4& modelMatrix, const glm::vec2& size, const Color& color, float width = 1.0f);
        // TODO: custom shader

        static thread_local Context* currentContext;
    private:
        static void startNewBatch();
        static void flushBatch();

        static constexpr uint32_t MAX_QUADS_PER_BATCH = 1000;
        static constexpr uint32_t MAX_VERTICES_PER_BATCH = MAX_QUADS_PER_BATCH * 4;
        static constexpr uint32_t MAX_INDICES_PER_BATCH = MAX_QUADS_PER_BATCH * 6;
    };


    // 默认着色器
    // clang-format off
    const std::string DEFAULT_2D_VERTEX_SHADER_SRC = R"(
        #version 330 core

        layout (location = 0) in vec4 aPos;
        layout (location = 1) in vec4 aColor;
        layout (location = 2) in vec2 aTexCoord;

        uniform mat4 u_ViewProjectMatrix;

        out vec2 textureCoord;
        out vec4 color;

        void main(){
            textureCoord = aTexCoord;
            color = aColor;
            gl_Position = u_ViewProjectMatrix * aPos;
        }

    )";
    const std::string DEFAULT_2D_FRAGMENT_SHADER_SRC = R"(
        #version 330 core

        in vec2 textureCoord;
        in vec4 color;

        uniform sampler2D u_Texture;

        out vec4 o_color;

        void main(){
            o_color = texture(u_Texture, textureCoord) * color;
        }
    )";
    // clang-format on
}