#pragma once
#include <deque>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

#include "Core/Logger.hpp"

namespace Console {
    constexpr double BLINK_INTERVAL = 0.5;

    struct CMDLine {
        CMDLine(const glm::vec3 color, const std::string &text) : color(color), text(text) {}

        glm::vec3 color;
        std::string text;
    };

    extern int g_scrollOffset;
    extern bool g_isVisible;
    extern std::vector<CMDLine> g_messages;
    extern std::deque<std::string> g_history;
    extern int g_historyIndex;
    extern std::string g_inputField;
    extern int g_cursorIndent;
    extern std::string g_suggestion;

    void Init();
    void Update(double dt);

    void Draw();
    // void DrawDebugInfo();

    bool IsVisible();

    void Toggle();

    void NavigateHistory(int direction);
    void HandleBackspace();

    void ExecuteCommand(const std::string &command);

    void Print(const glm::vec3 &color, const std::string &message);
    void Print(Logger::Severity sev, const std::string &message);
    void Print(const CMDLine &message);
} // namespace Console
