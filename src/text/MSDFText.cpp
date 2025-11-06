#include "MSDFText.hpp"
#include "resourcemgr/resourcemgr.hpp"
#include "game/config.hpp"

namespace MSDFText {
	GLuint vao = 0, vbo = 0;


	Glyph Font::getGlyph(const char& c) {
		const uint8_t code = static_cast<uint8_t>(c);

		auto it = this->glyphs.find(code);
		if (it == this->glyphs.end()) {
			it = this->glyphs.find(GLYPH_PLACEHOLDER);
		}

		return it->second;
	}

	void Init() {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
	}

	int GetMaxCharactersForWidth(const std::string& text, const FontPtr& font, const float scale,
	                             const float maxWidth) {
		float width = 0.f;
		int maxCharacters = 0;

		for (const char cc : text) {
			// if (cc == '\n') maxCharacters = 0; // new line
			width += font->getGlyph(cc).advance * scale;
			if (width > maxWidth) break;
			maxCharacters++;
		}

		return maxCharacters;
	}

	int GetMaxCharactersForWidth(const Text& text, const float maxWidth) {
		return GetMaxCharactersForWidth(text.string, text.font, text.scale, maxWidth);
	}

	float CalcTextWidth(const Text& text) {
		return CalcTextWidth(text.string, text.font, text.scale);
	}

	float CalcTextWidth(const std::string& text, const FontPtr& font, float scale) {
		float width = 0.f;

		for (const char cc : text) {
			width += font->getGlyph(cc).advance * scale;
		}

		return width;
	}

	void DrawText(const Text& text) {
		MSDFText::DrawText(text.string, text.font, text.position.x, text.position.y, text.scale, text.color);
	}

	void DrawText(const std::string& text, const FontPtr& font, const float x, const float y, const float scale,
	              const glm::vec4 color) {
		struct Vtx {
			float x, y, u, v;
		};

		if (!font) return;

		std::vector<Vtx> verts;
		verts.reserve(text.size() * 6);

		float penX = x;
		float penY = y + font->ascender * scale; // baseline offset (≈ ascender)

		for (const char cc : text) {
			// if (cc == '\n') {
			// 	penX = x;
			// 	penY -= font->lineHeight * scale;
			// 	continue;
			// }

			const Glyph& g = font->getGlyph(cc);

			float gx0 = penX + g.planeLeft * scale;
			float gy0 = penY + g.planeBottom * scale;
			float gx1 = penX + g.planeRight * scale;
			float gy1 = penY + g.planeTop * scale;

			float u0 = g.uvLeft;
			float v0 = g.uvBottom;
			float u1 = g.uvRight;
			float v1 = g.uvTop;

			verts.push_back({gx0, gy0, u0, v0});
			verts.push_back({gx1, gy0, u1, v0});
			verts.push_back({gx1, gy1, u1, v1});
			verts.push_back({gx1, gy1, u1, v1});
			verts.push_back({gx0, gy1, u0, v1});
			verts.push_back({gx0, gy0, u0, v0});

			penX += g.advance * scale;
		}

		if (verts.empty()) return;

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vtx), verts.data(), GL_DYNAMIC_DRAW);

		GLint posLoc = 0;
		GLint uvLoc = 1;
		glEnableVertexAttribArray(posLoc);
		glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)0);
		glEnableVertexAttribArray(uvLoc);
		glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)(2 * sizeof(float)));

		ResourceMgr::GetShaderByName("msdf_text")->use();

		glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(g_config.r_resolution.x), 0.0f,
		                            static_cast<float>(g_config.r_resolution.y));

		ResourceMgr::GetShaderByName("msdf_text")->setUniformMat4fv("uProjection", false, proj);
		ResourceMgr::GetShaderByName("msdf_text")->setUniform4f("uColor", color.r, color.g, color.b, color.a);
		ResourceMgr::GetShaderByName("msdf_text")->setUniform1f("uScale", scale);
		ResourceMgr::GetShaderByName("msdf_text")->setUniform1f("uPxRange", font->distanceRange);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font->atlas);
		ResourceMgr::GetShaderByName("msdf_text")->setUniform1i("uAtlas", 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());

		glDisableVertexAttribArray(posLoc);
		glDisableVertexAttribArray(uvLoc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}
}
