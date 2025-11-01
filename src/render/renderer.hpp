#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "../game/config.hpp"
#include "../game/scene.hpp"

namespace Render {
	struct PostEffects {
		bool quantization;
		int quantizationLevel;

		bool vignette;
		float vignetteIntensity;
		glm::vec3 vignetteColor;
	};

	class Renderer {
	public:
		Renderer(Config& config);

		void drawShadows(Scene& scene);

		// draw scene to the FBO
		void drawScene(Scene& scene);

		// applying post fx and rendering to the screen FBO
		void renderFrame(PostEffects effects);

		void updateShadowRes();
		void updateRenderRes();

		~Renderer();

	private:
		//GLFWwindow* window;

		void renderClear();

		void createFrameBuffer();
		void createQuadVAO();
		void createUBO();

		void updateUBOMatrices(const glm::mat4& projection, const glm::mat4& view);
		void updateUBOLight(Light& light);
		void UpdateUBOData(const glm::vec3& viewPos);

		Shader postfxShader;

		SpritePtr shadow;

		Config& config;

		uint32_t FBO, RBO;
		uint32_t UBOMatrices, UBOLight, UBOData;

		uint32_t quadVAO, quadVBO;
		uint32_t textureColorBuffer;
	};
}
