#include "camera.hpp"

#include <iostream>

void Camera::mouseScrolled(double offset) {
	if (radius >= 1.f && radius <= 100.f)
		radius -= offset;
	if (radius <= 1.f)
		radius = 1.f;
	if (radius >= 100.f)
		radius = 100.f;
}

void Camera::updatePosition() {
	position = glm::vec3(
		target.x + radius * cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		target.y + radius * sin(glm::radians(pitch)),
		target.z + radius * sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	);
}

void Camera::mouseMoved(double xpos, double ypos) {

	float xoffset = xpos - lastx;
	float yoffset = ypos - lasty;  // Reversed since y-coordinates go from bottom to top

	lastx = xpos;
	lasty = ypos;

	if ( locked ) return; 

	xoffset *= sens;
	yoffset *= sens;

	yaw += xoffset;
	pitch += yoffset;

	// Ограничение pitch, чтобы камера не переворачивалась
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	//if (pitch <= 1) pitch = 1; // не проходить через пол
}
