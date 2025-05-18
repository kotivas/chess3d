#pragma once

#include <vector>
#include "../render/model.hpp"
#include "camera.hpp"
#include "light.hpp"

struct Scene {
	std::vector<Render::ModelPtr> models; 
	Camera camera; 

	DirLight dirLight;
	PointLight pointLight; // later vector
	SpotLight spotLight;
};