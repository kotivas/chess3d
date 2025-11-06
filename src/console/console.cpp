#include "console.hpp"

#include "game/config.hpp"
#include "input/input.hpp"
#include "render/renderer.hpp"
#include "core/logger.hpp"
#include "text/MSDFText.hpp"
#include "resourcemgr/resourcemgr.hpp"

namespace Console {
	int g_scrollOffset = 0;
	bool g_isVisible = false;
	std::vector<CMDMessage> g_messages;

	void Toggle() {
		g_isVisible = !g_isVisible;
	}

	bool IsVisible() {
		return g_isVisible;
	}

	void Init() {
		Print(SeverityLevel::Info, "Console inited");
	}

	void Update() {
		if (Input::GetScrollYOffset()) {
			g_scrollOffset += Input::GetScrollYOffset() < 0 ? -1 : 1;
		}
	}

	void Draw() {
		if (!g_isVisible) return;

		const MSDFText::FontPtr font = ResourceMgr::GetFontByName("inconsolata_light");
		if (!font) {
			Log::Error("UNABLE TO GET CONSOLE FONT");
			return;
		}

		const int& fontScale = g_config.con_fontScale;
		const int& maxVisibleLines = g_config.con_maxVisibleLines;

		const float lineOffset = font->lineHeight * fontScale;
		const float lineyzero = g_config.r_resolution.y - 2 * font->ascender * fontScale;
		const float maxHeight = maxVisibleLines * lineOffset;
		const float maxWidth = g_config.r_resolution.x;
		Renderer::DrawRectOnScreen(0, 0, maxWidth, maxHeight, g_config.con_backgroundColor);
		Renderer::DrawRectOnScreen(0, maxHeight, maxWidth, 1, {1, 1, 0, 1});

		if (g_messages.empty()) return;
		// STEP 1 --------------------------------- get visible messages vector
		std::vector<CMDMessage> messages;

		int total = static_cast<int>(g_messages.size());
		int maxScroll = std::max(0, total - maxVisibleLines);
		g_scrollOffset = std::clamp(g_scrollOffset, 0, maxScroll);

		int end = total - g_scrollOffset; // newest msgs
		int start = std::max(0, end - maxVisibleLines); // start

		messages.assign(g_messages.begin() + start, g_messages.begin() + end);
		// STEP 2 --------------------------------- parse line boxes
		std::vector<std::string> line_boxes;

		for (const auto& msg : messages) {
			if (msg.text.length() > MSDFText::GetMaxCharactersForWidth(msg.text, font, fontScale, maxWidth) ||
				std::ranges::count(msg.text, '\n') != 0) {
				// todo multiple lines
				int maxChar = MSDFText::GetMaxCharactersForWidth(msg.text, font, fontScale, maxWidth);
				line_boxes.push_back(msg.text.substr(0, maxChar));
				line_boxes.push_back(msg.text.substr(maxChar));
			} else {
				line_boxes.push_back(msg.text);
			}
		}

		if (line_boxes.size() > maxVisibleLines) {
			line_boxes.erase(line_boxes.begin(), line_boxes.begin() + (line_boxes.size() - maxVisibleLines));
		}

		// STEP 3 --------------------------------- rendering text

		for (int i = 0; i < line_boxes.size(); i++) {
			MSDFText::DrawText(line_boxes[i], font, 0, lineyzero - i * lineOffset, fontScale, {1, 1, 1, 1});
		}
	}

	void Print(SeverityLevel level, const std::string& message) {
		Print({level, message}); // so all prints leads to the main one
	}

	void Print(const CMDMessage& message) {
		g_scrollOffset = 0;
		g_messages.push_back(message);
	}
}

/*



std::vector<std::string> history; // буфер всех команд
int scrollOffset = 0;              // на сколько строк прокручено вверх
const int maxVisibleLines = 20;    // сколько строк показываем на экране

int startIndex = std::max(0, int(history.size()) - maxVisibleLines - scrollOffset);

for (int i = 0; i < maxVisibleLines; ++i) {
	int idx = startIndex + i;
	if (idx >= history.size()) break;
	RenderText(history[idx], x, y - i * lineHeight);
}





 */
