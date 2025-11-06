#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Config {
	// --- SYSTEM ---
	glm::ivec2 sys_windowResolution;

	// --- RENDER ---
	glm::ivec2 r_resolution;
	uint16_t r_shadowRes;
	float r_renderDistance;
	bool r_vsync;
	glm::vec3 r_fillColor;

	// --- CONSOLE ---
	int con_fontScale;
	int con_maxVisibleLines;
	glm::vec4 con_backgroundColor;
};

extern Config g_config;
