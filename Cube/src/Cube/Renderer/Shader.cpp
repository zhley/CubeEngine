#include "Shader.h"

#include "Cube/Core/Log.h"

namespace Cube {

    Shader::Shader(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) {
        init(vertexShaderSrc, fragmentShaderSrc);
    }

    Shader::Shader(const std::string& shaderSrc) {
        // extract vertex and fragment shader source from the single string
        // symbols: "#VertexShader"-> vertex shader, "#FragmentShader"-> fragment shader
        std::string vertexSymbol = "#VertexShader";
        std::string fragmentSymbol = "#FragmentShader";
        size_t vertexBegin = shaderSrc.find(vertexSymbol);
        size_t fragmentBegin = shaderSrc.find(fragmentSymbol);
        if(vertexBegin == std::string::npos || fragmentBegin == std::string::npos) {
            CB_CORE_ERROR("Shader symbols not found");
            CB_ASSERT("Shader source format error!");
            return;
        }
        std::string vertexShaderSrc = shaderSrc.substr(vertexBegin + vertexSymbol.length(), fragmentBegin - vertexBegin - vertexSymbol.length());
        std::string fragmentShaderSrc = shaderSrc.substr(fragmentBegin + fragmentSymbol.length(), shaderSrc.length() - fragmentBegin - fragmentSymbol.length());
        init(vertexShaderSrc, fragmentShaderSrc);
    }

    Shader::~Shader() {
        glDeleteProgram(id);
    }

    void Shader::bind() {
        glUseProgram(id);
    }

    void Shader::unbind() { glUseProgram(0); }

    uint32_t Shader::getId() const {return id;}

    uint32_t Shader::compileShader(GLenum type, const std::string src) {
        uint32_t shader = glCreateShader(type);
        const char* s = src.c_str();
        glShaderSource(shader, 1, &s, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::string shaderType = (type == GL_VERTEX_SHADER ? "vertex shader" : "fragment shader");
            CB_CORE_ERROR(shaderType + " compile error : " + std::string(log));
            CB_ASSERT("Shader compile error!");
        }
        return shader;
    }

    void Shader::init(const std::string& vertexShaderSrc, const std::string& fragmentShaderSrc) {
        uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
        uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        glLinkProgram(id);

        GLint success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if(!success) {
            char log[512];
            glGetProgramInfoLog(id, 512, nullptr, log);
            CB_CORE_ERROR("shader link error: " + std::string(log));
            CB_ASSERT("Shader link error");
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Shader::setFloat(const std::string& name, float val) {
        glUniform1f(glGetUniformLocation(id, name.c_str()), val);
    }

    void Shader::setInt(const std::string& name, int val) {
        glUniform1i(glGetUniformLocation(id, name.c_str()), val);
    }

    void Shader::setVec2(const std::string& name, glm::vec2 val) {
        glUniform2f(glGetUniformLocation(id, name.c_str()), val.x, val.y);
    }

    void Shader::setVec3(const std::string& name, glm::vec3 val) {
        glUniform3f(glGetUniformLocation(id, name.c_str()), val.x, val.y, val.z);
    }

    void Shader::setVec4(const std::string& name, glm::vec4 val) {
        glUniform4f(glGetUniformLocation(id, name.c_str()), val.x, val.y, val.z, val.w);
    }

    void Shader::setMat3(const std::string& name, glm::mat3 val) {
        glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &val[0][0]);
    }

    void Shader::setMat4(const std::string& name, glm::mat4 val) {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &val[0][0]);
    }

}  // namespace Cube