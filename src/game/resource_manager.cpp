#include "resource_manager.hpp"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../util.hpp"
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <filesystem>

namespace ResourceManager {
	static uint32_t defaultTexture;

	MSDFText::FontPtr LoadMSDFFont(const std::string& pngPath, const std::string& jsonPath) {
		MSDFText::FontPtr font = std::make_shared<MSDFText::Font>();

		int w, h, channels;
		stbi_set_flip_vertically_on_load(true); // keep true if you generated with --yorigin bottom
		unsigned char* data = stbi_load(pngPath.c_str(), &w, &h, &channels, 0);
		if (!data) {
			std::cerr << "[MSDF] Failed to load texture: " << pngPath << "\n";
			return nullptr;
		}

		font->atlasW = w;
		font->atlasH = h;

		GLint format = (channels == 3) ? GL_RGB : GL_RGBA;

		glGenTextures(1, &font->atlas);
		glBindTexture(GL_TEXTURE_2D, font->atlas);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);

		// Important: no mipmaps, only linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // no mip levels
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		// 👇 Clamp to edge (prevent bleeding between glyphs)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		stbi_image_free(data);

		// --- Load JSON metadata ---
		// FILE* fp = fopen(jsonPath.c_str(), "rb");
		std::ifstream jsonfile(jsonPath);
		if (!jsonfile.good()) {
			std::cerr << "[MSDF] Failed to open JSON: " << jsonPath << "\n";
			return font;
		}

		char readBuffer[65536];
		rapidjson::IStreamWrapper is(jsonfile, readBuffer, sizeof(readBuffer));
		rapidjson::Document doc;
		doc.ParseStream(is);

		jsonfile.close();

		if (!doc.IsObject()) {
			std::cerr << "[MSDF] Invalid JSON format in: " << jsonPath << "\n";
			return font;
		}

		// --- Parse atlas info ---
		if (!doc.HasMember("atlas") || !doc["atlas"].IsObject()) {
			std::cerr << "[MSDF] Missing atlas section in metadata\n";
			return nullptr;
		}

		const auto& atlas = doc["atlas"];
		font->distanceRange = atlas["distanceRange"].GetFloat();
		font->atlasW = atlas["width"].GetInt();
		font->atlasH = atlas["height"].GetInt();

		// --- Parse metrics ---
		if (!doc.HasMember("metrics") || !doc["metrics"].IsObject()) {
			std::cerr << "[MSDF] Missing metrics section\n";
			return nullptr;
		}

		const auto& metrics = doc["metrics"];
		font->lineHeight = metrics["lineHeight"].GetFloat();
		font->ascender = metrics["ascender"].GetFloat();
		font->descender = metrics["descender"].GetFloat();

		// --- Parse glyphs ---
		if (!doc.HasMember("glyphs") || !doc["glyphs"].IsArray()) {
			std::cerr << "[MSDF] Missing glyph array\n";
			return nullptr;
		}

		const auto& glyphs = doc["glyphs"];
		for (const auto& g : glyphs.GetArray()) {
			MSDFText::Glyph glyph{};
			glyph.codepoint = g["unicode"].GetUint();
			glyph.advance = g["advance"].GetFloat();

			if (g.HasMember("planeBounds")) {
				const auto& plane = g["planeBounds"];
				glyph.planeLeft = plane["left"].GetFloat();
				glyph.planeBottom = plane["bottom"].GetFloat();
				glyph.planeRight = plane["right"].GetFloat();
				glyph.planeTop = plane["top"].GetFloat();
			}

			if (g.HasMember("atlasBounds")) {
				const auto& ab = g["atlasBounds"];
				glyph.uvLeft = ab["left"].GetFloat() / font->atlasW;
				glyph.uvRight = ab["right"].GetFloat() / font->atlasW;
				glyph.uvBottom = ab["bottom"].GetFloat() / font->atlasH;
				glyph.uvTop = ab["top"].GetFloat() / font->atlasH;
			}

			font->glyphs[glyph.codepoint] = glyph;
		}

		std::cout << "[MSDF] Loaded font: " << font->glyphs.size()
			<< " glyphs (" << w << "x" << h << " atlas)\n";

		return font;
	}

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
		stbi_set_flip_vertically_on_load(false);
		unsigned char* image = stbi_load(path.c_str(), &width, &height, 0, 3);

		if (!image) {
			std::cout << OUT_WARNING << "Unable to load " << path << " - " << stbi_failure_reason() << std::endl;
			return defaultTexture;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
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
			std::cout << OUT_WARNING << "No diffuse texture for material: " << nmat->name;
			std::cout << " (unable to find at path " << "assets/textures/" + modelName + "/" + nmat->name +
				"_diffuse.png)" << std::endl;
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

		std::cout << OUT_DEBUG << "Model loaded: " << name << std::endl;
		return model;
	}
}
