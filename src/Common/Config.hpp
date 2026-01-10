#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Config {
    // -- NO PREFIX ---
    float sensitivity;

    // --- SYSTEM ---
    glm::vec2 windowResolution;

    bool autoExposure;
    float autoExposureSpeed;

    // --- CONSOLE ---
    float consoleFontScale;
    int consoleLines;
    glm::vec4 consoleColor;
};

extern Config g_config;
