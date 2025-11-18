#pragma once
#include <glm/vec3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Camera {
	struct Camera {
		Camera(float fov = 45.f, glm::vec3 position = {0, 0, 0},
		       float znear = 0.1, float zfar = 100)
			: position(position), yaw(0), pitch(0), fov(fov), nearPlane(znear), farPlane(zfar), forward(0, 0, 1),
			  up(0, 1, 0) {}

		glm::vec3 position;
		float yaw;
		float pitch;
		float fov;

		float nearPlane;
		float farPlane;

		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;
		glm::vec3 forward;
		glm::vec3 up;
		// right = glm::cross(forward, up)

		void calcProjMat(const float res_x, const float res_y) {
			projectionMatrix = glm::perspective(glm::radians(fov), res_x / res_y, nearPlane, farPlane);
		}
	};
}
