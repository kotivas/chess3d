#pragma once
#include "Renderer/Model.hpp"
#include "Renderer/Shader.hpp"
#include "UI/Text/MSDFText.hpp"
#include <assimp/scene.h>
#include <unordered_map>

#include "Renderer/Sky.hpp"

namespace ResourceMgr {
	// TODO make centrilazed textures and meshes

	uint32_t CreateDefaultTexture(glm::ivec3 color1, glm::ivec3 color2);
	uint32_t CreateTexture(const std::string& path);

	Renderer::MeshPtr ProcessMesh(const aiMesh* aiMesh);
	Renderer::MaterialPtr ProcessMaterial(aiMaterial* aiMaterial, const std::string& directory);
	GLuint compileShader(GLenum shaderType, const std::string& src);

	bool LoadModel(const std::string& name, const std::string& path, Renderer::ShaderPtr shader);
	void LoadMSDFFont(const std::string& name, const std::string& pngPath, const std::string& jsonPath);
	void LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath,
	                const std::string& geometryPath = "");
	void LoadTexture(const std::string& name, const std::string& path);
	void LoadSkybox(const std::string& name, const std::string& path);

	uint32_t GetTextureByName(const std::string& name);
	Renderer::ModelPtr GetModelByName(const std::string& name);
	Renderer::ShaderPtr GetShaderByName(const std::string& name);
	MSDFText::FontPtr GetFontByName(const std::string& name);

	extern std::unordered_map<std::string, uint32_t> g_textures;
	extern std::unordered_map<std::string, Renderer::ModelPtr> g_models;
	extern std::unordered_map<std::string, MSDFText::FontPtr> g_fonts;
	extern std::unordered_map<std::string, Renderer::ShaderPtr> g_shaders;
	extern std::unordered_map<const aiMaterial*, Renderer::MaterialPtr> g_materials;
	extern uint32_t defaultTexture;
}
