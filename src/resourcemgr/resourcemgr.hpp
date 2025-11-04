#pragma once
#include "./render/model.hpp"
#include "./render/shader.hpp"
#include "./text/MSDFText.hpp"
#include <assimp/scene.h>
#include <unordered_map>

namespace ResourceMgr {
	// TODO make centrilazed textures and meshes

	uint32_t CreateDefaultTexture(glm::ivec3 color1, glm::ivec3 color2);
	uint32_t CreateTexture(const std::string& path);

	Render::MeshPtr ProcessMesh(aiMesh* aiMesh);
	Render::MaterialPtr ProcessMaterial(aiMaterial* aiMaterial, const std::string& modelName);
	GLuint compileShader(GLenum shaderType, const std::string& src);

	void LoadModel(const std::string& name, const std::string& path, Render::ShaderPtr shader);
	void LoadMSDFFont(const std::string& name, const std::string& pngPath, const std::string& jsonPath);
	void LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,
	                const std::string& geometryPath = "");
	void LoadTexture(const std::string& name, const std::string& path);

	uint32_t GetTextureByName(const std::string& name);
	Render::ModelPtr GetModelByName(const std::string& name);
	Render::ShaderPtr GetShaderByName(const std::string& name);
	MSDFText::FontPtr GetFontByName(const std::string& name);

	extern std::unordered_map<std::string, uint32_t> g_textures;
	extern std::unordered_map<std::string, Render::ModelPtr> g_models;
	extern std::unordered_map<std::string, MSDFText::FontPtr> g_fonts;
	extern std::unordered_map<std::string, Render::ShaderPtr> g_shaders;
	extern uint32_t defaultTexture;
}
