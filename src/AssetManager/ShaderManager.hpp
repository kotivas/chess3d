#pragma once

#include "AssetManager/Handle.hpp"
#include "Renderer/Shader.hpp"

namespace AssetManager {
	struct ShaderSlot {
		std::unique_ptr<Renderer::Shader> shader;
		std::string name;
		std::string fragment_path;
		std::string geometry_path;
		std::string vertex_path;
	};

	class ShaderManager {
	public:
		ShaderManager() : _next_id(0) {
		}

		[[nodiscard]] Renderer::Shader* get(const std::string& name) const;
		[[nodiscard]] Renderer::Shader* get(const ShaderHandle& handle) const;
		[[nodiscard]] ShaderHandle getHandle(const std::string& name) const;
		void reload(const std::string& name);
		void reload(const ShaderHandle& handle);
		void load(const std::string& name, const std::string& vertex_path, const std::string& fragment_path,
		          const std::string& geometry_path = "");
		void free(const ShaderHandle& handle);

	private:
		static uint16_t compileShader(uint16_t shader_type, const char* source);
		static std::unique_ptr<Renderer::Shader> makeShader(const std::string& vertex_path,
		                                                    const std::string& fragment_path,
		                                                    const std::string& geometry_path);

		uint32_t _next_id;
		std::vector<ShaderSlot> _shaders;
		std::unordered_map<std::string, ShaderHandle> _handles;
	};
}
