#pragma once
#include <cstdint>
#include <glad/glad.h>

namespace Renderer {
    enum class TextureType : uint16_t {
        Tex2D = GL_TEXTURE_2D,
        Tex3D = GL_TEXTURE_3D,
        Tex1D = GL_TEXTURE_1D,
        TexCube = GL_TEXTURE_CUBE_MAP,
        TexBuffer = GL_TEXTURE_BUFFER,
        Tex2DArray = GL_TEXTURE_2D_ARRAY,
        TexCubeArray = GL_TEXTURE_CUBE_MAP_ARRAY,
        Tex1DArray = GL_TEXTURE_1D_ARRAY,
    };

    enum class TextureWrapMode : uint16_t {
        Repeat = GL_REPEAT,
        MirrorerRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER
    };
} // namespace Renderer
