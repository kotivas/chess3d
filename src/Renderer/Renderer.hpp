#pragma once

#include <glm/glm.hpp>

#include "../Game/Scene.hpp"
#include "PostEffects/GaussianBlur.hpp"
#include "RenderPasses/LightningPass.hpp"
#include "RenderPasses/ShadowPass.hpp"
#include <GLFW/glfw3.h>
#include "ResourceMgr/ResourceMgr.hpp"
#include "Sky.hpp"

namespace Renderer {
    void Init();
    void GlInit();
    bool SetGlDebugCallback(GLDEBUGPROC fn);

    void Render(const Scene &scene, double dt);
    void ApplyPostProcess(double dt);

    void UpdateRenderRes();

    void DrawText(const MsdfText::Text &text);
    void DrawSky(const SkyPtr &sky, const Camera::CameraInfo &cam, float time);
    void DrawTextureOnScreen(uint32_t texture, float x, float y, float w, float h);
    void DrawRectOnScreen(float x, float y, float w, float h, const glm::vec4 &color);
    void DrawDebug(float scale, float time, glm::vec3 cam_pos);

    void GenerateRenderItem(const std::shared_ptr<Drawable> &drawable, std::vector<RenderItem> &items);

    void RenderClear();

    void CreateSceneFbo();
    void CreateScreenFBO();
    void CreateQuadVao();

    float FrameAvgLuminance();

    void Shutdown();

    static uint32_t sceneFBO, RBO;

    static PostEffects::GaussianBlur blur;

    static uint32_t quadVAO, quadVBO;
    static uint32_t sceneColorBufs[2];

    static int g_fps = 0.f;
    static double g_fpsAccum = 0.0f;

    static uint32_t screenFBO;
    static uint32_t screenColorBuf;

    static ShadowPass shadowPass;
    static LightningPass lightPass;

    static AssetManager::ShaderHandle quadShader;
    static AssetManager::ShaderHandle solidShader;
    static AssetManager::ShaderHandle textureShader;
    static AssetManager::ShaderHandle postfxShader;
    static AssetManager::ShaderHandle msdfTextShader;

    inline RenderSettings settings;
    inline GLFWwindow *g_window = nullptr;
}; // namespace Renderer
