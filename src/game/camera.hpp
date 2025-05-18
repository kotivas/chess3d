#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {

	void mouseScrolled(double offset);
	void mouseMoved(double xpos, double ypos);
	void updatePosition();

	//inline glm::mat4 getViewMatrix() const { return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f)); }

	glm::vec3 position; // позиция камеры в пространстве
	glm::vec3 target; // точка в пространстве, вокруг которой вращается камера
	float radius; // Расстояние от камеры до точки target.
	float yaw, pitch;
	float lastx, lasty; // Последние координаты мыши
	float sens;
	float fov;

	bool locked;
};