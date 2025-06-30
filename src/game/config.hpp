#pragma once
#include <glm/vec2.hpp>

struct Config {
	// --- RESOLUTIONS ---
	glm::ivec2 windowRes;
	glm::ivec2 renderRes;
	uint16_t shadowRes;

	// --- GRAPHICS ---
	float renderDistance;
	bool vsync;
	glm::vec3 fillColor;
};
