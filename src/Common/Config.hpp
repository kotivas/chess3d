#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Color.hpp"

struct Config {
	// -- NO PREFIX ---
	float sensitivity;

	// --- SYSTEM ---
	glm::u16vec2 sys_windowResolution;

	// -- FX ---
	bool fx_bloom;
	float fx_chromaticOffset;
	bool fx_quantization;
	int fx_quantizationLevel;
	bool fx_vignette;
	float fx_vignetteIntensity;
	float fx_saturation;
	float fx_exposure;
	bool fx_autoExposure;
	float fx_autoExposureSpeed;

	Color::rgb_t fx_vignetteColor;

	// --- RENDER ---
	int r_blurPasses;
	float r_gamma;
	glm::u16vec2 r_resolution;
	int r_shadowRes;
	float r_renderDistance;
	bool r_vsync;

	// --- CONSOLE ---
	float con_fontScale;
	int con_maxVisibleLines;
	Color::rgba_t con_backgroundColor;
};

extern Config g_config;
