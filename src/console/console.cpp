#include "console.hpp"

#include "game/config.hpp"
#include "input/input.hpp"
#include "render/renderer.hpp"
#include "core/logger.hpp"
#include "text/MSDFText.hpp"
#include "resourcemgr/resourcemgr.hpp"

namespace Console {
	std::vector<CMDMessage> g_messages;
	int scrollOffset = 0;
	bool g_isVisible = false;;

	void Toggle() {
		g_isVisible = !g_isVisible;
	}

	void Init() {
		Print(Info, "Console inited");
	}

	void Update() {
		if (Input::GetScrollYOffset()) {
			scrollOffset += Input::GetScrollYOffset() < 0 ? -1 : 1;
		}
	}

	// TODO REFACTOR THIS
	void Draw() {
		if (!g_isVisible) return;

		const MSDFText::FontPtr font = ResourceMgr::GetFontByName("inconsolata_light");
		if (!font) {
			Log::Error("UNABLE TO GET CONSOLE FONT");
			return;
		}

		Renderer::DrawRectOnScreen(0, 0, g_config.renderRes.x, MESSAGE_PER_PAGE * FONT_SCALE * font->lineHeight, {0, 0, 0, 1});
		// maybe put all of this function insde of this namespace


		const int totalLines = static_cast<int>(g_messages.size());
		const int maxOffset = std::max(0, totalLines - MESSAGE_PER_PAGE);
		scrollOffset = std::clamp(scrollOffset, 0, maxOffset);
		int startIndex = totalLines - MESSAGE_PER_PAGE - scrollOffset;
		for (int i = 0; i < MESSAGE_PER_PAGE; ++i) {
			int idx = startIndex < 0 ? i : startIndex + i;
			if (idx >= totalLines) break;

			glm::vec3 msg_color;
			std::string out_text;
			switch (g_messages[idx].severity) {
			case SeverityLevel::Critical:
				msg_color = {0.47, 0, 0};
				out_text = "[CRIT] " + g_messages[idx].text;
				break;
			case SeverityLevel::Error:
				msg_color = {0.86, 0, 0};
				out_text = "[ERROR] " + g_messages[idx].text;
				break;
			case SeverityLevel::Warning:
				msg_color = {1, 0.54, 0};
				out_text = "[WARN] " + g_messages[idx].text;
				break;
			case SeverityLevel::Info:
				msg_color = {1, 1, 1};
				out_text = "[INFO] " + g_messages[idx].text;
				break;
			case SeverityLevel::Debug:
				msg_color = {1, 0.77, 0};
				out_text = "[DEBUG] " + g_messages[idx].text;
				break;
			}

			MSDFText::DrawText(out_text, font, 0,
			                   g_config.renderRes.y - FONT_SCALE * 1.5 - font->lineHeight * i * FONT_SCALE,
			                   FONT_SCALE, {msg_color, 1});
		}

		// int startIndex = std::max(0, int(g_messages.size()) - MESSAGE_PER_PAGE - scrollOffset);
		// for (int i = scrollOffset; i < MESSAGE_PER_PAGE; ++i) {
		// 	int idx = startIndex + i;
		// 	if (idx >= g_messages.size()) break;
		//
		// 	MSDFText::DrawText(g_messages[idx].text, font, 0, screenyzero - font->lineHeight * i * FONT_SCALE,
		// 	                   FONT_SCALE, {1, 1, 1, 1});
		// }
	}

	void Print(SeverityLevel level, const std::string& message) {
		Print({level, message}); // so all prints leads to the main one
	}

	void Print(const CMDMessage& message) {
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
