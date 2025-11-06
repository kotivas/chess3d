#include "console.hpp"

#include <format>
#include <sstream>

#include "com/config.hpp"
#include "core/cvar.hpp"
#include "core/logger.hpp"
#include "input/input.hpp"
#include "render/renderer.hpp"
#include "resourcemgr/resourcemgr.hpp"
#include "text/MSDFText.hpp"

namespace Console {
	int g_scrollOffset = 0;
	bool g_isVisible = false;
	std::string g_inputField;
	std::vector<CMDLine> g_messages;

	void Toggle() {
		g_isVisible = !g_isVisible;
	}

	bool IsVisible() {
		return g_isVisible;
	}

	void Init() {
		// Print("Console inited");
	}

	void ExecuteCommand(const std::string& command) {
		if (command.empty())
			return;

		std::istringstream iss(command);
		std::string name, value_str;

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
			for (const auto& cvar : CVarSys::g_cvars) {
				Print(Color::WHITE, cvar.name + ": " + cvar.desc);
			}
			return;
		}

		auto* cvar = CVarSys::Find(name);
		if (!cvar) {
			Log::Warning("Unknown command <" + name + ">");
			return;
		}

		// Try to read a value, if any
		if (!(iss >> value_str)) {
			Print(Color::WHITE,
			      std::format("{0} = {1} \n - {2} (def <{3}>, min <{4}>, max <{5}>)", name, cvar->val, cvar->desc,
			                  cvar->defVal, cvar->min, cvar->max));
			return;
		}

		// Try converting to float
		float value;
		try {
			value = std::stof(value_str);
		} catch (const std::exception& e) {
			Print(Color::LIGHT_RED, "Invalid argument");
			return;
		}

		// Execute
		cvar->set(value);
	}


	void Update() {
		if (!g_isVisible) return;

		if (Input::GetScrollYOffset() != 0.f) {
			g_scrollOffset += Input::GetScrollYOffset() < 0 ? -1 : 1;
		}

		g_inputField += Input::GetTextBuffer();

		if (!g_inputField.empty()) {
			if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) && Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
				std::size_t space_index = g_inputField.find_last_of(" ");
				if (space_index == std::string::npos) { g_inputField.clear(); } else {
					g_inputField.erase(space_index);
				}
			} else if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE)) {
				g_inputField.pop_back();
			}

			if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
				// g_messages.push_back({Color::WHITE, g_inputField});
				ExecuteCommand(g_inputField);
				g_inputField.clear();
			}
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

		const float lineHeight = font->lineHeight * fontScale;
		const float lineyzero = g_config.r_resolution.y - lineHeight;
		const float leftIndent = 5;
		const float maxHeight = lineyzero - maxVisibleLines * lineHeight;
		const float maxWidth = g_config.r_resolution.x - leftIndent;

		Renderer::DrawRectOnScreen(0, 0, maxWidth + leftIndent, maxHeight, g_config.con_backgroundColor);
		Renderer::DrawRectOnScreen(0, maxHeight, maxWidth + leftIndent, 1, {0.9, 0.3, 0, 1});

		// STER 0 --------------------------------- draw input field
		MSDFText::DrawText("> " + g_inputField, font, leftIndent, maxHeight + font->descender * fontScale, fontScale,
		                   {1, 1, 1, 1});

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
		std::string output;

		switch (sev) {
		case Logger::Severity::Info:
			color = Color::WHITE;
			output = "INFO: " + message;
			break;
		case Logger::Severity::None:
			color = Color::WHITE;
			break;
		case Logger::Severity::Debug: color = Color::ORANGE;
			output = "DEBUG: " + message;
			break;
		case Logger::Severity::Warning: color = Color::YELLOW;
			output = "WARNING: " + message;
			break;
		case Logger::Severity::Error: color = Color::RED;
			output = "ERROR: " + message;
			break;
		case Logger::Severity::Fatal: color = Color::MAROON;
			output = "FATAL: " + message;
			break;
		}

		Print({color, output});
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
