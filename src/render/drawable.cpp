#include "drawable.hpp"

#include "../util.hpp"

namespace Render {
	void Mesh::setup() {
		// Генерация и настройка VAO, VBO и EBO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// Загрузка данных вершин в VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		// Загрузка данных индексов в EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// Настройка атрибута позиции
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

		// Настройка атрибута нормали
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		// Настройка атрибута текстурных координат
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		glBindVertexArray(0);
	}

	void Material::apply() const {
		assert(shader);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baseColor);

		shader->use();

		shader->setUniform1i("baseColor", 0);
	}

	void Mesh::draw(const Transform& model) {
		assert(material);

		material->apply();

		glm::mat4 modelMat = this->transform.getMatrix() * model.getMatrix();
		material->shader->setUniformMat4fv("u_Model", GL_FALSE, modelMat);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::draw(const ShaderPtr& shader, const Transform& model) {
		assert(material);

		shader->use();

		glm::mat4 modelMat = this->transform.getMatrix() * model.getMatrix();
		shader->setUniformMat4fv("u_Model", GL_FALSE, modelMat);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	Mesh::~Mesh() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}


	void Model::draw(const ShaderPtr& shader, const Transform& model) {
		for (const MeshPtr& mesh : meshes) {
			if (mesh->drawable) {
				mesh->draw(shader, this->transform);
			}
		}
	}

	void Model::draw(const Transform& model) {
		for (const MeshPtr& mesh : meshes) {
			if (mesh->drawable) {
				mesh->draw(this->transform);
			}
		}
	}

	MeshPtr Model::findMeshByName(const std::string& name) const {
		for (const MeshPtr& mesh : meshes) {
			if (mesh && mesh->name == name) {
				return mesh;
			}
		}
		std::cerr << OUT_WARNING << "No mesh with name " << name << " in model " << name << std::endl;
		return nullptr;
	}


	Model::~Model() {
		std::cout << OUT_WARNING << "Model " << name << " destructed" << std::endl;
		meshes.clear();
	}

	Sprite::Sprite(uint32_t texture)
		: texture(texture), shader(nullptr) {
		name = "ShadowBlob";

		std::vector<Vertex> vertices = {
		{{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
	};

		std::vector indices = {0, 2, 1, 0, 3, 2};

		// Генерация и настройка VAO, VBO и EBO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// Загрузка данных вершин в VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		// Загрузка данных индексов в EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// Настройка атрибута позиции
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

		// Настройка атрибута нормали
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		// Настройка атрибута текстурных координат
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		glBindVertexArray(0);
	}

	void Sprite::draw(const Transform& model) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		shader->use();
		shader->setUniform1i("baseColor", 0);

		glm::mat4 modelMat = this->transform.getMatrix() * model.getMatrix();
		shader->setUniformMat4fv("u_Model", GL_FALSE, modelMat);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void Sprite::draw(const ShaderPtr& shader, const Transform& model) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		shader->use();
		shader->setUniform1i("baseColor", 0);

		glm::mat4 modelMat = this->transform.getMatrix() * model.getMatrix();
		shader->setUniformMat4fv("u_Model", GL_FALSE, modelMat);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 5, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
