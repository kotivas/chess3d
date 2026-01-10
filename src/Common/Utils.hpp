#pragma once
#include <optional>
#include <sstream>
#include <string>
#include "Renderer/Model.hpp"
#include "ResourceMgr/ResourceMgr.hpp"

namespace Utils {

    constexpr glm::vec3 BLACK{0.0f, 0.0f, 0.0f};
    constexpr glm::vec3 WHITE{1.0f, 1.0f, 1.0f};
    constexpr glm::vec3 RED{1.0f, 0.0f, 0.0f};
    constexpr glm::vec3 LIME{0.0f, 1.0f, 0.0f};
    constexpr glm::vec3 BLUE{0.0f, 0.0f, 1.0f};
    constexpr glm::vec3 YELLOW{1.0f, 1.0f, 0.0f};
    constexpr glm::vec3 MAGENTA{1.0f, 0.0f, 1.0f};
    constexpr glm::vec3 CYAN{0.0f, 1.0f, 1.0f};
    constexpr glm::vec3 ORANGE{1.0f, 0.65f, 0.0f};
    constexpr glm::vec3 GREEN{0.0f, 0.5f, 0.0f};
    constexpr glm::vec3 GRAY{0.5f, 0.5f, 0.5f};
    constexpr glm::vec3 SILVER{0.75f, 0.75f, 0.75f};
    constexpr glm::vec3 NAVY{0.0f, 0.0f, 0.5f};
    constexpr glm::vec3 PURPLE{0.5f, 0.0f, 0.5f};
    constexpr glm::vec3 TEAL{0.0f, 0.5f, 0.5f};
    constexpr glm::vec3 MAROON{0.5f, 0.0f, 0.0f};
    constexpr glm::vec3 LIGHT_RED{1.0f, 0.4f, 0.4f};

    template <typename T, size_t N> std::string ArrayToString(const std::array<T, N> &arr) {
        std::ostringstream oss;
        for (size_t i = 0; i < N; ++i) {
            oss << arr[i];
            if (i != N - 1) oss << ", ";
        }
        return oss.str();
    }

    std::string Trim(const std::string &s);
    std::vector<float> ParseFloatList(const std::string &str);
    std::string ReadFromFile(const std::string &path);
    Renderer::MeshPtr CreateSphereMesh(const std::string &name, float radius, uint32_t stackCount, uint32_t sliceCount);
    Renderer::MeshPtr CreateCubeMesh(const std::string &name);
    Renderer::MeshPtr CreatePlaneMesh(const std::string &name, float size, float tile);
    void SaveAsPng(const std::string &path, std::vector<uint8_t> &pixels, uint16_t w, uint16_t h, uint16_t channels);

    template <typename T> std::optional<T> TryParse(const std::string &str);
} // namespace Utils
