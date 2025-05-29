#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "shader.hpp"
#include <array>

namespace Render {
	struct DirShadowData {
		DirShadowData()
			: shader(nullptr), resolution(512), lightSpaceMatrix(0), shadowMap(0), shadowMapFBO(0) {
		}

		// should be set manually
		ShaderPtr shader;
		uint32_t resolution;
		// will generate automatically
		glm::mat4 lightSpaceMatrix;
		uint32_t shadowMap;
		uint32_t shadowMapFBO;

		void generate();
		void calculateLightSpaceMatrix(const glm::vec3& lightDir, float nearPlane, float farPlane);

		~DirShadowData();
	};

	struct SpotShadowData {
		SpotShadowData()
			: shader(nullptr), resolution(512), lightSpaceMatrix(0), shadowMap(0), shadowMapFBO(0) {
		}

		// should be set manually
		ShaderPtr shader;
		uint32_t resolution;
		// will generate automatically
		glm::mat4 lightSpaceMatrix;
		uint32_t shadowMap;
		uint32_t shadowMapFBO;

		void generate();
		void calculateLightSpaceMatrix(const glm::vec3& lightPos, const glm::vec3& lightDir, float fov,
		                               float nearPlane, float farPlane);

		~SpotShadowData();
	};

	struct OmniShadowData {
		OmniShadowData()
			: shader(nullptr), resolution(512), shadowCubemap(0), shadowCubemapFBO(0) {
		}

		ShaderPtr shader;
		uint32_t resolution;

		uint32_t shadowCubemap;
		uint32_t shadowCubemapFBO;

		std::array<glm::mat4, 6> transforms;

		void generate();
		void genTransformMatrixes(glm::vec3 lightPos, float nearPlane, float farPlane);

		~OmniShadowData();
	};
}
