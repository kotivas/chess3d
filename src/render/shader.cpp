#include "shader.hpp"
#include "../util.hpp"
#include <iostream>

namespace Render {

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
	_id = createShader(vertexPath, fragmentPath, geometryPath);
}

Shader::Shader()
	: _id(0) {}

void Shader::use() {
	glUseProgram(_id);
}

GLuint Shader::getID() const {
	return _id;
}

void Shader::loadShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
	_id = createShader(vertexPath, fragmentPath, geometryPath);
}

GLuint Shader::compileShader(GLenum shaderType, const std::string src) {
	GLuint id = glCreateShader(shaderType);
	const char* raw = src.c_str();

	glShaderSource(id, 1, &raw, nullptr);
	glCompileShader(id);

	GLint success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(id, 512, nullptr, infoLog);
		std::cout << OUT_ERROR << "Shader compilation failed:" << infoLog << std::endl;
		return 0;
	}

	return id;
}

GLuint Shader::createShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {

	GLuint vs = compileShader(GL_VERTEX_SHADER, Util::readFromFile(vertexPath));
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, Util::readFromFile(fragmentPath));
	GLuint gs = 0;
	if (!geometryPath.empty()) gs = compileShader(GL_GEOMETRY_SHADER, Util::readFromFile(geometryPath));

	GLuint program = glCreateProgram();

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	if (gs) glAttachShader(program, gs);

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	if (gs) glDeleteShader(gs);
	
	std::cout << OUT_DEBUG << "Shader loaded: " << vertexPath << " " << fragmentPath << " " << geometryPath << std::endl;

	return program;
}

// ------------------------------------------------------------------------
void Shader::setUniform1i(const std::string& name, int v0) const { glUniform1i(glGetUniformLocation(_id, name.c_str()), v0); }
void Shader::setUniform2i(const std::string& name, int v0, int v1) const { glUniform2i(glGetUniformLocation(_id, name.c_str()), v0, v1); }
void Shader::setUniform3i(const std::string& name, int v0, int v1, int v2) const { glUniform3i(glGetUniformLocation(_id, name.c_str()), v0, v1, v2); }
void Shader::setUniform4i(const std::string& name, int v0, int v1, int v2, int v3) const { glUniform4i(glGetUniformLocation(_id, name.c_str()), v0, v1, v2, v3); }
// ------------------------------------------------------------------------
void Shader::setUniform1f(const std::string& name, float v0) const { glUniform1f(glGetUniformLocation(_id, name.c_str()), v0); }
void Shader::setUniform2f(const std::string& name, float v0, float v1) const { glUniform2f(glGetUniformLocation(_id, name.c_str()), v0, v1); }
void Shader::setUniform2f(const std::string& name, glm::vec2 v) const { glUniform2f(glGetUniformLocation(_id, name.c_str()), v.x, v.y); }
void Shader::setUniform3f(const std::string& name, float v0, float v1, float v2) const { glUniform3f(glGetUniformLocation(_id, name.c_str()), v0, v1, v2); }
void Shader::setUniform3f(const std::string& name, glm::vec3 v) const { glUniform3f(glGetUniformLocation(_id, name.c_str()), v.x, v.y, v.z); }
void Shader::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const { glUniform4f(glGetUniformLocation(_id, name.c_str()), v0, v1, v2, 3); }
void Shader::setUniform4f(const std::string& name, glm::vec4 v) const { glUniform4f(glGetUniformLocation(_id, name.c_str()), v.x, v.y, v.z, v.w); }

// ------------------------------------------------------------------------
void Shader::setUniformMat4fv(const std::string& name, GLsizei count, GLboolean transpose, glm::mat4& value) const {
	glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), count, transpose, glm::value_ptr(value));
}


}