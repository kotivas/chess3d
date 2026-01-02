#pragma once
#include "Model.hpp"
#include "Shader.hpp"
#include "AssetManager/Handle.hpp"

namespace Renderer {
	glm::vec3 HosekWilkie(float cos_theta, float gamma, float cos_gamma,
	                      const std::array<glm::vec3, 10>& params);

	struct Sky {
		void setup();

		std::vector<Vertex> vertices;
		uint32_t vao, vbo;
		AssetManager::ShaderHandle shader;

		glm::vec3 sun_direction;
		glm::vec3 sun_color;
		float atm_turbidity;
		std::array<glm::vec3, 10> atm_params;

		void calculateSun(const glm::vec2& sun_pos);

		~Sky();
	};

	using SkyPtr = std::shared_ptr<Sky>;
}
