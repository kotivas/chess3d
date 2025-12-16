#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/vec4.hpp>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Common/Color.hpp"

namespace MSDFText {

	constexpr char GLYPH_PLACEHOLDER = 0x63; // character [?]: if there is no glyph corresponding to the character

	struct Glyph {
		uint32_t codepoint;
		float advance;
		float planeLeft, planeRight, planeTop, planeBottom;
		float uvLeft, uvRight, uvTop, uvBottom;
	};

	struct Font {
		GLuint atlas = 0;
		int atlasW = 0, atlasH = 0;
		float descender = 0, ascender = 0;
		float distanceRange = 0.0f;
		float lineHeight = 1.0f;
		std::unordered_map<uint32_t, Glyph> glyphs;

		Glyph getGlyph(const char& c);
		float getStringWidth(const std::string& text, float scale);
	};

	using FontPtr = std::shared_ptr<Font>;

	struct Text {
		std::string string;
		float scale;
		glm::vec2 position;
		Color::rgba_t color;
		FontPtr font;
	};

	void Init();

	float CalcTextWidth(const std::string& text, const FontPtr& font, float scale);
	float CalcTextWidth(const Text& text);

	int GetMaxCharactersForWidth(const std::string& text, const FontPtr& font, float scale, float maxWidth);
	int GetMaxCharactersForWidth(const Text& text, float maxWidth);

	// float MaxCharPer
}
