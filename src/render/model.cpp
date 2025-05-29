#include "model.hpp"

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

		// Настройка атрибутов вершин
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

		glBindVertexArray(0);
	}

	void Material::apply() const {
		assert(shader);

		// bind diffuse map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, diffuse[0]);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, specular[0]);

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
	}

	void Mesh::draw(Transform& model) {
		assert(material);

		material->apply();


		glm::mat4 modelMat = this->transform.getMatrix() * model.getMatrix();
		material->shader->setUniformMat4fv("u_Model", GL_FALSE, modelMat);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::draw(const Transform& model, const ShaderPtr& shader) {
		assert(material);

		shader->use();

		glm::mat4 modelMat = this->transform.getMatrix() * model.getMatrix();
		shader->setUniformMat4fv("u_Model", GL_FALSE, modelMat);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void Model::draw(const ShaderPtr& shader) {
		for (const MeshPtr& mesh : meshes) {
			if (mesh->drawable) {
				mesh->draw(this->transform, shader);
			}
		}
	}

	void Model::draw() {
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
}
