#include "util.hpp"

namespace Util {
	Render::MeshPtr CreatePlaneMesh(float shininess, const std::string& name) {
		Render::MeshPtr mesh = std::make_shared<Render::Mesh>();;
		Render::MaterialPtr mat = std::make_shared<Render::Material>();

		// setup material

		mat->name = name;
		mat->diffuse[0] = ResourceManager::CreateDefaultTexture({200, 200, 200}, {200, 200, 200}); // TODO solid color
		mat->shininess = shininess;

		// setup mesh

		mesh->name = name;

		mesh->vertices = {
			{{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
		};

		mesh->indices = {0, 2, 1, 0, 3, 2};
		mesh->material = mat;

		mesh->setup();

		return mesh;
	}

	Render::MeshPtr CreateSphereMesh(float radius, uint32_t stackCount, uint32_t sliceCount) {
		Render::MeshPtr mesh = std::make_shared<Render::Mesh>();
		mesh->name = "sphere";

		// Генерация вершин
		mesh->vertices.push_back({
			glm::vec3(0.0f, radius, 0.0f), // position
			glm::vec3(0.0f, 1.0f, 0.0f), // normal
			glm::vec2(0.5f, 0.0f) // tex_coords
		});

		const float pi = glm::pi<float>();
		for (uint32_t stack = 1; stack < stackCount; ++stack) {
			float theta = stack * pi / stackCount;
			float sinTheta = sin(theta);
			float cosTheta = cos(theta);

			for (uint32_t slice = 0; slice <= sliceCount; ++slice) {
				float phi = slice * 2.0f * pi / sliceCount;
				float sinPhi = sin(phi);
				float cosPhi = cos(phi);

				Render::Vertex v;
				v.pos = glm::vec3(
					radius * sinTheta * cosPhi,
					radius * cosTheta,
					radius * sinTheta * sinPhi
				);
				v.normal = glm::normalize(v.pos);
				v.texCoords = glm::vec2(
					static_cast<float>(slice) / sliceCount,
					static_cast<float>(stack) / stackCount
				);

				mesh->vertices.push_back(v);
			}
		}

		mesh->vertices.push_back({
			glm::vec3(0.0f, -radius, 0.0f), // position
			glm::vec3(0.0f, -1.0f, 0.0f), // normal
			glm::vec2(0.5f, 1.0f) // tex_coords
		});

		// Генерация индексов (counter-clockwise порядок)
		const uint32_t poleStart = 0;
		const uint32_t ringVertexCount = sliceCount + 1;

		// Верхний полюс
		for (uint32_t slice = 0; slice < sliceCount; ++slice) {
			mesh->indices.push_back(poleStart);
			mesh->indices.push_back(1 + (slice + 1) % sliceCount);
			mesh->indices.push_back(1 + slice);
		}

		// Основные кольца
		for (uint32_t stack = 0; stack < stackCount - 2; ++stack) {
			uint32_t ringStart = 1 + stack * ringVertexCount;
			uint32_t nextRingStart = ringStart + ringVertexCount;

			for (uint32_t slice = 0; slice < sliceCount; ++slice) {
				// Первый треугольник квада (counter-clockwise)
				mesh->indices.push_back(ringStart + slice);
				mesh->indices.push_back(ringStart + slice + 1);
				mesh->indices.push_back(nextRingStart + slice);

				// Второй треугольник квада (counter-clockwise)
				mesh->indices.push_back(nextRingStart + slice);
				mesh->indices.push_back(ringStart + slice + 1);
				mesh->indices.push_back(nextRingStart + slice + 1);
			}
		}

		// Нижний полюс
		const uint32_t bottomPoleIndex = static_cast<uint32_t>(mesh->vertices.size() - 1);
		const uint32_t lastRingStart = 1 + (stackCount - 2) * ringVertexCount;

		for (uint32_t slice = 0; slice < sliceCount; ++slice) {
			mesh->indices.push_back(bottomPoleIndex);
			mesh->indices.push_back(lastRingStart + slice);
			mesh->indices.push_back(lastRingStart + slice + 1);
		}

		mesh->setup();

		return mesh;
	}

	const std::string readFromFile(const std::string& path) {
		std::string content;
		std::ifstream fileStream(path, std::ios::in);

		if (!fileStream.is_open()) {
			std::cout << OUT_ERROR << "Could not open file: " << path << std::endl;
			return "";
		}

		std::string line = "";
		while (!fileStream.eof()) {
			std::getline(fileStream, line);
			content.append(line + "\n");
		}

		fileStream.close();
		return content;
	}
}
