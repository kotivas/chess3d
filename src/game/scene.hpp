#pragma once

#include <vector>
#include "../render/drawable.hpp"
#include "camera.hpp"

// point light
struct Light {
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;

	float constant;
    float linear;
    float quadratic;
};

struct Scene {
	std::vector<Render::DrawableObjectPtr> objects;
	Camera camera;
	Light light;
};