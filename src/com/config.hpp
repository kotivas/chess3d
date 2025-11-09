#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "color.hpp"

struct Config {
	// --- SYSTEM ---
	glm::ivec2 sys_windowResolution;

	// -- FX ---
	bool fx_quantization;
	int fx_quantizationLevel;
	bool fx_vignette;
	float fx_vignetteIntensity;
	Color::rgb_t fx_vignetteColor;

	// --- RENDER ---
	float r_gamma;
	glm::ivec2 r_resolution;
	int r_shadowRes;
	float r_renderDistance;
	bool r_vsync;
	Color::rgb_t r_fillColor;

	// --- CONSOLE ---
	float con_fontScale;
	int con_maxVisibleLines;
	Color::rgba_t con_backgroundColor;
};

extern Config g_config;
