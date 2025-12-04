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

		void update(float timeOfDay) {
			// every hour sun rotates at 15°
			sunDir = glm::normalize(glm::vec3(0.0, sin(timeOfDay*0.1), cos(timeOfDay*0.1)));

			// x = cos(pitch) * cos(heading)
			// y = sin(pitch)
			// z = cos(pitch) * sin(heading)


		}

		void generateGeometry(float radius = 100.f, uint32_t segments = 32, uint32_t rings = 16,
		                      float skirtHeight = 1.0f) {
			for (uint32_t y = 0; y <= rings; ++y) {
				float v = float(y) / float(rings);
				float phi = v * glm::pi<float>();

				for (uint32_t x = 0; x <= segments; ++x) {
					float u = float(x) / float(segments);
					float theta = u * glm::two_pi<float>();

					glm::vec3 pos{
						radius * sin(phi) * cos(theta),
						radius * cos(phi),
						radius * sin(phi) * sin(theta)
					};
					glm::vec3 normal = glm::normalize(pos);

					vertices.push_back(Vertex{
						pos,
						normal,
						glm::vec2(u, v),
						glm::vec3(1.0f, 0.0f, 0.0f)
					});
				}
			}

			// skirt
			for (uint32_t x = 0; x <= segments; ++x) {
				float u = float(x) / float(segments);
				float theta = u * glm::two_pi<float>();
				glm::vec3 pos{
					radius * cos(theta),
					-skirtHeight,
					radius * sin(theta)
				};
				glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);

				vertices.push_back(Vertex{
					pos,
					normal,
					glm::vec2(u, 1.0f),
					glm::vec3(1.0f, 0.0f, 0.0f)
				});
			}
		}

		void setup() {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

			// Vertex positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

			glBindVertexArray(0);
		}

		std::vector<Vertex> vertices;
		// std::vector<uint32_t> indices;
		uint32_t VAO, VBO;
		ShaderPtr shader;
		glm::vec3 sunDir;
		Color::rgb_t sunColor;

		float cirrusDensity;
		float cumulusDensity;
	};

	using SkyPtr = std::shared_ptr<Sky>;
}
