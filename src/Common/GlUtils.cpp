#include "Common/GlUtils.hpp"
#include <Core/Logger.hpp>
#include <chrono>
#include "Common/Config.hpp"
#include "Common/Utils.hpp"

namespace GlUtils {
    void SaveFrame(const std::string &directory) {
        // width * height * channels (e.x RGB)

        auto now = std::chrono::system_clock::now();
        std::string filepath = directory + "/" + std::format("{:%d-%m-%Y_%H_%M_%S}", now) + ".png";

        const uint16_t width = g_config.windowResolution.x;
        const uint16_t height = g_config.windowResolution.y;

        std::vector<uint8_t> pixels(width * height * 3);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        std::vector<unsigned char> flippedPixels(pixels.size());
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x)
                for (int c = 0; c < 3; ++c)
                    flippedPixels[(height - 1 - y) * width * 3 + x * 3 + c] = pixels[y * width * 3 + x * 3 + c];
        }

        Utils::SaveAsPng(filepath, flippedPixels, width, height, 3);
        Log::Info("Screenshot saved as {0}", filepath);
    }

    void APIENTRY GlDebugOutput(
        GLenum source,
        GLenum type,
        unsigned int id,
        GLenum severity,
        GLsizei length,
        const char *message,
        const void *userParam) {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::string severityStr;
        std::string sourceStr;
        std::string typeStr;
        auto logSeverity = Logger::Severity::Debug;

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
        default:
            severityStr = "unknown";
        }

        switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceStr = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "Other";
            break;
        default:
            sourceStr = "unknown";
        }

        switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeStr = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeStr = "Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeStr = "Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "Other";
            break;
        default:
            typeStr = "unknown";
        }

        Log::Log(logSeverity, "GL ({0};{1};{2}): {3} ({4})", severityStr, sourceStr, typeStr, message, id);
    }
} // namespace GlUtils
