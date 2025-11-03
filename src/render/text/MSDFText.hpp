#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/vec4.hpp>
#include <memory>
#include <glm/vec3.hpp>

namespace MSDFText {
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
	};

	using FontPtr = std::shared_ptr<Font>;

	struct Text {
		std::string text;
		float scale;
		glm::vec3 position;
		glm::vec4 color;
		FontPtr font;
	};

	void Init();
	void DrawText(const std::string& text, const FontPtr& font, float x, float y, float scale, glm::vec4 color);
	void DrawText(const Text& text);
}
