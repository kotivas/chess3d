#pragma once
#include <string>
#include <utility>
#include <vector>

namespace Console {
	enum SeverityLevel : uint8_t {
		Critical = 0,
		Error,
		Warning,
		Info,
		Debug
	};

	struct CMDMessage {
		CMDMessage(const SeverityLevel sev, std::string text)
			: severity(sev), text(std::move(text)) {}
		// timestamp
		SeverityLevel severity;
		std::string text;
	};

	constexpr int MESSAGE_PER_PAGE = 20;
	constexpr int FONT_SCALE = 20;

	extern std::vector<CMDMessage> g_messages;
	extern int scrollOffset;
	extern bool g_isVisible;

	void Init();
	void Update();
	void Draw();

	void Toggle();

	void Print(SeverityLevel level, const std::string& message);
	void Print(const CMDMessage& message);
}
