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
	};

	struct Transform {
		glm::vec3 position{0};
		glm::vec3 rotation{0};
		glm::vec3 scale{1};

		[[nodiscard]] glm::mat4 getMatrix() const {
			glm::mat4 transform{1};
			transform = glm::translate(transform, position);

			transform = glm::rotate(transform, glm::radians(rotation.x), {1, 0, 0});
			transform = glm::rotate(transform, glm::radians(rotation.y), {0, 1, 0});
			transform = glm::rotate(transform, glm::radians(rotation.z), {0, 0, 1});

			transform = glm::scale(transform, scale);
			return (transform == glm::mat4(0)) ? glm::mat4(1) : transform;
		}
	};

	struct Material {
		std::string name;

		uint32_t baseColor;
		ShaderPtr shader;

		void apply() const;
	};

	using MaterialPtr = std::shared_ptr<Material>;

	class DrawableObject {
	public:
		virtual void draw(const Transform& model) = 0;
		virtual void draw(const ShaderPtr& shader, const Transform& model) = 0;

		std::string name = "undefined";
		Transform transform;
		bool castShadow{true};

		virtual ~DrawableObject() = default;
	};

	using DrawableObjectPtr = std::shared_ptr<DrawableObject>;

	class Mesh : public DrawableObject {
	public:
		Mesh() : drawable(true), VBO(0), VAO(0), EBO(0) {}

		bool drawable;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t VBO, VAO, EBO;
		MaterialPtr material;

		void draw(const Transform& model) override;
		void draw(const ShaderPtr& shader, const Transform& model) override;

		void setup();

		~Mesh() override;
	};

	using MeshPtr = std::shared_ptr<Mesh>;

	class Model : public DrawableObject {
	public:
		Model() = default;

		[[nodiscard]] MeshPtr findMeshByName(const std::string& name) const;

		std::vector<MeshPtr> meshes; // unordered map for quicker search by name
		void draw(const Transform& model = {}) override;
		void draw(const ShaderPtr& shader, const Transform& model) override;

		~Model() override;
	};
	using ModelPtr = std::shared_ptr<Model>;

	class Sprite : public DrawableObject {
	public:
		Sprite(uint32_t texture);

		ShaderPtr shader;

		void draw(const Transform& model = {}) override;
		void draw(const ShaderPtr& shader, const Transform& model) override;

		~Sprite() override = default;

	private:
		uint32_t texture;
		uint32_t VAO, VBO, EBO;
	};

	using SpritePtr = std::shared_ptr<Sprite>;

}
