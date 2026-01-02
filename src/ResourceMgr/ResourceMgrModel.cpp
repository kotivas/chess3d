#include "ResourceMgr.hpp"

#include "Common/Utils.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <fstream>
#include <filesystem>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <ranges>
#include <glm/gtx/matrix_decompose.hpp>

#include "Core/Logger.hpp"

namespace ResourceMgr {
	Renderer::MeshPtr ProcessMesh(const aiMesh* aiMesh) {
		Renderer::MeshPtr mesh = std::make_unique<Renderer::Mesh>();
		mesh->name = aiMesh->mName.C_Str();

		for (unsigned int j = 0; j < aiMesh->mNumVertices; j++) {
			Renderer::Vertex vertex;

			vertex.pos.x = aiMesh->mVertices[j].x;
			vertex.pos.y = aiMesh->mVertices[j].y;
			vertex.pos.z = aiMesh->mVertices[j].z;

			if (aiMesh->HasTangentsAndBitangents()) {
				vertex.tangent.x = aiMesh->mTangents[j].x;
				vertex.tangent.y = aiMesh->mTangents[j].y;
				vertex.tangent.z = aiMesh->mTangents[j].z;
			}

			if (aiMesh->HasNormals()) {
				vertex.normal.x = aiMesh->mNormals[j].x;
				vertex.normal.y = aiMesh->mNormals[j].y;
				vertex.normal.z = aiMesh->mNormals[j].z;
			}

			if (aiMesh->HasTextureCoords(0)) {
				vertex.texCoords.x = aiMesh->mTextureCoords[0][j].x;
				vertex.texCoords.y = aiMesh->mTextureCoords[0][j].y;
			}

			mesh->vertices.push_back(vertex);
		}

		for (unsigned int j = 0; j < aiMesh->mNumFaces; j++) {
			aiFace face = aiMesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++) {
				mesh->indices.push_back(face.mIndices[k]);
			}
		}

		mesh->setup();
		return mesh;
	}

	const char* TextureTypeToString(aiTextureType type) {
		switch (type) {
		case aiTextureType_NONE: return "NONE";
		case aiTextureType_DIFFUSE: return "DIFFUSE";
		case aiTextureType_SPECULAR: return "SPECULAR";
		case aiTextureType_AMBIENT: return "AMBIENT";
		case aiTextureType_EMISSIVE: return "EMISSIVE";
		case aiTextureType_NORMALS: return "NORMALS";
		case aiTextureType_BASE_COLOR: return "BASE_COLOR";
		default: return "UNKNOWN";
		}
	}

	glm::mat4 AssimpMatrixToGLM(const aiMatrix4x4& m) {
		return glm::mat4{
			m.a1, m.b1, m.c1, m.d1,
			m.a2, m.b2, m.c2, m.d2,
			m.a3, m.b3, m.c3, m.d3,
			m.a4, m.b4, m.c4, m.d4
		};
	}

	std::string getFileName(const std::string& fullPath) {
		size_t pos1 = fullPath.find_last_of('/');
		size_t pos2 = fullPath.find_last_of('\\');
		size_t pos;

		if (pos1 != std::string::npos && pos2 != std::string::npos)
			pos = std::max(pos1, pos2);
		else if (pos1 != std::string::npos)
			pos = pos1;
		else if (pos2 != std::string::npos)
			pos = pos2;
		else
			return fullPath; // нет слэшей, возвращаем как есть

		return fullPath.substr(pos + 1);
	}

	Renderer::MaterialPtr ProcessMaterial(aiMaterial* aiMaterial, const std::string& directory) {
		Renderer::MaterialPtr nmat = std::make_shared<Renderer::Material>();

		nmat->name = aiMaterial->GetName().C_Str();

		float shininess;
		aiMaterial->Get(AI_MATKEY_SHININESS, shininess);
		nmat->shininess = shininess;
		// if (nmat->shininess == 0) nmat->shininess = 32.f;

		aiString diffuseTexPath;
		if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexPath) == AI_SUCCESS) {
			nmat->diffuse = CreateTexture2D(directory + getFileName(diffuseTexPath.C_Str()),
			                                Renderer::TextureWrapMode::ClampToEdge);
			nmat->useDiffuse = true;
		} else {
			std::string texpath = diffuseTexPath.C_Str();
			Log::Warning("Unable to find material {0} diffuse texture at {1}", nmat->name, texpath);
			nmat->solidColor = {0.5, 0.0, 1.0};
		}

		aiString specularTexPath;
		if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &specularTexPath) == AI_SUCCESS) {
			nmat->specular = CreateTexture2D(directory + getFileName(specularTexPath.C_Str()),
			                                 Renderer::TextureWrapMode::ClampToEdge);
			nmat->useSpecular = true;
		}

		aiString normalTexPath;
		if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalTexPath) == AI_SUCCESS) {
			nmat->normal = CreateTexture2D(directory + getFileName(normalTexPath.C_Str()),
			                               Renderer::TextureWrapMode::ClampToEdge);
			nmat->useNormal = true;
		}

		aiString heightTexPath;
		if (aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &heightTexPath) == AI_SUCCESS) {
			nmat->normal = CreateTexture2D(directory + getFileName(heightTexPath.C_Str()),
			                               Renderer::TextureWrapMode::ClampToEdge);
			nmat->useNormal = true;
		}

		return nmat;
	}

	void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& transform, const Renderer::ModelPtr& model, const std::string& model_dir) {
		glm::mat4 global_transform = transform * AssimpMatrixToGLM(node->mTransformation);

		if (node->mNumMeshes > 0) {
			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
				Renderer::MeshPtr mesh = ProcessMesh(aiMesh);

				Renderer::Transform3d trans;
				glm::vec3 skew;
				glm::vec4 s;

				glm::decompose(global_transform, trans.scale, trans.quaternion, trans.position, skew, s);
				trans.quaternion = glm::normalize(trans.quaternion);

				aiMaterial* mat = scene->mMaterials[aiMesh->mMaterialIndex];
				if (!g_materials.contains(mat)) {
					Renderer::MaterialPtr nmat;

					nmat = ProcessMaterial(mat, model_dir);
					g_materials.emplace(mat, nmat);
				}

				mesh->material = g_materials[mat];

				mesh->transform = trans;

				model->meshes.push_back(mesh);
			}
		}

		for (int i = 0; i < node->mNumChildren; i++) ProcessNode(node->mChildren[i], scene, global_transform, model, model_dir);
	}

	bool LoadModel(const std::string& name, const std::string& path, AssetManager::ShaderHandle shader) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path, aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_OptimizeMeshes |
			aiProcess_OptimizeGraph);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			Log::Error("Failed to load model: " + std::string(importer.GetErrorString()));
			return false;
		}

		Renderer::ModelPtr model = std::make_unique<Renderer::Model>(name);
		const std::string directory = std::filesystem::path(path).parent_path().string() + "/";


		ProcessNode(scene->mRootNode, scene, glm::mat4(1.f), model, directory);

		for (const auto& mat : g_materials | std::views::values) {
			mat->shader = shader;
		}


		Log::Debug("Model loaded: " + name);
		g_models.insert({name, model});

		if (scene->HasLights()) Log::Info("Found lights but not handeled in {0}", path);
		if (scene->HasCameras()) Log::Info("Found cameras but not handeled in {0}", path);
		if (scene->HasAnimations()) Log::Info("Found animations but not handeled in {0}", path);

		return true;
	}
}
