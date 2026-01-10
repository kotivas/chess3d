#pragma once
#include <array>


#include "AssetManager/Handle.hpp"
#include "Model.hpp"
#include "Shader.hpp"

namespace Renderer {
    glm::vec3 HosekWilkie(float cos_theta, float gamma, float cos_gamma, const std::array<glm::vec3, 10> &params);

    struct Sky {
        void setup();

        std::vector<Vertex> vertices;
        uint32_t vao, vbo;
        AssetManager::ShaderHandle shader;

        glm::vec3 sunDirection;
        glm::vec3 sunColor;
        float atmTurbidity;
        std::array<glm::vec3, 10> atmParams;

        void calculateSun(const glm::vec2 &sun_pos);

        ~Sky();
    };

    using SkyPtr = std::shared_ptr<Sky>;
} // namespace Renderer
