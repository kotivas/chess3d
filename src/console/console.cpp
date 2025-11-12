#include "console.hpp"

#include <format>
#include <ranges>
#include <sstream>

#include "com/config.hpp"
#include "core/backend.hpp"
#include "core/cmdutils.hpp"
#include "core/cvar.hpp"
#include "core/logger.hpp"
#include "input/input.hpp"
#include "render/renderer.hpp"
#include "resourcemgr/resourcemgr.hpp"
#include "text/MSDFText.hpp"

namespace Console {
	int g_scrollOffset = 0;
	int g_historyIndex = -1;
	int g_cursorIndent = 0;
	std::string g_inputField;
	std::string g_suggestion;
	std::vector<CMDLine> g_messages;
	std::deque<std::string> g_history;
	bool g_isVisible = false;
	bool g_blinked = false;
	double g_blinkTimer = 0.f;

	void Toggle() {
		g_isVisible = !g_isVisible;
	}

	bool IsVisible() {
		return g_isVisible;
	}

	void Init() {
		// Print(Color::CYAN, "it is what it is");
	}

	void ExecuteCommand(const std::string& command) {
		if (command.empty())
			return;

		std::istringstream iss(command);
		std::string name;

		Print(Color::WHITE, "> " + command);

		// Parse command name and optional value
		if (!(iss >> name)) {
			Log::Error("invalid command syntax");
			return;
		}

		// Handle built-in commands
		if (name == "clear") {
			g_messages.clear();
			return;
		}
		if (name == "help") {
			Print(Color::WHITE, "help: Show all avaliable commands");
			Print(Color::WHITE, "clear: Clear console history");
			Print(Color::WHITE, "status: Get resource usage for current process");
			for (const auto& val : CVar::g_cvars | std::views::values) {
				Print(Color::WHITE, val.name + ": " + val.desc);
			}
			return;
		}

		if (name == "status") {
			Print(Color::WHITE,
			      "VirtMem usage (MB): " + std::to_string(float(Backend::VirtMemoryUsage()) / 8.f / 1024.f / 1024.f));
			Print(Color::WHITE, "CPU usage (%) " + std::to_string(Backend::CpuUsage()));

			return;
		}

		auto* cvar = CMDUtils::Find(name);
		if (!cvar) {
			Log::Warning("Unknown command <" + name + ">");
			return;
		}

		if (std::string value_str; !(iss >> value_str)) {
			Print(Color::WHITE,
			      std::format("{0} = {1} \n - {2} (def <{3}>, min <{4}>, max <{5}>)", name,
			                  CMDUtils::ToString(cvar->val), cvar->desc,
			                  CMDUtils::ToString(cvar->defVal), cvar->minFloat, cvar->maxFloat));
			return;
		}

		CMDUtils::Execute(name, command.substr(command.find_first_of(' ') + 1));
	}


	void Update(double dt) {
		if (!g_isVisible) return;

		g_blinkTimer += dt;
		if (g_blinkTimer >= BLINK_INTERVAL) {
			g_blinked = !g_blinked;
			g_blinkTimer -= BLINK_INTERVAL;
		}

		if (Input::GetScrollYOffset() != 0.f) {
			g_scrollOffset += Input::GetScrollYOffset() < 0 ? -1 : 1;
		}

		if (!g_history.empty()) {
			if (Input::IsKeyPressed(Key::Up)) NavigateHistory(+1);
			else if (Input::IsKeyPressed(Key::Down)) NavigateHistory(-1);
		}

		if (Input::IsKeyPressed(Key::Right))
			g_cursorIndent = std::max(0, g_cursorIndent - 1);
		else if (Input::IsKeyPressed(Key::Left))
			g_cursorIndent = std::min(
				int(g_inputField.size()), g_cursorIndent + 1);


		if (!Input::GetTextBuffer().empty()) {
			g_inputField.insert(g_inputField.size() - g_cursorIndent, Input::GetTextBuffer());
		}


		if (!g_inputField.empty()) {
			if (Input::IsKeyPressed(Key::Backspace)) HandleBackspace();

			if (Input::IsKeyPressed(Key::Enter)) {
				ExecuteCommand(g_inputField);
				g_history.push_front(g_inputField);
				g_historyIndex = -1;
				g_cursorIndent = 0;
				g_inputField.clear();
				g_suggestion.clear();
			}
		}
	}

	void NavigateHistory(int direction) {
		if (g_history.empty()) return;

		g_historyIndex = std::clamp(g_historyIndex + direction, -1, static_cast<int>(g_history.size()) - 1);
		g_inputField = (g_historyIndex >= 0) ? g_history[g_historyIndex] : "";
	}

	void HandleBackspace() {
		if (g_inputField.empty()) return;

		g_cursorIndent = std::clamp(g_cursorIndent, 0, static_cast<int>(g_inputField.size()));
		int erasePos = static_cast<int>(g_inputField.size()) - g_cursorIndent - 1;

		if (erasePos >= 0 && erasePos < static_cast<int>(g_inputField.size()))
			g_inputField.erase(erasePos, 1);
		else if (g_cursorIndent == 0)
			g_inputField.pop_back();
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

		const float lineHeight = font->lineHeight * fontScale;
		const float lineyzero = g_config.r_resolution.y - lineHeight;
		const float leftIndent = 5;
		const float maxHeight = lineyzero - maxVisibleLines * lineHeight;
		const float maxWidth = g_config.r_resolution.x - leftIndent;

		Renderer::DrawRectOnScreen(0, 0, maxWidth + leftIndent, maxHeight, g_config.con_backgroundColor);
		Renderer::DrawRectOnScreen(0, maxHeight, maxWidth + leftIndent, 1, {Color::YELLOW, 1});

		// STER 0 --------------------------------- draw input field
		MSDFText::DrawText("> " + g_inputField, font, leftIndent, maxHeight + font->descender * fontScale, fontScale,
		                   {1, 1, 1, 1});

		if (!g_blinked) {
			float x = leftIndent;
			std::string calcs = "> " + g_inputField;
			calcs.erase(calcs.length() - g_cursorIndent, g_cursorIndent);
			for (const char& cc : calcs) x += font->getGlyph(cc).advance * fontScale;

			Renderer::DrawRectOnScreen(x, maxHeight - font->lineHeight * fontScale, 1, lineHeight / 2,
			                           {Color::WHITE, 1});
		}

		if (g_messages.empty()) return;
		// STEP 1 --------------------------------- get visible messages vector
		std::vector<CMDLine> messages;

		int total = static_cast<int>(g_messages.size());
		int maxScroll = std::max(0, total - maxVisibleLines);
		g_scrollOffset = std::clamp(g_scrollOffset, 0, maxScroll);

		int end = total - g_scrollOffset; // newest msgs
		int start = std::max(0, end - maxVisibleLines); // start

		messages.assign(g_messages.begin() + start, g_messages.begin() + end);
		// STEP 2 --------------------------------- parse line boxes
		std::vector<CMDLine> line_boxes;

		for (const auto& msg : messages) {
			const int maxChar = MSDFText::GetMaxCharactersForWidth(msg.text, font, fontScale, maxWidth);

			if (msg.text.length() > maxChar) {
				// todo multiple lines
				line_boxes.emplace_back(msg.color, msg.text.substr(0, maxChar));
				line_boxes.emplace_back(msg.color, msg.text.substr(maxChar));
			} else if (msg.text.find('\n') != std::string::npos) {
				line_boxes.emplace_back(msg.color, msg.text.substr(0, msg.text.find('\n')));
				line_boxes.emplace_back(msg.color, msg.text.substr(msg.text.find('\n') + 1));
			} else {
				line_boxes.emplace_back(msg.color, msg.text);
			}
		}

		if (line_boxes.size() > maxVisibleLines) {
			line_boxes.erase(line_boxes.begin(), line_boxes.begin() + (line_boxes.size() - maxVisibleLines));
		}

		// STEP 3 --------------------------------- rendering text

		for (int i = 0; i < line_boxes.size(); i++) {
			MSDFText::DrawText(line_boxes[i].text, font, leftIndent, lineyzero - i * lineHeight, fontScale,
			                   {line_boxes[i].color, 1});
		}
	}

	void Print(const Color::rgb_t& color, const std::string& message) {
		Print({color, message}); // so all prints leads to the main one
	}

	void Print(const Logger::Severity sev, const std::string& message) {
		Color::rgb_t color;

		switch (sev) {
		case Logger::Severity::Info: color = Color::WHITE;
			break;
		case Logger::Severity::Debug: color = Color::ORANGE;
			break;
		case Logger::Severity::Warning: color = Color::YELLOW;
			break;
		case Logger::Severity::Error: color = Color::RED;
			break;
		case Logger::Severity::Fatal: color = Color::MAROON;
			break;
		}

		Print({color, message});
	}

	void Print(const CMDLine& message) {
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
