#pragma once
#include <deque>
#include <string>
#include <vector>
#include "com/color.hpp"
#include "core/logger.hpp"

namespace Console {
	constexpr double BLINK_INTERVAL = 0.5;

	struct CMDLine {
		CMDLine(const Color::rgb_t color, const std::string& text)
			: color(color), text(text) {}

		Color::rgb_t color;
		std::string text;
	};

	extern int g_scrollOffset;
	extern bool g_isVisible;
	extern std::vector<CMDLine> g_messages;
	extern std::deque<std::string> g_history;
	extern int g_historyIndex;
	extern std::string g_inputField;
	extern double g_blinkTimer;
	extern bool g_blinked;
	extern int g_cursorIndent;
	extern std::string g_suggestion;

	void Init();
	void Update(double dt);
	void Draw();

	bool IsVisible();

	void Toggle();

	void NavigateHistory(int direction);
	void HandleBackspace();

	void ExecuteCommand(const std::string& command);

	void Print(const Color::rgb_t& color, const std::string& message);
	void Print(Logger::Severity sev, const std::string& message);
	void Print(const CMDLine& message);
}
