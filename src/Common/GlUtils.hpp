#pragma once
#include <string>
#include <glad/glad.h>

namespace GlUtils {
	void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
	void SaveFrame(const std::string& directory);
}
