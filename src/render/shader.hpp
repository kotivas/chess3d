#pragma once

#include <string>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "../game/camera.hpp"
#include "../game/light.hpp"

#include <memory>

namespace Render {

class Shader {
public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
	Shader();

	void use();

	// TODO isLoaded

	GLuint getID() const;

	void loadShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");

	void setUniform1i(const std::string& name, int v0) const;
	void setUniform2i(const std::string& name, int v0, int v1) const;
	void setUniform3i(const std::string& name, int v0, int v1, int v2) const;
	void setUniform4i(const std::string& name, int v0, int v1, int v2, int v3) const;

    void setUniform1f(const std::string& name, float v0) const;
	void setUniform2f(const std::string& name, float v0, float v1) const;
	void setUniform2f(const std::string& name, glm::vec2 v) const;
	void setUniform3f(const std::string& name, float v0, float v1, float v2) const;
	void setUniform3f(const std::string& name, glm::vec3 v) const;
	void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
	void setUniform4f(const std::string& name, glm::vec4 v) const;

	void setUniformMat4fv(const std::string& name, GLboolean transpose, glm::mat4& value) const;

private:
	GLuint createShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
	GLuint compileShader(GLenum shaderType, const std::string src);

	GLuint _id;
};
using ShaderPtr = std::shared_ptr<Shader>;
}