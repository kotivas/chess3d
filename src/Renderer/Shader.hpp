#pragma once

#include <string>
#include <glm/glm.hpp>
#include <memory>
#include "Common/Color.hpp"

namespace Renderer {
	class Shader {
	public:
		Shader();
		Shader(uint16_t id);

		void use() const;

		uint16_t& getId();

		void setUniform1i(const std::string& name, int v0) const;
		void setUniform2i(const std::string& name, int v0, int v1) const;
		void setUniform3i(const std::string& name, int v0, int v1, int v2) const;
		void setUniform4i(const std::string& name, int v0, int v1, int v2, int v3) const;
		void setUniform1f(const std::string& name, float v0) const;
		void setUniform2f(const std::string& name, float v0, float v1) const;
		void setUniform2f(const std::string& name, glm::vec2 v) const;
		void setUniform3f(const std::string& name, float v0, float v1, float v2) const;
		void setUniform3f(const std::string& name, glm::vec3 v) const;
		void setUniform3f(const std::string& name, Color::rgb_t c) const;
		void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) const;
		void setUniform4f(const std::string& name, glm::vec4 v) const;
		void setUniform4f(const std::string& name, Color::rgba_t c) const;
		void setUniformMat4fv(const std::string& name, bool transpose, glm::mat4 value) const;

		~Shader();

	private:
		uint16_t _id;
	};
}
