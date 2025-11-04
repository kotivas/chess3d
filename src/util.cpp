#include "util.hpp"

#include "core/logger.hpp"

namespace Util {
	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
	                            const char* message, const void* userParam) {
		// ignore non-significant error/warning codes
		// if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
		if (id == 131204 || id == 131185) return;

		std::string severityStr;
		std::string sourceStr;
		std::string typeStr;
		Logger::Severity logSeverity = Logger::Severity::Debug;

		switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			severityStr = "high";
			logSeverity = Logger::Severity::Error;
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			severityStr = "medium";
			logSeverity = Logger::Severity::Warning;
			break;
		case GL_DEBUG_SEVERITY_LOW:
			severityStr = "low";
			logSeverity = Logger::Severity::Info;
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			severityStr = "info";
			logSeverity = Logger::Severity::Debug;
			break;
		default: severityStr = "unknown";
		}

		switch (source) {
		case GL_DEBUG_SOURCE_API: sourceStr = "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application";
			break;
		case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other";
			break;
		default: sourceStr = "unknown";
		}

		switch (type) {
		case GL_DEBUG_TYPE_ERROR: typeStr = "Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behaviour";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behaviour";
			break;
		case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance";
			break;
		case GL_DEBUG_TYPE_MARKER: typeStr = "Marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group";
			break;
		case GL_DEBUG_TYPE_OTHER: typeStr = "Other";
			break;
		default: typeStr = "unknown";
		}

		std::string msg = "GL (" + severityStr + "; " + sourceStr + "; " + typeStr + "): " + message + " (" +
			std::to_string(id) + ")";
		Log::Log(logSeverity, msg);
	}

	Render::MeshPtr CreatePlaneMesh(float shininess, const std::string& name) {
		Render::MeshPtr mesh = std::make_shared<Render::Mesh>();;
		Render::MaterialPtr mat = std::make_shared<Render::Material>();

		// setup material

		mat->name = name;
		mat->diffuse[0] = ResourceMgr::CreateDefaultTexture({200, 200, 200}, {200, 200, 200}); // TODO solid color
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
			Log::Error("Could`nt open file " + path);
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
