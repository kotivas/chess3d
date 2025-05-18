#pragma once
#include <glm/vec2.hpp>

struct Config {

	// --- RESOLUTIONS ---
	glm::ivec2 windowRes;
	glm::ivec2 renderRes;
	int shadowRes;

	// --- GRAPHICS ---
	float renderDistance;
	bool vsync;

};