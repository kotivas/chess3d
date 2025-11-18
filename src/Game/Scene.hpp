#pragma once

#include <vector>
#include "../Renderer/Model.hpp"
#include "Camera.hpp"
#include "../Renderer/Light.hpp"

struct Scene {
	std::vector<Renderer::DrawableObjectPtr> objects;
	Camera::Camera camera;

	Renderer::DirLight dirLight;
	Renderer::PointLight pointLight; // later vector
	Renderer::SpotLight spotLight;
};