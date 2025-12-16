#pragma once
#include "Renderer/Model.hpp"
#include "Renderer/Shader.hpp"
#include "UI/Text/MSDFText.hpp"
#include <assimp/scene.h>
#include <unordered_map>

#include "Renderer/Sky.hpp"
#include "Renderer/Texture.hpp"

namespace ResourceMgr {
	extern std::unordered_map<std::string, uint32_t> g_textures;
	extern std::unordered_map<std::string, Renderer::ModelPtr> g_models;
	extern std::unordered_map<std::string, MSDFText::FontPtr> g_fonts;
	extern std::unordered_map<const aiMaterial*, Renderer::MaterialPtr> g_materials;

	uint32_t CreateTexture2D(const std::string& path, Renderer::TextureWrapMode wrap_mode);
	uint32_t CreateTexture3D(const std::string& path, uint16_t h, uint16_t w, uint16_t d,
	                         Renderer::TextureWrapMode wrap_mode);

	// ---------- GEOMETRY ----------
	Renderer::MeshPtr ProcessMesh(const aiMesh* aiMesh);
	Renderer::MaterialPtr ProcessMaterial(aiMaterial* aiMaterial, const std::string& directory);

	// ---------- RESOURCES ----------
	bool LoadModel(const std::string& name, const std::string& path, AssetManager::ShaderHandle shader);
	void LoadMSDFFont(const std::string& name, const std::string& pngPath, const std::string& jsonPath);
	void LoadTexture(const std::string& name, Renderer::TextureType type, Renderer::TextureWrapMode wrap_mode,
	                 const std::string& path);
	void LoadSkybox(const std::string& name, const std::string& path);

	inline Renderer::ModelPtr GetModelByName(const std::string& name) {
		return g_models.contains(name) ? g_models[name] : nullptr;
	}

	inline MSDFText::FontPtr GetFontByName(const std::string& name) {
		return g_fonts.contains(name) ? g_fonts[name] : nullptr;
	}

	inline uint32_t GetTextureByName(const std::string& name) {
		return g_textures.contains(name) ? g_textures[name] : 0;
	}
}
