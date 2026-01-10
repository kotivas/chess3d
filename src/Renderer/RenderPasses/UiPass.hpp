#pragma once
#include "Renderer/RenderContext.hpp"

// todo Pass interface
namespace Renderer {
    class UiPass {
    public:
        UiPass();

        void init();
        void pass(RenderContext &ctx);

    private:
        // drawText
        // drawTexture
        // drawRectangle
        // drawLine
        // Alligment enum class
    };
} // namespace Renderer
