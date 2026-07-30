#pragma once
#include <string>

#include "glad/glad.h"
#include "glm/glm.hpp"

namespace Cube {

    class Shader {
    public:
        Shader(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc);
        Shader(const std::string& shaderSrc);
        ~Shader();

        void bind();
        static void unbind();

        uint32_t getId() const;

        // set Uniform
        void setFloat(const std::string& name, float val);
        void setInt(const std::string& name, int val);
        void setVec2(const std::string& name, glm::vec2 val);
        void setVec3(const std::string& name, glm::vec3 val);
        void setVec4(const std::string& name, glm::vec4 val);
        void setMat3(const std::string& name, glm::mat3 val);
        void setMat4(const std::string& name, glm::mat4 val);

    private:
        uint32_t compileShader(GLenum type, const std::string src);
        void init(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc);

        uint32_t id;
    };
}  // namespace Cube