#pragma once

#include <vector>
#include "../render/model.hpp"
#include "camera.hpp"
#include "light.hpp"

struct Scene {
	std::vector<Render::DrawableObjectPtr> objects;
	Camera camera;

	DirLight dirLight;
	PointLight pointLight; // later vector
	SpotLight spotLight;
};