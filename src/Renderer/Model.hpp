#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <memory>
#include <iostream>
#include "Renderer/Material.hpp"

#include "Shader.hpp"

namespace Renderer {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
	};

	struct Transform3d {
		glm::vec3 position{0};
		glm::quat quaternion{0, 0, 0, 0};
		glm::vec3 scale{1};

		[[nodiscard]] glm::mat4 getMatrix() const {
			const glm::mat4 t = glm::translate({1}, position);
			const glm::mat4 s = glm::scale({1}, scale);
			const glm::mat4 r = glm::mat4_cast(quaternion);
			return t * r * s;
		}
	};

	class Drawable {
	public:
		Drawable(const std::string& name)
			: drawable(true), name(name) {
		}

		enum Type {Mesh, Model} type;

		bool drawable;
		std::string name;
		Transform3d transform;
		bool castShadow{true};

		virtual ~Drawable() = default;
	};

	class Mesh final : public Drawable {
	public:
		Mesh(const std::string& name = "undefined") : Drawable(name), VBO(0), VAO(0), EBO(0) {
			type = Drawable::Mesh;
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t VBO, VAO, EBO;
		MaterialPtr material;

		void setup();

		~Mesh() override;
	};

	using MeshPtr = std::shared_ptr<Mesh>;

	class Model final : public Drawable {
	public:
		Model(const std::string& name = "undefined")
			: Drawable(name) {
			type = Drawable::Model;
		}

		[[nodiscard]] MeshPtr findMeshByName(const std::string& name) const;

		std::vector<MeshPtr> meshes; // unordered map for quicker search by name

		~Model() override;
	};

	using ModelPtr = std::shared_ptr<Model>;
}
