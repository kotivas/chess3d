#pragma once

#include <vector>
#include "../render/model.hpp"
#include "camera.hpp"
#include "../render/light.hpp"

struct Scene {
	std::vector<Render::DrawableObjectPtr> objects;
	Camera camera;

	DirLight dirLight;
	PointLight pointLight; // later vector
	SpotLight spotLight;
};