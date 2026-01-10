#pragma once
#include <cstdint>

namespace Renderer::PostEffects {
    class GaussianBlur {
    public:
        GaussianBlur() = default;

        void init(int width, int height, uint32_t quad_vao, uint32_t quad_vbo);
        uint32_t blur(const uint32_t &input, int passes) const;

        ~GaussianBlur();

    private:
        unsigned int _pingpongFbo[2];
        unsigned int _pingpongBuffers[2];
        uint32_t _quadVao;
        uint32_t _quadVbo;
    };
} // namespace Renderer::PostEffects
