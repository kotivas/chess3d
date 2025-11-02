#pragma once

struct DirLight {
	int enable;

	alignas(16) glm::vec3 direction;

	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 specular;
};

struct PointLight {
	int enable;

	alignas(16) glm::vec3 position;

	float constant;
	float linear;
	float quadratic;

	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 specular;
};

struct SpotLight {
	int enable;

	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 direction;

	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 specular;
};
