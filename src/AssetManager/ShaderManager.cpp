#include "ShaderManager.hpp"
#include "Common/Utils.hpp"
#include "Core/Logger.hpp"

namespace AssetManager {
    Renderer::Shader *ShaderManager::get(const std::string &name) const {
        if (!_handles.contains(name)) return nullptr;
        return get(_handles.at(name));
    }

    Renderer::Shader *ShaderManager::get(const ShaderHandle &handle) const {
        if (_shaders.empty()) return nullptr;
        if (_shaders.size() <= handle.id) return nullptr;
        return _shaders[handle.id].shader.get();
    }

    ShaderHandle ShaderManager::getHandle(const std::string &name) const {
        if (!_handles.contains(name)) return ShaderHandle(-1);
        return _handles.at(name);
    }

    void ShaderManager::reload(const std::string &name) {
        reload(getHandle(name));
    }

    void ShaderManager::reload(const ShaderHandle &handle) {
        if (_shaders.empty()) return;
        if (_shaders.size() <= handle.id) return;

        ShaderSlot &shader = _shaders[handle.id];

        shader.shader = makeShader(shader.vertexPath, shader.fragmentPath, shader.geometryPath);

        Log::Info("Shader \"{0}\" ({1}) was reloaded", shader.name, handle.id);
    }

    std::unique_ptr<Renderer::Shader> ShaderManager::makeShader(
        const std::string &vertex_path, const std::string &fragment_path, const std::string &geometry_path) {
        const GLuint vs = compileShader(GL_VERTEX_SHADER, Utils::ReadFromFile(vertex_path).c_str());
        const GLuint fs = compileShader(GL_FRAGMENT_SHADER, Utils::ReadFromFile(fragment_path).c_str());
        GLuint gs = 0;
        if (!geometry_path.empty()) gs = compileShader(GL_GEOMETRY_SHADER, Utils::ReadFromFile(geometry_path).c_str());

        if (!vs || !fs || (!gs && !geometry_path.empty())) return nullptr;

        uint16_t shaderLoc = glCreateProgram();

        glAttachShader(shaderLoc, vs);
        glAttachShader(shaderLoc, fs);
        if (gs) glAttachShader(shaderLoc, gs);

        glLinkProgram(shaderLoc);
        glValidateProgram(shaderLoc);

        glDeleteShader(vs);
        glDeleteShader(fs);
        if (gs) glDeleteShader(gs);

        return std::make_unique<Renderer::Shader>(shaderLoc);
    }

    void ShaderManager::load(
        const std::string &name,
        const std::string &vertex_path,
        const std::string &fragment_path,
        const std::string &geometry_path) {
        ShaderSlot shaderSlot{
            .shader = makeShader(vertex_path, fragment_path, geometry_path),
            .name = name,
            .fragmentPath = fragment_path,
            .geometryPath = geometry_path,
            .vertexPath = vertex_path,
        };

        if (!shaderSlot.shader) {
            Log::Error("Unable to load shader \"{0}\"", name);
            return;
        }

        _shaders.push_back(std::move(shaderSlot));
        ShaderHandle handle;
        handle.id = _shaders.size() - 1;
        _handles.emplace(name, handle);
        Log::Debug("Shader \"{0}\" ({1}) loaded", name, handle.id);
    }

    void ShaderManager::free(const ShaderHandle &handle) {
        Log::Debug("Shader \"{0}\" ({1}) was unloaded", "$NAME", handle.id);
        _handles.erase("this");
        _shaders.erase(_shaders.begin() + handle.id);
    }

    uint16_t ShaderManager::compileShader(uint16_t shader_type, const char *source) {
        const GLuint loc = glCreateShader(shader_type);

        glShaderSource(loc, 1, &source, nullptr);
        glCompileShader(loc);

        GLint success;
        glGetShaderiv(loc, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(loc, 512, nullptr, infoLog);
            Log::Error("Shader compilation failed: " + std::string(infoLog));
            return 0;
        }
        return loc;
    }
} // namespace AssetManager
