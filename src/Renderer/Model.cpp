#include "Model.hpp"

#include "Common/Utils.hpp"
#include "Core/Logger.hpp"

namespace Renderer {
    void Mesh::setup() {
        // gen VAO VBO EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // vertices -> VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        // indices -> EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // positions attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

        // Настройка атрибута нормали
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

        // Настройка атрибута текстурных координат
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));

        // Настройка атрибута тангента
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tangent));

        glBindVertexArray(0);
    }

    Mesh::~Mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    MeshPtr Model::findMeshByName(const std::string &name) const {
        for (const MeshPtr &mesh : meshes)
            if (mesh && mesh->name == name) return mesh;
        Log::Warning("No mesh with name {0} in model {1}", name, this->name);
        return nullptr;
    }


    Model::~Model() {
        meshes.clear();
    }
} // namespace Renderer
