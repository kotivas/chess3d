#pragma once

#include <string>
#include <fstream>
#include "./render/model.hpp"
#include "./game/resource_manager.hpp"

namespace Util {
	// #define OUT_ERROR "\033[1m\033[31m[ERROR] \033[0m"
	// #define OUT_DEBUG "\033[1m\033[93m[DEBUG] \033[0m"
	// #define OUT_WARNING "\033[1m\033[33m[WARNING] \033[0m"
	// #define OUT_INFO "\033[1m\033[34m[INFO] \033[0m"

#define OUT_ERROR "[ERROR] "
#define OUT_DEBUG "[DEBUG] "
#define OUT_WARNING "[WARNING] "
#define OUT_INFO "[INFO] "

	const std::string readFromFile(const std::string& path);
	Render::MeshPtr CreateSphereMesh(float radius, uint32_t stackCount, uint32_t sliceCount);
	Render::MeshPtr CreatePlaneMesh(float shininess, const std::string& name);
}
