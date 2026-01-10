#include "Utils.hpp"
#include <chrono>
#include <fstream>
#include <ranges>
#include "Core/Logger.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Utils {
    std::string Trim(const std::string &s) {
        const auto start = std::ranges::find_if_not(s, ::isspace);
        const auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
        return (start < end ? std::string(start, end) : std::string());
    }

    std::vector<float> ParseFloatList(const std::string &str) {
        std::vector<float> vals;
        std::string clean;
        clean.reserve(str.size());
        for (const char c : str) clean += (c == ',' ? ' ' : c);

        std::istringstream ss(clean);
        float v;
        while (ss >> v) vals.push_back(v);

        return vals;
    }

    template <typename T> std::optional<T> TryParse(const std::string &str) {
        return std::nullopt;
    }

    template <> std::optional<int> TryParse<int>(const std::string &str) {
        try {
            return std::stoi(Trim(str));
        } catch (...) { return std::nullopt; }
    }

    template <> std::optional<float> TryParse<float>(const std::string &str) {
        try {
            return std::stof(Trim(str));
        } catch (...) { return std::nullopt; }
    }

    template <> std::optional<bool> TryParse<bool>(const std::string &str) {
        const std::string trimstr = Trim(str);
        if (trimstr == "true" || trimstr == "1") return true;
        if (trimstr == "false" || trimstr == "0") return false;
        return std::nullopt;
    }

    template <> std::optional<std::string> TryParse<std::string>(const std::string &str) {
        return str;
    }

    template <> std::optional<std::array<float, 2>> TryParse<std::array<float, 2>>(const std::string &str) {
        auto vals = ParseFloatList(str);
        if (vals.size() != 2) return std::nullopt;
        return std::array{vals[0], vals[1]};
    }

    template <> std::optional<std::array<float, 3>> TryParse<std::array<float, 3>>(const std::string &str) {
        auto vals = ParseFloatList(str);
        if (vals.size() != 3) return std::nullopt;
        return std::array{vals[0], vals[1], vals[2]};
    }

    template <> std::optional<std::array<float, 4>> TryParse<std::array<float, 4>>(const std::string &str) {
        const auto vals = ParseFloatList(str);
        if (vals.size() != 4) return std::nullopt;
        return std::array{vals[0], vals[1], vals[2], vals[3]};
    }

    Renderer::MeshPtr CreatePlaneMesh(const std::string &name, float size, float tile) {
        auto mesh = std::make_shared<Renderer::Mesh>(name);
        mesh->material = std::make_shared<Renderer::Material>(name);
        mesh->name = name;
        mesh->vertices = {
            {{-size, 0.0f, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1, 0, 0}},
            {{size, 0.0f, -size}, {0.0f, 1.0f, 0.0f}, {tile, 0.0f}, {1, 0, 0}},
            {{size, 0.0f, size}, {0.0f, 1.0f, 0.0f}, {tile, tile}, {1, 0, 0}},
            {{-size, 0.0f, size}, {0.0f, 1.0f, 0.0f}, {0.0f, tile}, {1, 0, 0}}};
        mesh->indices = {0, 2, 1, 0, 3, 2};
        mesh->setup();
        return mesh;
    }

    Renderer::MeshPtr CreateCubeMesh(const std::string &name) {
        auto mesh = std::make_shared<Renderer::Mesh>(name);
        mesh->material = std::make_shared<Renderer::Material>(name);

        mesh->vertices = {
            // Front face (+Z)
            {{-1, -1, 1}, {0, 0, 1}, {0, 0}},
            {{1, -1, 1}, {0, 0, 1}, {1, 0}},
            {{1, 1, 1}, {0, 0, 1}, {1, 1}},
            {{-1, 1, 1}, {0, 0, 1}, {0, 1}},

            // Back face (-Z)
            {{1, -1, -1}, {0, 0, -1}, {0, 0}},
            {{-1, -1, -1}, {0, 0, -1}, {1, 0}},
            {{-1, 1, -1}, {0, 0, -1}, {1, 1}},
            {{1, 1, -1}, {0, 0, -1}, {0, 1}},

            // Left face (-X)
            {{-1, -1, -1}, {-1, 0, 0}, {0, 0}},
            {{-1, -1, 1}, {-1, 0, 0}, {1, 0}},
            {{-1, 1, 1}, {-1, 0, 0}, {1, 1}},
            {{-1, 1, -1}, {-1, 0, 0}, {0, 1}},

            // Right face (+X)
            {{1, -1, 1}, {1, 0, 0}, {0, 0}},
            {{1, -1, -1}, {1, 0, 0}, {1, 0}},
            {{1, 1, -1}, {1, 0, 0}, {1, 1}},
            {{1, 1, 1}, {1, 0, 0}, {0, 1}},

            // Top face (+Y)
            {{-1, 1, 1}, {0, 1, 0}, {0, 0}},
            {{1, 1, 1}, {0, 1, 0}, {1, 0}},
            {{1, 1, -1}, {0, 1, 0}, {1, 1}},
            {{-1, 1, -1}, {0, 1, 0}, {0, 1}},

            // Bottom face (-Y)
            {{-1, -1, -1}, {0, -1, 0}, {0, 0}},
            {{1, -1, -1}, {0, -1, 0}, {1, 0}},
            {{1, -1, 1}, {0, -1, 0}, {1, 1}},
            {{-1, -1, 1}, {0, -1, 0}, {0, 1}},
        };

        mesh->indices = {
            0,  1,  2,  2,  3,  0,  // Front (+Z)
            4,  5,  6,  6,  7,  4,  // Back (-Z)
            8,  9,  10, 10, 11, 8,  // Left (-X)
            12, 13, 14, 14, 15, 12, // Right (+X)
            16, 17, 18, 18, 19, 16, // Top (+Y)
            20, 21, 22, 22, 23, 20  // Bottom (-Y)
        };
        mesh->setup();
        return mesh;
    }

    Renderer::MeshPtr
    CreateSphereMesh(const std::string &name, float radius, uint32_t stackCount, uint32_t sliceCount) {
        auto mesh = std::make_shared<Renderer::Mesh>(name);
        mesh->material = std::make_shared<Renderer::Material>(name);

        mesh->vertices.push_back({
            glm::vec3(0.0f, radius, 0.0f), // position
            glm::vec3(0.0f, 1.0f, 0.0f),   // normal
            glm::vec2(0.5f, 0.0f)          // tex_coords
        });

        for (uint32_t stack = 1; stack < stackCount; ++stack) {
            float theta = stack * glm::pi<float>() / stackCount;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (uint32_t slice = 0; slice <= sliceCount; ++slice) {
                float phi = slice * 2.0f * glm::pi<float>() / sliceCount;
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                Renderer::Vertex v;
                v.pos = glm::vec3(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
                v.normal = glm::normalize(v.pos);
                v.texCoords = glm::vec2(static_cast<float>(slice) / sliceCount, static_cast<float>(stack) / stackCount);

                mesh->vertices.push_back(v);
            }
        }

        mesh->vertices.push_back({
            glm::vec3(0.0f, -radius, 0.0f), // position
            glm::vec3(0.0f, -1.0f, 0.0f),   // normal
            glm::vec2(0.5f, 1.0f)           // tex_coords
        });

        const uint32_t poleStart = 0;
        const uint32_t ringVertexCount = sliceCount + 1;

        for (uint32_t slice = 0; slice < sliceCount; ++slice) {
            mesh->indices.push_back(poleStart);
            mesh->indices.push_back(1 + (slice + 1) % sliceCount);
            mesh->indices.push_back(1 + slice);
        }

        for (uint32_t stack = 0; stack < stackCount - 2; ++stack) {
            uint32_t ringStart = 1 + stack * ringVertexCount;
            uint32_t nextRingStart = ringStart + ringVertexCount;

            for (uint32_t slice = 0; slice < sliceCount; ++slice) {
                mesh->indices.push_back(ringStart + slice);
                mesh->indices.push_back(ringStart + slice + 1);
                mesh->indices.push_back(nextRingStart + slice);

                mesh->indices.push_back(nextRingStart + slice);
                mesh->indices.push_back(ringStart + slice + 1);
                mesh->indices.push_back(nextRingStart + slice + 1);
            }
        }

        const uint32_t bottomPoleIndex = static_cast<uint32_t>(mesh->vertices.size() - 1);
        const uint32_t lastRingStart = 1 + (stackCount - 2) * ringVertexCount;

        for (uint32_t slice = 0; slice < sliceCount; ++slice) {
            mesh->indices.push_back(bottomPoleIndex);
            mesh->indices.push_back(lastRingStart + slice);
            mesh->indices.push_back(lastRingStart + slice + 1);
        }

        mesh->setup();
        return mesh;
    }

    std::string ReadFromFile(const std::string &path) {
        std::string content;
        std::ifstream fileStream(path, std::ios::in);
        if (!fileStream.is_open()) {
            Log::Error("Could`nt open file " + path);
            return "";
        }
        std::string line;
        while (!fileStream.eof()) {
            std::getline(fileStream, line);
            content.append(line + "\n");
        }
        fileStream.close();
        return content;
    }

    void SaveAsPng(const std::string &path, std::vector<uint8_t> &pixels, uint16_t w, uint16_t h, uint16_t channels) {
        stbi_write_png(path.c_str(), w, h, channels, pixels.data(), w * channels);
    }
} // namespace Utils
