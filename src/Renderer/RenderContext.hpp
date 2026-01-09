#pragma once
#include <glad/glad.h>

#include "RenderSettings.hpp"
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

	struct LightRenderInfo {
		DirLight directional;
		PointLight point;
		SpotLight spot;
	};

	struct RenderContext {
		const RenderSettings& settings;

		std::vector<RenderItem> render_items;
		uint32_t target_framebuffer;

		LightRenderInfo light_info;
		Camera::CameraInfo camera;

		ShadowInfo dir_shadow;
		ShadowInfo spot_shadow;
		std::vector<OmniShadowInfo> point_shadows;
	};
}
