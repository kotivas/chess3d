#pragma once
#include <string>
#include <vector>
#include "com/color.hpp"
#include "core/logger.hpp"

namespace Console {
	struct CMDLine {
		CMDLine(const Color::rgb_t color, const std::string& text)
			: color(color), text(text) {}

		Color::rgb_t color;
		std::string text;
	};

	extern int g_scrollOffset;
	extern bool g_isVisible;
	extern std::vector<CMDLine> g_messages;
	extern std::string g_inputField;

	void Init();
	void Update();
	void Draw();

	bool IsVisible();

	void Toggle();

	void ExecuteCommand(const std::string& command);

	void Print(const Color::rgb_t& color, const std::string& message);
	void Print(Logger::Severity sev, const std::string& message);
	void Print(const CMDLine& message);
}
