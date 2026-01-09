#pragma once
#include "Renderer/RenderContext.hpp"

namespace Renderer {
	class LightningPass {
	public:
		struct ShaderUniforms {
			float nearPlane;
			float farPlane;
			glm::vec3 viewPos;

			float parallaxScale;

		};

		LightningPass()
			: _ubo_matrices(0), _ubo_lights(0) {
		}

		void init();
		void pass(const RenderContext& ctx) const;

	private:
		void drawRenderItem(const RenderItem& item, const ShaderUniforms& unifs) const;
		void createUboMatrices();
		void createUboLights();

		void updateUboMatrices(const glm::mat4& projection, const glm::mat4& view,
		                       const glm::mat4& dir_light_space_matrix, const glm::mat4& spot_light_space_matrix) const;
		void updateUboLights(const DirLight& dir_light, const PointLight& point_light,
		                     const SpotLight& spot_light) const;

		uint32_t _ubo_matrices;
		uint32_t _ubo_lights;
	};
}
