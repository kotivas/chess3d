#pragma once
#include <algorithm>
#include "Renderer/Renderer.hpp"

namespace Renderer::PostEffects {
	constexpr float EXPOSURE_WHITE = 1.f;

	// logarithmic interpolation adjusted for reasonable exposure limits
	inline float GetLerpExposure(float exposure, float avgLuminance, float exposureSpeed) {
		const float targetExposure = EXPOSURE_WHITE / std::max(avgLuminance, 0.003f);
		float logExposure = std::lerp(std::log(exposure), std::log(targetExposure), exposureSpeed);
		return std::clamp(std::exp(logExposure), 0.1f, 2.0f);
	}
}
