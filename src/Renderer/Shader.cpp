#include "Shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Renderer {
    Shader::Shader() : _handle(0) {}

    Shader::Shader(const uint16_t id) : _handle(id) {}

    void Shader::use() const {
        glUseProgram(_handle);
    }

    uint16_t &Shader::getHandle() {
        return _handle;
    }

    Shader::~Shader() {
        glDeleteProgram(_handle);
    }

    // ------------------------------------------------------------------------
    void Shader::setUniform1i(const std::string &name, int v0) const {
        glUniform1i(glGetUniformLocation(_handle, name.c_str()), v0);
    }

    void Shader::setUniform2i(const std::string &name, int v0, int v1) const {
        glUniform2i(glGetUniformLocation(_handle, name.c_str()), v0, v1);
    }

    void Shader::setUniform3i(const std::string &name, int v0, int v1, int v2) const {
        glUniform3i(glGetUniformLocation(_handle, name.c_str()), v0, v1, v2);
    }

    void Shader::setUniform4i(const std::string &name, int v0, int v1, int v2, int v3) const {
        glUniform4i(glGetUniformLocation(_handle, name.c_str()), v0, v1, v2, v3);
    }

    // ------------------------------------------------------------------------
    void Shader::setUniform1f(const std::string &name, float v0) const {
        glUniform1f(glGetUniformLocation(_handle, name.c_str()), v0);
    }

    void Shader::setUniform2f(const std::string &name, float v0, float v1) const {
        glUniform2f(glGetUniformLocation(_handle, name.c_str()), v0, v1);
    }

    void Shader::setUniform2f(const std::string &name, glm::vec2 v) const {
        glUniform2f(glGetUniformLocation(_handle, name.c_str()), v.x, v.y);
    }

    void Shader::setUniform3f(const std::string &name, float v0, float v1, float v2) const {
        glUniform3f(glGetUniformLocation(_handle, name.c_str()), v0, v1, v2);
    }

    void Shader::setUniform3f(const std::string &name, glm::vec3 v) const {
        glUniform3f(glGetUniformLocation(_handle, name.c_str()), v.x, v.y, v.z);
    }

    void Shader::setUniform4f(const std::string &name, float v0, float v1, float v2, float v3) const {
        glUniform4f(glGetUniformLocation(_handle, name.c_str()), v0, v1, v2, 3);
    }

    void Shader::setUniform4f(const std::string &name, glm::vec4 v) const {
        glUniform4f(glGetUniformLocation(_handle, name.c_str()), v.x, v.y, v.z, v.w);
    }

    void Shader::setUniformMat4fv(const std::string &name, bool transpose, glm::mat4 value) const {
        glUniformMatrix4fv(glGetUniformLocation(_handle, name.c_str()), 1, transpose, glm::value_ptr(value));
    }
} // namespace Renderer
