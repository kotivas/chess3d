#include "resourcemgr.hpp"

#include "../com/util.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <filesystem>

#include "core/logger.hpp"


namespace ResourceMgr {

	Render::MeshPtr ProcessMesh(aiMesh* aiMesh) {
		Render::MeshPtr mesh = std::make_unique<Render::Mesh>();
		mesh->name = aiMesh->mName.C_Str();
		// Обработка вершин
		for (unsigned int j = 0; j < aiMesh->mNumVertices; j++) {
			Render::Vertex vertex;

			// Позиция вершины
			vertex.pos.x = aiMesh->mVertices[j].x;
			vertex.pos.y = aiMesh->mVertices[j].y;
			vertex.pos.z = aiMesh->mVertices[j].z;

			if (aiMesh->HasTangentsAndBitangents()) {
				// mTangents/ mBittangetnts
				vertex.tangent.x = aiMesh->mTangents[j].x;
				vertex.tangent.y = aiMesh->mTangents[j].y;
				vertex.tangent.z = aiMesh->mTangents[j].z;
			}

			// Нормали
			if (aiMesh->HasNormals()) {
				vertex.normal.x = aiMesh->mNormals[j].x;
				vertex.normal.y = aiMesh->mNormals[j].y;
				vertex.normal.z = aiMesh->mNormals[j].z;
			}

			// Текстурные координаты
			if (aiMesh->HasTextureCoords(0)) {
				vertex.texCoords.x = aiMesh->mTextureCoords[0][j].x;
				vertex.texCoords.y = aiMesh->mTextureCoords[0][j].y;
			}

			mesh->vertices.push_back(vertex);
		}

		// Обработка индексов
		for (unsigned int j = 0; j < aiMesh->mNumFaces; j++) {
			aiFace face = aiMesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++) {
				mesh->indices.push_back(face.mIndices[k]);
			}
		}

		//// Материал меша
		//if (aiMesh->mMaterialIndex >= 0) {
		//	mesh->material = mat;
		//}

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
		case aiTextureType_BASE_COLOR: return "BASE_COLOR"; // Для PBR
		// ... другие типы
		default: return "UNKNOWN";
		}
	}

	Render::MaterialPtr ProcessMaterial(aiMaterial* aiMaterial, const std::string& modelName) {
		Render::MaterialPtr nmat = std::make_shared<Render::Material>();

		aiString matName;
		aiMaterial->Get(AI_MATKEY_NAME, matName);
		nmat->name = matName.C_Str();

		float shininess;
		aiMaterial->Get(AI_MATKEY_SHININESS, shininess);
		nmat->shininess = shininess;
		if (nmat->shininess == 0) nmat->shininess = 32.f;

		aiString diffuseTexPath;
		if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexPath) == AI_SUCCESS) {
			nmat->diffuse[0] = CreateTexture(diffuseTexPath.C_Str());
		} else if (std::filesystem::exists("assets/textures/" + modelName + "/" + nmat->name + "_diffuse.png")) {
			// assets/textures/desk/BaseColor.png
			nmat->diffuse[0] = CreateTexture("assets/textures/" + modelName + "/" + nmat->name + "_diffuse.png");
		} else {
			Log::Warning("No diffuse texture for material: " + std::string(nmat->name) + " (unable to find at path " +
			             "assets/textures/" + modelName + "/" + nmat->name + "_diffuse.png)");
			nmat->diffuse[0] = CreateDefaultTexture({0, 0, 0}, {255, 0, 255});
		}

		aiString specularTexPath;
		if (aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &specularTexPath) == AI_SUCCESS) {
			nmat->specular[0] = CreateTexture(specularTexPath.C_Str());
		} else if (std::filesystem::exists("assets/textures/" + modelName + "/" + nmat->name + "_specular.png")) {
			nmat->specular[0] = CreateTexture("assets/textures/" + modelName + "/" + nmat->name + "_specular.png");
		}

		aiString normalTexPath;
		if (aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &normalTexPath) == AI_SUCCESS) {
			nmat->normal[0] = CreateTexture(normalTexPath.C_Str());
		} else if (std::filesystem::exists("assets/textures/" + modelName + "/" + nmat->name + "_normal.png")) {
			nmat->normal[0] = CreateTexture("assets/textures/" + modelName + "/" + nmat->name + "_normal.png");
		}

		return nmat;
	}

	void LoadModel(const std::string& name, const std::string& path, Render::ShaderPtr shader) {
		defaultTexture = CreateDefaultTexture({0, 0, 0}, {255, 0, 255});

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			Log::Error("Failed to load model: " + std::string(importer.GetErrorString()));
			return;
		}

		// create model and set name
		Render::ModelPtr model = std::make_unique<Render::Model>();
		std::string model_name = path.substr(path.find_last_of("/\\") + 1);
		model_name = model_name.substr(0, model_name.find_last_of('.'));
		model->name = model_name;


		// handle materials
		std::vector<Render::MaterialPtr> nmats;

		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* aiMaterial = scene->mMaterials[i];
			nmats.push_back(ProcessMaterial(aiMaterial, model->name));
			nmats.back()->shader = shader;
		}

		// if there is no material in file, create and use default one
		if (scene->mNumMaterials == 0) {
			Log::Warning("No materials found in model: " + path);
			Render::MaterialPtr nmat = std::make_shared<Render::Material>();
			nmat->diffuse[0] = defaultTexture;
			nmat->name = "default";
			nmat->shader = shader;
			nmats.push_back(nmat);
		}

		// handle meshes

		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* aiMesh = scene->mMeshes[i];
			Render::MeshPtr mesh = ProcessMesh(aiMesh);
			mesh->material = nmats[aiMesh->mMaterialIndex];
			model->meshes.push_back(mesh);
		}

		Log::Debug("Model loaded: " + name);

		g_models.insert({name, model});
	}
}
