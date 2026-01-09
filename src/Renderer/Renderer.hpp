#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "PostEffects/GaussianBlur.hpp"
#include "../Game/Scene.hpp"
#include "Sky.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Common/Color.hpp"
#include "RenderPasses/LightningPass.hpp"
#include "RenderPasses/ShadowPass.hpp"
#include "ResourceMgr/ResourceMgr.hpp"

namespace Renderer {
	void Init();
	void GlInit();
	bool SetGlDebugCallback(GLDEBUGPROC fn);

	void Render(Scene& scene, double dt);
	void ApplyPostProcess(double dt);

	void UpdateRenderRes();

	void DrawText(const MSDFText::Text& text);
	void DrawSky(SkyPtr sky, const Camera::CameraInfo& cam, float time);
	void DrawTextureOnScreen(uint32_t texture, float x, float y, float w, float h);
	void DrawRectOnScreen(float x, float y, float w, float h, const glm::vec4& color);
	void DrawDebug(int fps, float scale, float time, glm::vec3 cam_pos);

	void GenerateRenderItem(const std::shared_ptr<Drawable>& drawable, std::vector<RenderItem>& items);

	void RenderClear();

	void CreateSceneFBO();
	void CreateScreenFBO();
	void CreateQuadVAO();

	float GetSceneAvgLuminance();

	void Shutdown();

	static uint32_t sceneFBO, RBO;

	static PostEffects::GaussianBlur blur;

	static uint32_t quadVAO, quadVBO;
	static uint32_t sceneColorBufs[2];

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
	inline GLFWwindow* g_window = nullptr;
};
