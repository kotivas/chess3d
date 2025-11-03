#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "../game/scene.hpp"
#include "shadow.hpp"

namespace Render {
	struct PostEffects {
		float gamma;

		bool quantization;
		int quantizationLevel;

		bool vignette;
		float vignetteIntensity;
		glm::vec3 vignetteColor;
	};
}

namespace Renderer {
	void Init();
	void InitGLFW();

	// render shadow map from light position
	void GenShadowMaps(Scene& scene);

	// draw scene to the FBO
	void FrameBegin(Scene& scene);

	// applying post fx and rendering to the screen FBO
	void FrameEnd(Render::PostEffects effects);

	// void UpdateShadowRes();
	void UpdateRenderRes();

	void RenderClear();

	void CreateFrameBuffer();
	void CreateQuadVAO();
	void CreateUBO();

	void UpdateUBOMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& dirLightSpaceMatrix,
	                       const glm::mat4& spotLightSpaceMatrix);
	void UpdateUBOLights(DirLight& dirLight, PointLight& pointLight, SpotLight& spotLight);
	void UpdateUBOData(const glm::vec3& viewPos);

	void Shutdown();

	static Render::Shader postfxShader;

	static uint32_t FBO, RBO;
	static uint32_t UBOMatrices, UBOLights, UBOData;

	static Render::DirShadowData dirShadow;
	static Render::SpotShadowData spotShadow;
	static Render::OmniShadowData pointShadow;

	static uint32_t quadVAO, quadVBO;
	static uint32_t textureColorBuffer;

	inline GLFWwindow* g_window = nullptr;
};
