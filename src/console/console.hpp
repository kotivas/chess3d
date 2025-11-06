#pragma once
#include <string>
#include <utility>
#include <vector>

namespace Console {
	enum class SeverityLevel : uint8_t {
		Fatal = 0,
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

	extern int g_scrollOffset;
	extern bool g_isVisible;
	extern std::vector<CMDMessage> g_messages;

	void Init();
	void Update();
	void Draw();

	bool IsVisible();

	void Toggle();

	void Print(SeverityLevel level, const std::string& message);
	void Print(const CMDMessage& message);
}
