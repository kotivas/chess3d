#pragma once

#include <vector>
#include "../Renderer/Model.hpp"
#include "Camera.hpp"
#include "../Renderer/Light.hpp"
#include "Renderer/Sky.hpp"

struct Scene {
	std::vector<std::shared_ptr<Renderer::Drawable>> objects;
	Camera::CameraInfo camera;
	float time; // in seconds 24hours

	Renderer::SkyPtr sky;
	Renderer::DirLight dir_light;
	Renderer::PointLight point_light; // later vector
	Renderer::SpotLight spot_light;
};
