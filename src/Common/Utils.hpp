#pragma once
#include <optional>
#include <sstream>
#include <string>
#include "Renderer/Model.hpp"
#include "ResourceMgr/ResourceMgr.hpp"

namespace Utils {
	template <typename T, size_t N>
	std::string array_to_string(const std::array<T, N>& arr) {
		std::ostringstream oss;
		for (size_t i = 0; i < N; ++i) {
			oss << arr[i];
			if (i != N - 1) oss << ", ";
		}
		return oss.str();
	}

	std::string trim(const std::string& s);
	std::vector<float> ParseFloatList(const std::string& str);
	std::string ReadFromFile(const std::string& path);
	Renderer::MeshPtr CreateSphereMesh(const std::string& name, float radius, uint32_t stackCount, uint32_t sliceCount);
	Renderer::MeshPtr CreateCubeMesh(const std::string& name);
	Renderer::MeshPtr CreatePlaneMesh(const std::string& name);
	void SaveAsPNG(const std::string& path, std::vector<uint8_t>& pixels, uint16_t w, uint16_t h, uint16_t channels);

	template <typename T>
	std::optional<T> TryParse(const std::string& str);
}
