#include "cameracontroller.hpp"

#include "com/config.hpp"
#include "console/console.hpp"
#include "core/logger.hpp"
#include "input/input.hpp"

namespace Camera {
	void OrbitCameraController::update(Camera& cam, double dt) {
		cam.calcProjMat(g_config.sys_windowResolution.x, g_config.sys_windowResolution.y);
		cam.position = glm::vec3(
			_target.x + _radius * cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch)),
			_target.y + _radius * sin(glm::radians(cam.pitch)),
			_target.z + _radius * sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch))
		);

		cam.viewMatrix = glm::lookAt(cam.position, _target, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void OrbitCameraController::handleControls(Camera& cam) {
		if (Input::IsRightMouseDown()) {
			double xoffset = Input::GetMouseX() - _lastx;
			double yoffset = Input::GetMouseY() - _lasty;

			xoffset *= g_config.sensitivity;
			yoffset *= g_config.sensitivity;

			cam.yaw += xoffset;
			cam.pitch += yoffset;

			if (cam.pitch > 89.0f) cam.pitch = 89.0f;
			if (cam.pitch < -89.0f) cam.pitch = -89.0f;
		}

		_lastx = static_cast<float>(Input::GetMouseX());
		_lasty = static_cast<float>(Input::GetMouseY());

		if (Input::GetScrollYOffset()) {
			if (_radius >= 0.f)
				_radius -= Input::GetScrollYOffset();
			if (_radius <= 0.f)
				_radius = 0.f;
		}
	}

	void FPSCameraController::update(Camera& cam, double dt) {
		cam.calcProjMat(g_config.sys_windowResolution.x, g_config.sys_windowResolution.y);
		cam.forward = {
			cos(glm::radians(cam.pitch)) * cos(glm::radians(cam.yaw)),
			sin(glm::radians(cam.pitch)),
			cos(glm::radians(cam.pitch)) * sin(glm::radians(cam.yaw))
		};
		cam.viewMatrix = glm::lookAt(cam.position, cam.position + cam.forward, {0, 1, 0});
	}

	void FPSCameraController::handleControls(Camera& cam) {
		if ( Console::IsVisible() ) {
			_lastx = static_cast<float>(Input::GetMouseX());
			_lasty = static_cast<float>(Input::GetMouseY());
			glfwSetInputMode(Renderer::g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			return;
		}

		glfwSetInputMode(Renderer::g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		double xoffset = Input::GetMouseX() - _lastx;
		double yoffset = Input::GetMouseY() - _lasty;

		xoffset *= g_config.sensitivity;
		yoffset *= g_config.sensitivity;

		cam.yaw += xoffset;
		cam.pitch += -yoffset; // inverse

		if (cam.pitch > 89.0f) cam.pitch = 89.0f;
		if (cam.pitch < -89.0f) cam.pitch = -89.0f;

		_lastx = static_cast<float>(Input::GetMouseX());
		_lasty = static_cast<float>(Input::GetMouseY());

		float speed_factor = 1;
		if (Input::IsKeyDown(Key::LeftShift)) speed_factor = 0.1;

		if (Input::IsKeyDown(Key::W)) cam.position += cam.forward * _speed * speed_factor;
		if (Input::IsKeyDown(Key::S))cam.position -= cam.forward * _speed * speed_factor;
		if (Input::IsKeyDown(Key::A))cam.position -= glm::cross(cam.forward, cam.up) * _speed * speed_factor;
		if (Input::IsKeyDown(Key::D))cam.position += glm::cross(cam.forward, cam.up) * _speed * speed_factor;
		if (Input::IsKeyDown(Key::Space))cam.position += cam.up * _speed * speed_factor;
		if (Input::IsKeyDown(Key::LeftControl))cam.position -= cam.up * _speed * speed_factor;


		if (Input::GetScrollYOffset()) {
			_speed += Input::GetScrollYOffset() > 0 ? 0.01 : -0.01;
			if (_speed < 0) _speed = 0;
		}
	}
}
