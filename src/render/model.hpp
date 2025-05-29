#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>

#include "shader.hpp"

namespace Render {

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
    glm::vec3 bitangent;
};
struct Transform {
	glm::vec3 position{0};
	glm::vec3 rotation{0};
	glm::vec3 scale{1};

	[[nodiscard]] glm::mat4 getMatrix() const {
		glm::mat4 transform{ 1 };
		transform = glm::translate(transform, position);

		transform = glm::rotate(transform, glm::radians(rotation.x), { 1, 0, 0 });
		transform = glm::rotate(transform, glm::radians(rotation.y), { 0, 1, 0 });
		transform = glm::rotate(transform, glm::radians(rotation.z), { 0, 0, 1 });

		transform = glm::scale(transform, scale);
		return (transform == glm::mat4(0)) ? glm::mat4(1) : transform;
	}
};

struct Material {
	std::string name;

	uint32_t diffuse[3];
	uint32_t specular[3];
	uint32_t normal[3];

	float shininess{0};
	ShaderPtr shader;
	glm::vec3 solidColor;
	bool useSolidColor{false};
	//GLuint ior;       // index of refraction
	//GLuint dissolve;  // 1 == opaque; 0 == fully transparent
	void apply() const;
};
using MaterialPtr = std::shared_ptr<Material>;

struct Mesh {
	Mesh() : name("undefined"),
		VBO(0), VAO(0), EBO(0), drawable(true) {}

	bool drawable;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	Transform transform;
	std::string name;
	uint32_t VBO, VAO, EBO;
	MaterialPtr material;

	void draw(Transform& model);
	void draw(const Transform& model, const ShaderPtr& shader);

	void setup();
};
using MeshPtr = std::shared_ptr<Mesh>;

struct Model {
	Model() : name("undefined"), castShadow(true) {}

	MeshPtr findMeshByName(const std::string& name) const;

	void draw();
	void draw(const ShaderPtr& shader);

	//std::vector<Material> materials;
	std::vector<MeshPtr> meshes; // unordered map for quicker search by name
	std::string name;
	bool castShadow;
	Transform transform;

	~Model();
};

using ModelPtr = std::shared_ptr<Model>;

}