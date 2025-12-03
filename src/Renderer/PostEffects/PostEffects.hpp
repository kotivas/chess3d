#pragma once
#include <algorithm>
#include "Renderer/Renderer.hpp"

namespace Renderer::PostEffects {
	constexpr float EXP_WHITE = 1.f;
	constexpr float EXP_MIN = 0.1f;
	constexpr float EXP_MAX = 2.0f;
	constexpr float EXP_DARK_SPEED_FACTOR = 0.25f; // dark adaptation speed factor (based on fx_autoExposureSpeed)

	// logarithmic interpolation adjusted for reasonable exposure limits
	inline float GetLerpExposure(float exposure, float avgLuminance, float exposureSpeed) {
		const float targetExposure = EXP_WHITE / std::max(avgLuminance, 0.003f);
		const float speed = targetExposure < exposure ? exposureSpeed : exposureSpeed * EXP_DARK_SPEED_FACTOR;
		const float logExposure = std::lerp(std::log(exposure), std::log(targetExposure), speed);
		return std::clamp(std::exp(logExposure), EXP_MIN, EXP_MAX);
	}
}
