#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "PostEffects/GaussianBlur.hpp"
#include "../Game/Scene.hpp"
#include "Shadow.hpp"
#include "Skybox.hpp"
#include "Common/Color.hpp"


namespace Renderer {
	void Init();
	void InitGLFW();

	// render shadow map from light position
	void GenShadowMaps(Scene& scene);

	void FrameBegin(Scene& scene);
	void ApplyPostProcess(double dt);
	void FrameEnd();

	// void UpdateShadowRes();
	void UpdateRenderRes();

	void DrawSkybox(SkyboxPtr sky, Camera::Camera cam);

	void DrawTextureOnScreen(uint32_t texture, float x, float y, float w, float h);
	void DrawRectOnScreen(float x, float y, float w, float h, const Color::rgba_t& color);

	void RenderClear();

	void CreateSceneFBO();
	void CreateScreenFBO();
	void CreateQuadVAO();
	void CreateUBO();

	void UpdateUBOMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& dirLightSpaceMatrix,
	                       const glm::mat4& spotLightSpaceMatrix);
	void UpdateUBOLights(DirLight& dirLight, PointLight& pointLight, SpotLight& spotLight);
	void UpdateUBOData(const glm::vec3& viewPos);

	float GetSceneAvgLuminance();

	void Shutdown();

	static uint32_t sceneFBO, RBO;
	static uint32_t UBOMatrices, UBOLights, UBOData;

	static DirShadowData dirShadow;
	static SpotShadowData spotShadow;
	static OmniShadowData pointShadow;
	static PostEffects::GaussianBlur blur;

	static uint32_t quadVAO, quadVBO;
	static uint32_t sceneColorBufs[2];

	static uint32_t screenFBO;
	static uint32_t screenColorBuf;

	inline GLFWwindow* g_window = nullptr;
};
