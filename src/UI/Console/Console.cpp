#include "Console.hpp"

#include <format>
#include <ranges>
#include <sstream>

#include "AssetManager/AssetManager.hpp"
#include "Common/Config.hpp"
#include "Common/Utils.hpp"
#include "Core/Backend.hpp"
#include "Core/CMDUtils.hpp"
#include "Core/CVar.hpp"
#include "Core/Logger.hpp"
#include "Input/Input.hpp"
#include "Renderer/Renderer.hpp"
#include "ResourceMgr/ResourceMgr.hpp"
#include "UI/Text/MSDFText.hpp"

namespace Console {
    int g_scrollOffset = 0;
    int g_historyIndex = -1;
    int g_cursorIndent = 0;
    std::string g_inputField;
    std::string g_suggestion;
    std::vector<CMDLine> g_messages;
    std::deque<std::string> g_history;
    bool g_isVisible = false;

    void Toggle() {
        g_isVisible = !g_isVisible;
    }

    bool IsVisible() {
        return g_isVisible;
    }

    void Init() {
        // Print(Color::CYAN, "it is what it is");
    }

    void ExecuteCommand(const std::string &command) {
        if (command.empty()) return;
        Print(Utils::WHITE, "> " + command);

        std::istringstream iss(command);
        std::string name;
        std::string value;

        iss >> name;
        iss >> value;

        // Parse command name and optional value
        if (name.empty()) {
            Log::Error("invalid command syntax");
            return;
        }

        // Handle built-in commands
        if (name == "clear") {
            g_messages.clear();
            return;
        }
        if (name == "help") {
            Print(Utils::WHITE, "help: Show all avaliable commands");
            Print(Utils::WHITE, "clear: Clear console history");
            Print(Utils::WHITE, "status: Get resource usage for current process");
            for (const auto &val : CVar::g_cvars | std::views::values) Print(Utils::WHITE, val.name + ": " + val.desc);
            return;
        }
        if (name == "status") {
            Print(
                Utils::WHITE,
                "VirtMem usage (MB): " + std::to_string(float(Backend::VirtMemoryUsage()) / 8.f / 1024.f / 1024.f));
            Print(Utils::WHITE, "CPU usage (%) " + std::to_string(Backend::CpuUsage()));

            return;
        }
        if (name == "reload_shader") {
            if (!value.empty())
                AssetManager::g_shaderManager.reload(value);
            else
                Log::Warning("reload_shader <shader name>");
            return;
        }


        auto *cvar = CMDUtils::Find(name);
        if (!cvar) {
            Log::Warning("Unknown command <" + name + ">");
            return;
        }

        if (value.empty()) {
            Print(
                Utils::WHITE,
                std::format(
                    "{0} = {1} \n - {2} (def <{3}>, min <{4}>, max <{5}>)", name, CMDUtils::ToString(cvar->val),
                    cvar->desc, CMDUtils::ToString(cvar->defVal), cvar->minFloat, cvar->maxFloat));
            return;
        }

        CMDUtils::Execute(name, command.substr(command.find_first_of(' ') + 1));
    }


    void Update(double dt) {
        if (!g_isVisible) return;

        if (Input::GetScrollYOffset() != 0.f) g_scrollOffset += Input::GetScrollYOffset() < 0 ? -1 : 1;

        if (!g_history.empty()) {
            if (Input::IsKeyPressed(Key::Up))
                NavigateHistory(+1);
            else if (Input::IsKeyPressed(Key::Down))
                NavigateHistory(-1);
        }

        if (Input::IsKeyPressed(Key::Right))
            g_cursorIndent = std::max(0, g_cursorIndent - 1);
        else if (Input::IsKeyPressed(Key::Left))
            g_cursorIndent = std::min(int(g_inputField.size()), g_cursorIndent + 1);


        if (!Input::GetTextBuffer().empty() && Input::GetTextBuffer().back() != '`')
            g_inputField.insert(g_inputField.size() - g_cursorIndent, Input::GetTextBuffer());


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

    void Draw() { // FIXME IT'S NOT SUPPOSED TO BE HERE
        if (!g_isVisible) return;

        const MsdfText::FontPtr font = ResourceMgr::GetFontByName("inconsolata_light");
        if (!font) {
            Log::Error("UNABLE TO GET CONSOLE FONT");
            return;
        }

        const float &fontScale = g_config.consoleFontScale;
        const int &maxVisibleLines = g_config.consoleLines;
        const float lineHeight = font->lineHeight * fontScale;
        const float lineyzero = Renderer::settings.renderResolution.y - lineHeight;

        const float leftIndent = 5;
        const float height = lineHeight * (maxVisibleLines + 1) - (font->descender * fontScale) * 2;
        const float width = Renderer::settings.renderResolution.x - leftIndent;

        Renderer::DrawRectOnScreen(0, 0, width + leftIndent, height, g_config.consoleColor);
        Renderer::DrawRectOnScreen(0, height, width + leftIndent, 1, {1, 1, 0, 1});

        Renderer::DrawText(
            {"> " + g_inputField,
             fontScale,
             {leftIndent, lineyzero - maxVisibleLines * lineHeight},
             {Utils::WHITE, 1},
             font});

        if (g_messages.empty()) return;
        // STEP 1 --------------------------------- get visible messages vector
        std::vector<CMDLine> messages;

        int total = static_cast<int>(g_messages.size());
        int maxScroll = std::max(0, total - maxVisibleLines);
        g_scrollOffset = std::clamp(g_scrollOffset, 0, maxScroll);

        int end = total - g_scrollOffset;               // newest msgs
        int start = std::max(0, end - maxVisibleLines); // start

        messages.assign(g_messages.begin() + start, g_messages.begin() + end);
        // STEP 2 --------------------------------- parse line boxes
        std::vector<CMDLine> line_boxes;

        for (const auto &msg : messages) {
            const int maxChar = MsdfText::GetMaxCharactersForWidth(msg.text, font, fontScale, width);
            std::string str = msg.text;

            int i = 0;
            while (str.length() > maxChar) {
                line_boxes.emplace_back(msg.color, msg.text.substr(maxChar * i, maxChar));
                str = str.substr(maxChar);
                i++;
            }
            line_boxes.emplace_back(msg.color, str);
        }

        if (line_boxes.size() > maxVisibleLines)
            line_boxes.erase(line_boxes.begin(), line_boxes.begin() + (line_boxes.size() - maxVisibleLines));

        // STEP 3 --------------------------------- rendering text

        for (int i = 0; i < line_boxes.size(); i++) {
            Renderer::DrawText(
                {line_boxes[i].text,
                 fontScale,
                 {leftIndent, lineyzero - i * lineHeight},
                 {line_boxes[i].color, 1},
                 font});
        }
    }

    void Print(const glm::vec3 &color, const std::string &message) {
        Print({color, message}); // so all prints leads to the main one
    }

    void Print(const Logger::Severity sev, const std::string &message) {
        glm::vec3 color;

        switch (sev) {
        case Logger::Severity::Info:
            color = Utils::WHITE;
            break;
        case Logger::Severity::Debug:
            color = Utils::ORANGE;
            break;
        case Logger::Severity::Warning:
            color = Utils::YELLOW;
            break;
        case Logger::Severity::Error:
            color = Utils::RED;
            break;
        case Logger::Severity::Fatal:
            color = Utils::MAROON;
            break;
        }

        Print({color, message});
    }

    void Print(const CMDLine &message) {
        g_scrollOffset = 0;
        g_messages.push_back(message);
    }
} // namespace Console
