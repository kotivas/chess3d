#pragma once
#include <glad/glad.h>

#include "Game/Scene.hpp"

namespace Renderer {
	struct ShadowInfo {
		uint32_t shadow_map;
		glm::mat4 light_space;
	};

	struct OmniShadowInfo {
		uint32_t shadow_map;
		std::array<glm::mat4, 6> transforms;
	};

	struct RenderItem {
		MeshPtr mesh;
		MaterialPtr material;
		glm::mat4 world_transform;
	};

	struct RenderContext {
		float near;
		float far;
		uint16_t render_width;
		uint16_t render_height;

		std::vector<RenderItem> render_items;
		Scene& scene;
		uint32_t target_fbo;

		ShadowInfo dir_shadow;
		ShadowInfo spot_shadow;
		std::vector<OmniShadowInfo> point_shadows;
	};
}
