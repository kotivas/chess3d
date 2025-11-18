#include "ResourceMgr.hpp"
#include "./Renderer/Shader.hpp"
#include "Core/Logger.hpp"
#include "Common/Utils.hpp"

namespace ResourceMgr {
	GLuint compileShader(GLenum shaderType, const std::string& src) {
		GLuint id = glCreateShader(shaderType);
		const char* raw = src.c_str();

		glShaderSource(id, 1, &raw, nullptr);
		glCompileShader(id);

		GLint success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(id, 512, nullptr, infoLog);
			Log::Error("Shader compilation failed: " + std::string(infoLog));
			return 0;
		}

		return id;
	}

	void LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,
	                const std::string& geometryPath) {
		Renderer::ShaderPtr shader = std::make_shared<Renderer::Shader>();

		GLuint vs = compileShader(GL_VERTEX_SHADER, Utils::readFromFile(vertexPath));
		GLuint fs = compileShader(GL_FRAGMENT_SHADER, Utils::readFromFile(fragmentPath));
		GLuint gs = 0;
		if (!geometryPath.empty()) gs = compileShader(GL_GEOMETRY_SHADER, Utils::readFromFile(geometryPath));

		shader->_id = glCreateProgram();

		glAttachShader(shader->_id, vs);
		glAttachShader(shader->_id, fs);
		if (gs)
			glAttachShader(shader->_id, gs);

		glLinkProgram(shader->_id);
		glValidateProgram(shader->_id);

		glDeleteShader(vs);
		glDeleteShader(fs);
		if (gs)
			glDeleteShader(gs);

		Log::Debug("Shader loaded: " + vertexPath + " " + fragmentPath + " " + geometryPath);

		g_shaders.insert({name, shader});
	}
}
