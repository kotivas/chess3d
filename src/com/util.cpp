#include "util.hpp"
#include <fstream>
#include <ranges>
#include "../core/logger.hpp"

namespace Util {

	std::string trim(const std::string& s) {
		const auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
		const auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
		return (start < end ? std::string(start, end) : std::string());
	}

	template <typename T>
	std::optional<T> TryParse(const std::string& str) {
		Log::Warning("Util::TryParse<T>: no parser defined for this type");
		return std::nullopt;
	}

	template <>
	std::optional<int> TryParse<int>(const std::string& str) {
		try {
			return std::stoi(trim(str));
		} catch (...) { return std::nullopt; }
	}

	template <>
	std::optional<float> TryParse<float>(const std::string& str) {
		try {
			return std::stof(trim(str));
		} catch (...) { return std::nullopt; }
	}

	template <>
	std::optional<bool> TryParse<bool>(const std::string& str) {
		const std::string trimstr = trim(str);
		if (trimstr == "true" || trimstr == "1") return true;
		if (trimstr == "false" || trimstr == "0") return false;
		return std::nullopt;
	}

	template <>
	std::optional<std::string> TryParse<std::string>(const std::string& str) {
		return str;
	}

	std::vector<float> ParseFloatList(const std::string& str) {
		std::vector<float> vals;
		std::string clean;
		clean.reserve(str.size());
		for (char c : str)
			clean += (c == ',' ? ' ' : c);

		std::istringstream ss(clean);
		float v;
		while (ss >> v)
			vals.push_back(v);

		return vals;
	}

	// new parsers
	template <>
	std::optional<std::array<float, 2>> TryParse<std::array<float, 2>>(const std::string& str) {
		auto vals = ParseFloatList(str);
		if (vals.size() != 2) return std::nullopt;
		return std::array{vals[0], vals[1]};
	}

	template <>
	std::optional<std::array<float, 3>> TryParse<std::array<float, 3>>(const std::string& str) {
		auto vals = ParseFloatList(str);
		if (vals.size() != 3) return std::nullopt;
		return std::array{vals[0], vals[1], vals[2]};
	}

	template <>
	std::optional<std::array<float, 4>> TryParse<std::array<float, 4>>(const std::string& str) {
		auto vals = ParseFloatList(str);
		if (vals.size() != 4) return std::nullopt;
		return std::array{vals[0], vals[1], vals[2], vals[3]};
	}


	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
	                            const char* message, const void* userParam) {
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

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

		Log::Log(logSeverity, "GL ({0};{1};{2}): {3} ({4})", severityStr, sourceStr, typeStr, message, id);
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
