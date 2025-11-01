#include "renderer.hpp"

#include "../game/resource_manager.hpp"

namespace Render {
	Renderer::Renderer(Config& config)
		: config(config), FBO(0), RBO(0),
		  UBOMatrices(0), quadVAO(0), quadVBO(0),
		  textureColorBuffer(0) {
		postfxShader.loadShader("assets/shaders/postfx.vert", "assets/shaders/postfx.frag");

		glViewport(0, 0, config.windowRes.x, config.windowRes.y);

		// glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction
		// glEnable(GL_DEPTH_TEST);

		glEnable(GL_CULL_FACE); // Включаем отсечение задних граней
		glCullFace(GL_BACK); // Указываем, какие грани отсекать (задние)
		glFrontFace(GL_CCW); // Указываем порядок вершин для лицевых граней (CCW по умолчанию)

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		uint32_t shadow_tex = ResourceManager::CreateTexture("assets/textures/shadow.png");
		ShaderPtr shader = std::make_shared<Shader>("assets/shaders/shadow.vert",
		                                            "assets/shaders/shadow.frag");

		shadow = std::make_shared<Sprite>(shadow_tex);
		shadow->shader = shader;

		createFrameBuffer();
		createQuadVAO();
		createUBO();
	}

	void Renderer::createFrameBuffer() {
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		// create a color attachment texture
		glGenTextures(1, &textureColorBuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.renderRes.x, config.renderRes.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, config.renderRes.x, config.renderRes.y, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.renderRes.x, config.renderRes.y);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
		// now actually attach it
		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::createQuadVAO() {
		// Вершины квадрата
		float quadVertices[] = {
			// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

	void Renderer::createUBO() {
		// ====== MATRICES ======
		glGenBuffers(1, &UBOMatrices);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBOMatrices, 0, 2 * sizeof(glm::mat4));
		// ====== LIGHT ======
		glGenBuffers(1, &UBOLight);

		static_assert(sizeof(Light) == 64);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Light), NULL,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 1, UBOLight, 0, sizeof(Light));
		// ====== DATA ======
		glGenBuffers(1, &UBOData);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOData);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3) + sizeof(float), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 2, UBOData, 0, sizeof(glm::vec3) + sizeof(float));
	}

	void Renderer::renderClear() {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClearColor(config.fillColor.r, config.fillColor.g, config.fillColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::updateUBOMatrices(const glm::mat4& projection, const glm::mat4& view) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::updateUBOLight(Light& light) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(light), &light);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::UpdateUBOData(const glm::vec3& viewPos) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOData);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(viewPos), glm::value_ptr(viewPos));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec3), sizeof(float), &config.renderDistance);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::drawScene(Scene& scene) {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // draw everything in custom framebuffer

		updateUBOLight(scene.light);
		UpdateUBOData(scene.camera.position);
		updateUBOMatrices(
			glm::perspective(glm::radians(scene.camera.fov), (float)config.windowRes.x / (float)config.windowRes.y,
			                 0.1f, config.renderDistance), scene.camera.getViewMatrix());


		for (auto& object : scene.objects) {
			object->draw({});
		}

		for (auto& object : scene.objects) {
			if (!object->castShadow) continue;
			Transform transform;
			transform.position = object->transform.position;
			transform.scale = object->transform.scale * glm::vec3(20);
			shadow->draw(transform);

			// glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(objX, floorY + 0.01f, objZ));
			// model = glm::scale(model, glm::vec3(1.5f, 1.0f, 1.5f));
			// shader.setMat4("model", model);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // set default framebuffer
	}

	void Renderer::renderFrame(PostEffects effects) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

		glViewport(0, 0, config.windowRes.x, config.windowRes.y);

		postfxShader.use();
		postfxShader.setUniform1i("effects.quantization", effects.quantization);
		postfxShader.setUniform1i("effects.quantizationLevel", effects.quantizationLevel);
		postfxShader.setUniform2f("resolution", config.windowRes);

		postfxShader.setUniform1i("effects.vignette", effects.vignette);
		postfxShader.setUniform1f("effects.vignetteIntensity", effects.vignetteIntensity);
		postfxShader.setUniform3f("effects.vignetteColor", effects.vignetteColor);

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		// use the color attachment texture as the texture of the quad plane
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);

		renderClear();
	}


	void Renderer::updateRenderRes() {
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, config.renderRes.x, config.renderRes.y, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.renderRes.x, config.renderRes.y);
	}

	Renderer::~Renderer() {
		glDeleteFramebuffers(1, &FBO);
		glDeleteTextures(1, &textureColorBuffer);
		glDeleteRenderbuffers(1, &RBO);
	}
}
