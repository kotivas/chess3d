#pragma once
#include "Renderer/RenderContext.hpp"

namespace Renderer {
	class LightningPass {
	public:
		LightningPass()
			: _ubo_matrices(0), _ubo_lights(0), _ubo_data(0) {
		}

		void init();
		void pass(const RenderContext& ctx) const;
		static void drawRenderItem(const RenderItem& item);

	private:
		void createUboMatrices();
		void createUboLights();
		void createUboData();

		void updateUboMatrices(const glm::mat4& projection, const glm::mat4& view,
		                       const glm::mat4& dir_light_space_matrix, const glm::mat4& spot_light_space_matrix) const;
		void updateUboLights(const DirLight& dir_light, const PointLight& point_light,
		                     const SpotLight& spot_light) const;
		void updateUboData(const glm::vec3& viewPos, float far_plane) const;

		uint32_t _ubo_matrices;
		uint32_t _ubo_lights;
		uint32_t _ubo_data;
	};
}
