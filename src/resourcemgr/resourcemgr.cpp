#include "resourcemgr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "../com/util.hpp"
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <filesystem>

#include "core/logger.hpp"

namespace ResourceMgr {
	static uint32_t defaultTexture = 0;
	std::unordered_map<std::string, Render::ModelPtr> g_models;
	std::unordered_map<std::string, MSDFText::FontPtr> g_fonts;
	std::unordered_map<std::string, Render::ShaderPtr> g_shaders;
	std::unordered_map<std::string, uint32_t> g_textures;


	Render::ModelPtr GetModelByName(const std::string& name) {
		return g_models.contains(name) ? g_models[name] : nullptr;
	}

	Render::ShaderPtr GetShaderByName(const std::string& name) {
		return g_shaders.contains(name) ? g_shaders[name] : nullptr;
	}

	MSDFText::FontPtr GetFontByName(const std::string& name) {
		return g_fonts.contains(name) ? g_fonts[name] : nullptr;
	}

	uint32_t GetTextureByName(const std::string& name) {
		return g_textures.contains(name) ? g_textures[name] : defaultTexture;
	}

	void LoadTexture(const std::string& name, const std::string& path) {
		uint32_t texture = CreateTexture(path);
		g_textures.insert({name, texture});
	}


	void LoadMSDFFont(const std::string& name, const std::string& pngPath, const std::string& jsonPath) {
		MSDFText::FontPtr font = std::make_unique<MSDFText::Font>();

		int w, h, channels;
		stbi_set_flip_vertically_on_load(true); // keep true if you generated with --yorigin bottom

		unsigned char* data = stbi_load(pngPath.c_str(), &w, &h, &channels, 0);

		if (!data) {
			Log::Error("[MSDF] Failed to load texture: " + pngPath + " WHY: " + stbi_failure_reason());
			return;
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
		if (!jsonfile.is_open()) {
			Log::Error("[MSDF] Failed to open JSON: " + jsonPath + " WHY: " + std::to_string(jsonfile.rdstate()));
			return;
		}

		rapidjson::IStreamWrapper isw(jsonfile);
		rapidjson::Document doc;
		doc.ParseStream(isw);

		if (!doc.IsObject()) {
			Log::Error("[MSDF] Invalid JSON format in: " + jsonPath);
			return;
		}

		// --- Parse atlas info ---
		if (!doc.HasMember("atlas") || !doc["atlas"].IsObject()) {
			Log::Error("[MSDF] Missing atlas section in metadata in " + jsonPath);
			return;
		}

		const auto& atlas = doc["atlas"];
		font->distanceRange = atlas["distanceRange"].GetFloat();
		font->atlasW = atlas["width"].GetInt();
		font->atlasH = atlas["height"].GetInt();

		// --- Parse metrics ---
		if (!doc.HasMember("metrics") || !doc["metrics"].IsObject()) {
			Log::Error("[MSDF] Missing metrics section in " + jsonPath);
			return;
		}

		const auto& metrics = doc["metrics"];
		font->lineHeight = metrics["lineHeight"].GetFloat();
		font->ascender = metrics["ascender"].GetFloat();
		font->descender = metrics["descender"].GetFloat();

		// --- Parse glyphs ---
		if (!doc.HasMember("glyphs") || !doc["glyphs"].IsArray()) {
			Log::Error("[MSDF] Missing glyph array in " + jsonPath);
			return;
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

		Log::Debug(
			"MSDF Font loaded: " + pngPath + " (" + std::to_string(font->glyphs.size()) + " glyphs " +
			std::to_string(w) + "x" +
			std::to_string(h) + " atlas)");

		// return font;
		g_fonts.insert({name, font});
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
			Log::Warning("Unable to load " + path + " - " + stbi_failure_reason());
			return defaultTexture;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

		Log::Debug("Texture loaded: " + path);

		return textureLoc;
	}
}
