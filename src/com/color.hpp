#pragma once


// normalized color values for gpu
namespace Color {
	struct ColorRGB {
		constexpr ColorRGB(float r, float g, float b) : r(r), g(g), b(b) {}
		constexpr ColorRGB() : r(0), g(0), b(0) {}
		float r;
		float g;
		float b;
	} typedef rgb_t;

	struct ColorRGBA {
		constexpr ColorRGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
		constexpr ColorRGBA(ColorRGB rgb, float a) : r(rgb.r), g(rgb.g), b(rgb.b), a(a) {}
		constexpr ColorRGBA() : r(0), g(0), b(0), a(1) {}
		float r;
		float g;
		float b;
		float a;
	} typedef rgba_t;

	using hex_t = uint32_t; // 0xAABBFF

	inline rgb_t Normalize(uint8_t r, uint8_t g, uint8_t b) { return {r / 255.f, g / 255.f, b / 255.f}; }
	inline rgba_t Normalize(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return {r / 255.f, g / 255.f, b/255.f, a/255.f}; }
	
	constexpr rgb_t BLACK{0.0f, 0.0f, 0.0f};
	constexpr rgb_t WHITE{1.0f, 1.0f, 1.0f};
	constexpr rgb_t RED{1.0f, 0.0f, 0.0f};
	constexpr rgb_t LIME{0.0f, 1.0f, 0.0f};
	constexpr rgb_t BLUE{0.0f, 0.0f, 1.0f};
	constexpr rgb_t YELLOW{1.0f, 1.0f, 0.0f};
	constexpr rgb_t MAGENTA{1.0f, 0.0f, 1.0f};
	constexpr rgb_t CYAN{0.0f, 1.0f, 1.0f};
	constexpr rgb_t ORANGE{1.0f, 0.65f, 0.0f};
	constexpr rgb_t GREEN{0.0f, 0.5f, 0.0f};
	constexpr rgb_t GRAY{0.5f, 0.5f, 0.5f};
	constexpr rgb_t SILVER{0.75f, 0.75f, 0.75f};
	constexpr rgb_t NAVY{0.0f, 0.0f, 0.5f};
	constexpr rgb_t PURPLE{0.5f, 0.0f, 0.5f};
	constexpr rgb_t TEAL{0.0f, 0.5f, 0.5f};
	constexpr rgb_t MAROON{0.5f, 0.0f, 0.0f};
	constexpr rgb_t LIGHT_RED{1.0f, 0.4f, 0.4f};
}
