#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <memory>
#include <iostream>

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
		glm::quat quaternion{0, 0, 0, 1};
		glm::vec3 scale{1};

		[[nodiscard]] glm::mat4 getMatrix() const {
			glm::mat4 transform{1};
			transform = glm::translate(transform, position);
			transform *= glm::mat4_cast(quaternion);
			transform = glm::scale(transform, scale);
			return transform;
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

	class DrawableObject {
	public:
		DrawableObject(const std::string& name)
			: drawable(true), name(name) {
		}

		virtual void draw(const Transform3d& model = {}) = 0;
		virtual void draw(const ShaderPtr& shader, const Transform3d& model = {}) = 0;

		bool drawable;
		std::string name;
		Transform3d transform;
		bool castShadow{true};

		virtual ~DrawableObject() = default;
	};

	using DrawableObjectPtr = std::shared_ptr<DrawableObject>;

	class Mesh final : public DrawableObject {
	public:
		Mesh(const std::string& name = "undefined") : DrawableObject(name), VBO(0), VAO(0), EBO(0) {
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t VBO, VAO, EBO;
		MaterialPtr material;

		void draw(const Transform3d& model = {}) override;
		void draw(const ShaderPtr& shader, const Transform3d& model) override;

		void setup();

		~Mesh() override;
	};

	using MeshPtr = std::shared_ptr<Mesh>;

	class Model final : public DrawableObject {
	public:
		Model(const std::string& name = "undefined")
			: DrawableObject(name) {
		}

		[[nodiscard]] MeshPtr findMeshByName(const std::string& name) const;

		std::vector<MeshPtr> meshes; // unordered map for quicker search by name
		void draw(const Transform3d& model = {}) override;
		void draw(const ShaderPtr& shader, const Transform3d& model) override;

		~Model() override;
	};

	using ModelPtr = std::shared_ptr<Model>;
}
