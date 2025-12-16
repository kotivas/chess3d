#pragma once
#include <string>

#include "Shader.hpp"

namespace Renderer {
	struct Sky {
		// Sky();

		~Sky() {
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			// glDeleteBuffers(1, &EBO);
		}

		void update(float time) {
			// every hour sun rotates at 15° (0.00417° per sec)
			// float angular_speed = 15 * glm::pi<float>() / 180.0f; // rad/hour
			// angular_speed /= 3600;

			float dayFraction = glm::fract(time / 86400); // 0..1
			float sunElev = (dayFraction * glm::two_pi<float>()); // -1..1 (ночь/день)
			sunDir = glm::normalize(glm::vec3(0.0, sin(sunElev), cos(sunElev)));

		}

		// void generateGeometry(float radius = 100.f, uint32_t segments = 16, uint32_t rings = 16) {
		// 	for (uint32_t y = 0; y <= rings; ++y) {
		// 		float v = float(y) / float(rings);
		// 		float phi = v * glm::pi<float>();
		//
		// 		for (uint32_t x = 0; x <= segments; ++x) {
		// 			float u = float(x) / float(segments);
		// 			float theta = u * glm::two_pi<float>();
		//
		// 			glm::vec3 pos{
		// 				radius * sin(phi) * cos(theta),
		// 				radius * cos(phi),
		// 				radius * sin(phi) * sin(theta)
		// 			};
		// 			glm::vec3 normal = glm::normalize(pos);
		//
		// 			vertices.push_back(Vertex{
		// 				pos,
		// 				normal,
		// 				glm::vec2(u, v),
		// 				glm::vec3(1.0f, 0.0f, 0.0f)
		// 			});
		// 		}
		// 	}
		// }

		void setup() {

			float quadVertices[] = {
				-1.f,  1.f,
				-1.f, -1.f,
				 1.f,  1.f,
				 1.f, -1.f
			};

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
			glBindVertexArray(0);


			// glGenVertexArrays(1, &VAO);
			// glGenBuffers(1, &VBO);
			//
			// glBindVertexArray(VAO);
			//
			// glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
			//
			//
			// // Vertex positions
			// glEnableVertexAttribArray(0);
			// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
			//
			// glBindVertexArray(0);
		}

		std::vector<Vertex> vertices;
		// std::vector<uint32_t> indices;
		uint32_t VAO, VBO;
		AssetManager::ShaderHandle shader;
		glm::vec3 sunDir;
		Color::rgb_t sunColor;

		float cirrusDensity;
		float cumulusDensity;
	};

	using SkyPtr = std::shared_ptr<Sky>;
}
