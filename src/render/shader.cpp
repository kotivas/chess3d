#include "shader.hpp"
#include "../com/util.hpp"
#include <iostream>

namespace Render {
	Shader::Shader()
		: _id(0) {}

	void Shader::use() {
		glUseProgram(_id);
	}

	GLuint Shader::getID() const {
		return _id;
	}

	// ------------------------------------------------------------------------
	void Shader::setUniform1i(const std::string& name, int v0) const {
		glUniform1i(glGetUniformLocation(_id, name.c_str()), v0);
	}

	void Shader::setUniform2i(const std::string& name, int v0, int v1) const {
		glUniform2i(glGetUniformLocation(_id, name.c_str()), v0, v1);
	}

	void Shader::setUniform3i(const std::string& name, int v0, int v1, int v2) const {
		glUniform3i(glGetUniformLocation(_id, name.c_str()), v0, v1, v2);
	}

	void Shader::setUniform4i(const std::string& name, int v0, int v1, int v2, int v3) const {
		glUniform4i(glGetUniformLocation(_id, name.c_str()), v0, v1, v2, v3);
	}

	// ------------------------------------------------------------------------
	void Shader::setUniform1f(const std::string& name, float v0) const {
		glUniform1f(glGetUniformLocation(_id, name.c_str()), v0);
	}

	void Shader::setUniform2f(const std::string& name, float v0, float v1) const {
		glUniform2f(glGetUniformLocation(_id, name.c_str()), v0, v1);
	}

	void Shader::setUniform2f(const std::string& name, glm::vec2 v) const {
		glUniform2f(glGetUniformLocation(_id, name.c_str()), v.x, v.y);
	}

	void Shader::setUniform3f(const std::string& name, float v0, float v1, float v2) const {
		glUniform3f(glGetUniformLocation(_id, name.c_str()), v0, v1, v2);
	}

	void Shader::setUniform3f(const std::string& name, glm::vec3 v) const {
		glUniform3f(glGetUniformLocation(_id, name.c_str()), v.x, v.y, v.z);
	}

	void Shader::setUniform3f(const std::string& name, const Color::rgb_t& c) {
		glUniform3f(glGetUniformLocation(_id, name.c_str()), c.r, c.g, c.b);
	}

	void Shader::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const {
		glUniform4f(glGetUniformLocation(_id, name.c_str()), v0, v1, v2, 3);
	}

	void Shader::setUniform4f(const std::string& name, glm::vec4 v) const {
		glUniform4f(glGetUniformLocation(_id, name.c_str()), v.x, v.y, v.z, v.w);
	}

	void Shader::setUniform4f(const std::string& name, const Color::rgba_t& c) const {
		glUniform4f(glGetUniformLocation(_id, name.c_str()), c.r, c.g, c.b, c.a);
	}

	// ------------------------------------------------------------------------
	void Shader::setUniformMat4fv(const std::string& name, GLboolean transpose, glm::mat4& value) const {
		glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, transpose, glm::value_ptr(value));
	}
}
