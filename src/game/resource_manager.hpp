#pragma once
#include "../render/model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace ResourceManager {

uint32_t CreateDefaultTexture();
uint32_t CreateTexture(const std::string& path);

Render::MeshPtr ProcessMesh(aiMesh* aiMesh);
Render::MaterialPtr ProcessMaterial(aiMaterial* aiMaterial);
Render::ModelPtr LoadModel(const std::string& path, Render::ShaderPtr shader);

}