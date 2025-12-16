#include "ResourceMgr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <filesystem>
#include <fstream>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <glad/glad.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "Common/Utils.hpp"
#include "Core/Logger.hpp"

namespace ResourceMgr {
	std::unordered_map<std::string, MSDFText::FontPtr> g_fonts;
	std::unordered_map<std::string, Renderer::ModelPtr> g_models;
	std::unordered_map<std::string, uint32_t> g_textures;
	std::unordered_map<const aiMaterial*, Renderer::MaterialPtr> g_materials;

	void LoadTexture(const std::string& name, const Renderer::TextureType type,
	                 const Renderer::TextureWrapMode wrap_mode, const std::string& path) {
		uint32_t textureLoc;
		switch (type) {
		case Renderer::TextureType::Tex2D:
			textureLoc = CreateTexture2D(path, wrap_mode);
			break;
		case Renderer::TextureType::Tex3D:
			textureLoc = CreateTexture3D(path, 128, 128, 128, wrap_mode);
			break;
		default:
			textureLoc = 0;
			Log::Warning("Unsupported texture type! {0}", name);
			break;
		};
		g_textures.insert({name, textureLoc});
	}

	uint32_t CreateTexture3D(const std::string& path, uint16_t h, uint16_t w, uint16_t d,
	                         const Renderer::TextureWrapMode wrap_mode) {
		uint32_t textureLoc = 0;

		std::vector<float> noiseData(h * w * d);
		std::ifstream f(path, std::ios::binary);
		f.read((char*)noiseData.data(), noiseData.size() * sizeof(float));
		f.close();

		glGenTextures(1, &textureLoc);
		glBindTexture(GL_TEXTURE_3D, textureLoc);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F,
		             w, h, d, 0,
		             GL_RED, GL_FLOAT, noiseData.data());

		const GLint wrap = static_cast<GLint>(wrap_mode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap);

		glBindTexture(GL_TEXTURE_3D, 0);

		Log::Debug("3D Texture loaded: {0} ({1}x{2}x{3})", path, w, h, d);

		return textureLoc;
	}

	uint32_t CreateTexture2D(const std::string& path, const Renderer::TextureWrapMode wrap_mode) {
		uint32_t textureLoc;
		int width, height, comp;

		// Load and create a texture
		glGenTextures(1, &textureLoc);
		glBindTexture(GL_TEXTURE_2D, textureLoc);
		// All upcoming GL_TEXTURE_2D operations now have effect on this texture object

		const GLint wrap = static_cast<GLint>(wrap_mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap); // repeat by default
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		// Set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Load image, create texture and generate mipmaps
		stbi_set_flip_vertically_on_load(false);
		unsigned char* image = stbi_load(path.c_str(), &width, &height, &comp, 3);

		if (!image) {
			Log::Warning("Unable to load " + path + " - " + stbi_failure_reason());
			return 0;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

		Log::Debug("Texture loaded: {0} ({1}x{2} {3})", path, width, height, comp);

		return textureLoc;
	}


	void LoadMSDFFont(const std::string& name, const std::string& pngPath, const std::string& jsonPath) {
		MSDFText::FontPtr font = std::make_unique<MSDFText::Font>();

		int w, h, channels;
		stbi_set_flip_vertically_on_load(true); // keep true if you generated with --yorigin bottom

		unsigned char* data = stbi_load(pngPath.c_str(), &w, &h, &channels, 0);

		if (!data) {
			Log::Error("LoadMSDFFont: Failed to load image: " + pngPath + " WHY: " + stbi_failure_reason());
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
			Log::Error(
				"LoadMSDFFont: Failed to open JSON: " + jsonPath + " WHY: " + std::to_string(jsonfile.rdstate()));
			return;
		}

		rapidjson::IStreamWrapper isw(jsonfile);
		rapidjson::Document doc;
		doc.ParseStream(isw);

		if (!doc.IsObject()) {
			Log::Error("LoadMSDFFont: Invalid JSON format in: " + jsonPath);
			return;
		}

		// --- Parse atlas info ---
		if (!doc.HasMember("atlas") || !doc["atlas"].IsObject()) {
			Log::Error("LoadMSDFFont: Missing atlas section in metadata in " + jsonPath);
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

		Log::Debug("MSDF Font loaded: {0} ({1}x{2})", pngPath, w, h);

		// return font;
		g_fonts.insert({name, font});
	}
}
