#pragma once
#include <glad/glad.h>
#include <string>

namespace GlUtils {
    void APIENTRY GlDebugOutput(
        GLenum source,
        GLenum type,
        unsigned int id,
        GLenum severity,
        GLsizei length,
        const char *message,
        const void *userParam);
    void SaveFrame(const std::string &directory);
} // namespace GlUtils
