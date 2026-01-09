#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Renderer {
	struct PostFXSettings {
		bool bloom;

		float chromaticOffset;

		bool quantization;
		float quantizationLevel;

		bool vignette;
		float vignetteIntensity;
		glm::vec3 vignetteColor;

		float saturation;
		float exposure;
		float gamma;
	};

	struct RenderSettings {
		PostFXSettings FX;

		float parallaxScale;
		float shadowDistance;

		float dirShadowRes;
		float pointShadowRes;
		float spotShadowRes;

		float renderDistance;
		glm::vec2 renderResolution;

		int blurPasses;

		bool vsync;
	};
}
