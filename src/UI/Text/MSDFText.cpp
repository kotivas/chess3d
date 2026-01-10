#include "MSDFText.hpp"
#include "ResourceMgr/ResourceMgr.hpp"

namespace MsdfText {
    GLuint vao = 0, vbo = 0;

    float Font::getStringWidth(const std::string &text, float scale) {
        float width = 0.f;
        for (const char cc : text) width += getGlyph(cc).advance * scale;
        return width;
    }

    Glyph Font::getGlyph(const char &c) {
        const uint8_t code = static_cast<uint8_t>(c);

        auto it = this->glyphs.find(code);
        if (it == this->glyphs.end()) it = this->glyphs.find(GLYPH_PLACEHOLDER);

        return it->second;
    }

    void Init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    int
    GetMaxCharactersForWidth(const std::string &text, const FontPtr &font, const float scale, const float max_width) {
        float width = 0.f;
        int maxCharacters = 0;

        for (const char cc : text) {
            // if (cc == '\n') maxCharacters = 0; // new line
            width += font->getGlyph(cc).advance * scale;
            if (width > max_width) break;
            maxCharacters++;
        }

        return maxCharacters;
    }

    int GetMaxCharactersForWidth(const Text &text, const float max_width) {
        return GetMaxCharactersForWidth(text.string, text.font, text.scale, max_width);
    }

    float CalcTextWidth(const Text &text) {
        return CalcTextWidth(text.string, text.font, text.scale);
    }

    float CalcTextWidth(const std::string &text, const FontPtr &font, float scale) {
        float width = 0.f;

        for (const char cc : text) width += font->getGlyph(cc).advance * scale;

        return width;
    }
} // namespace MsdfText
