#pragma once

#include <vector>
#include "../Renderer/Model.hpp"
#include "Camera.hpp"
#include "../Renderer/Light.hpp"
#include "Renderer/Sky.hpp"

struct Scene {
	std::vector<Renderer::DrawableObjectPtr> objects;
	Camera::Camera camera;

	Renderer::SkyPtr sky;
	Renderer::DirLight dirLight;
	Renderer::PointLight pointLight; // later vector
	Renderer::SpotLight spotLight;
};
