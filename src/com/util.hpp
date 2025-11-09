#pragma once
#include <optional>
#include <string>

#include "../render/model.hpp"
#include "../resourcemgr/resourcemgr.hpp"

namespace Util {
	template <typename T, size_t N>
	std::string array_to_string(const std::array<T, N>& arr);

	std::string trim(const std::string& s);
	std::vector<float> ParseFloatList(const std::string& str);
	const std::string readFromFile(const std::string& path);
	Render::MeshPtr CreateSphereMesh(float radius, uint32_t stackCount, uint32_t sliceCount);
	Render::MeshPtr CreatePlaneMesh(float shininess, const std::string& name);
	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
	                            const char* message, const void* userParam);

	template <typename T>
	std::optional<T> TryParse(const std::string& str);
}
