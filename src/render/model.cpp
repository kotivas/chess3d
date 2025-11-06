#include "model.hpp"

#include "../com/util.hpp"
#include "core/logger.hpp"

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

		// Настройка атрибута тангента
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

		glBindVertexArray(0);
	}

	void Material::apply() const {
		assert(shader);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, diffuse[0]);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, specular[0]);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, normal[0]);

		shader->use();

		shader->setUniform1f("material.shininess", shininess); // to _materials
		shader->setUniform3f("material.solidColor", solidColor);
		shader->setUniform1i("material.useSolidColor", useSolidColor);
		shader->setUniform1i("shadowMap", 0);

		shader->setUniform1i("dirShadowMap", 0);
		shader->setUniform1i("spotShadowMap", 1);
		shader->setUniform1i("omniShadowMap", 2);
		shader->setUniform1i("material.diffuse", 3);
		shader->setUniform1i("material.specular", 4);
		shader->setUniform1i("material.normal", 5);
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
		Log::Warning("No mesh with name " + name + " in model " + this->name);
		return nullptr;
	}


	Model::~Model() {
		meshes.clear();
	}
}
