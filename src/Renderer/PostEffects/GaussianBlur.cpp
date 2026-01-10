#include "GaussianBlur.hpp"

#include "Core/Logger.hpp"
#include "ResourceMgr/ResourceMgr.hpp"

namespace Renderer::PostEffects {
    void GaussianBlur::init(int width, int height, uint32_t quad_vao, uint32_t quad_vbo) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };

        _quadVao = 0;
        _quadVbo = 0;

        // setup plane VAO
        glGenVertexArrays(1, &_quadVao);
        glGenBuffers(1, &_quadVbo);
        glBindVertexArray(_quadVao);
        glBindBuffer(GL_ARRAY_BUFFER, _quadVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

        glGenFramebuffers(2, _pingpongFbo);
        glGenTextures(2, _pingpongBuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, _pingpongFbo[i]);
            glBindTexture(GL_TEXTURE_2D, _pingpongBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _pingpongBuffers[i], 0);
        }
    }

    uint32_t GaussianBlur::blur(const uint32_t &input, int passes) const {
        // bool horizontal = true;
        // ShaderPtr shader = ResourceMgr::GetShaderByName("GaussianBlur");
        // shader->use();
        // for (unsigned int i = 0; i < passes; i++) {
        // 	glBindFramebuffer(GL_FRAMEBUFFER, _pingpongFBO[horizontal]);
        // 	shader->setUniform1i("horizontal", horizontal);
        // 	glBindTexture(GL_TEXTURE_2D, i == 0 ? input : _pingpongBuffers[!horizontal]);
        //
        // 	glBindVertexArray(_quadVAO);
        // 	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        // 	glBindVertexArray(0);
        // 	horizontal = !horizontal;
        // }
        //
        // return _pingpongBuffers[!horizontal];
        return 0;
    }

    GaussianBlur::~GaussianBlur() {
        glDeleteFramebuffers(2, _pingpongFbo);
        glDeleteTextures(2, _pingpongBuffers);
    }
} // namespace Renderer::PostEffects
