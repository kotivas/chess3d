#include "resource_manager.hpp"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../util.hpp"

#include <filesystem>

namespace ResourceManager {
	static uint32_t defaultTexture;

	uint32_t CreateDefaultTexture(glm::ivec3 color1, glm::ivec3 color2) {
		// Создание данных для текстуры 2x2 RGBA
		unsigned char textureData[2 * 2 * 4] = {
			// BLACK (0, 0, 0, 255)
			(uint8_t)color1.r, (uint8_t)color1.g, (uint8_t)color1.b, 255,
			// MAGENTA (255, 0, 255, 255)
			(uint8_t)color2.r, (uint8_t)color2.g, (uint8_t)color2.b, 255,
			// BLACK (0, 0, 0, 255)
			(uint8_t)color1.r, (uint8_t)color1.g, (uint8_t)color1.b, 255,
			// MAGENTA (255, 0, 255, 255)
			(uint8_t)color2.r, (uint8_t)color2.g, (uint8_t)color2.b, 255
		};

		// Генерация текстуры в OpenGL
		uint32_t textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}

	uint32_t CreateTexture(const std::string& path) {
		uint32_t textureLoc;
		int width, height;

		// Load and create a texture
		glGenTextures(1, &textureLoc);
		glBindTexture(GL_TEXTURE_2D, textureLoc);
		// All upcoming GL_TEXTURE_2D operations now have effect on this texture object

		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load image, create texture and generate mipmaps
		unsigned char* image = stbi_load(path.c_str(), &width, &height, 0, STBI_rgb_alpha);

		if (!image) {
			std::cout << OUT_WARNING << "Unable to load " << path << " - " << stbi_failure_reason() << std::endl;
			return defaultTexture;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

		std::cout << OUT_DEBUG << "Texture loaded: " << path << std::endl;

		return textureLoc;
	}

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

		aiString diffuseTexPath;
		if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexPath) == AI_SUCCESS) {
			nmat->baseColor = CreateTexture(diffuseTexPath.C_Str());
		} else if (std::filesystem::exists("assets/textures/" + modelName + "_diffuse.png")) {
			// assets/textures/desk/BaseColor.png
			nmat->baseColor = CreateTexture("assets/textures/" + modelName + "_diffuse.png");
		} else {
			std::cout << OUT_WARNING << "No diffuse texture for material: " << nmat->name;
			std::cout << " (unable to find at path " << "assets/textures/" + modelName + "_diffuse.png)" << std::endl;
			nmat->baseColor = CreateDefaultTexture({0, 0, 0}, {255, 0, 255});
		}

		return nmat;
	}

	Render::ModelPtr LoadModel(const std::string& path, Render::ShaderPtr shader) {
		defaultTexture = CreateDefaultTexture({0, 0, 0}, {255, 0, 255});

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			//throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
			std::cout << OUT_ERROR << "Failed to load model: " + std::string(importer.GetErrorString()) << std::endl;
			return nullptr;
		}

		// create model and set name
		Render::ModelPtr model = std::make_unique<Render::Model>();
		std::string name = path.substr(path.find_last_of("/\\") + 1);
		name = name.substr(0, name.find_last_of('.'));
		model->name = name;


		// handle materials
		std::vector<Render::MaterialPtr> nmats;

		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* aiMaterial = scene->mMaterials[i];
			nmats.push_back(ProcessMaterial(aiMaterial, model->name));
			nmats.back()->shader = shader;
		}

		// if there is no material in file, create and use default one
		if (scene->mNumMaterials == 0) {
			std::cout << OUT_WARNING << "No materials found in model" << std::endl;
			Render::MaterialPtr nmat = std::make_shared<Render::Material>();
			nmat->baseColor = defaultTexture;
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

		std::cout << OUT_DEBUG << "Model loaded: " << name << std::endl;
		return model;
	}
}
