#pragma once
#include "Renderer/RenderContext.hpp"
#include "Renderer/Texture.hpp"

namespace Renderer {
	struct ShadowData {
		ShadowData()
			: resolution(0), map(0), fbo(0), light_space(0), shader() {
		}

		uint32_t resolution;
		uint32_t map;
		uint32_t fbo;
		glm::mat4 light_space;
		AssetManager::ShaderHandle shader;
	};

	struct OmniShadowData {
		OmniShadowData()
			: resolution(0), cubemap(0), fbo(0), transforms(), shader() {
		}

		uint32_t resolution;
		uint32_t cubemap;
		uint32_t fbo;
		std::array<glm::mat4, 6> transforms;
		AssetManager::ShaderHandle shader;
	};

	class ShadowPass {
	public:
		void init(RenderSettings& settings);

		// TODO change shadow res in runtime
		void addPointShadow(float resolution);
		void pass(RenderContext& ctx);

		~ShadowPass();

	private:
		void drawRenderItem(const RenderItem& item, const Shader* shader) const;

		glm::mat4 calcDirLightSpace(const glm::vec3& light_dir, const Camera::CameraInfo& cam, float shadow_distance) const;
		glm::mat4 calcSpotLightSpace(const glm::vec3& light_pos, const glm::vec3& light_dir, float fov, float near,
		                             float far) const;
		std::array<glm::mat4, 6> calcPointLightSpace(const glm::vec3& light_pos, float near, float far,
		                                             float resolution) const;


		void drawDirectionalShadow(RenderContext& ctx);
		void drawSpotShadow(RenderContext& ctx);
		void drawPointShadow(RenderContext& ctx, uint8_t index);

		static void generateMap(uint32_t& map, uint32_t& fbo, uint32_t resolution);
		static void generateCubeMap(uint32_t& cube_map, uint32_t& fbo, uint32_t resolution);

		ShadowData _directional_shadow;
		ShadowData _spot_shadow;
		std::vector<OmniShadowData> _point_shadows;
	};
}
