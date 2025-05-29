#include "renderer.hpp"

namespace Render {
	Renderer::Renderer(Config& config)
		: config(config), FBO(0), RBO(0),
		  UBOMatrices(0), quadVAO(0), quadVBO(0),
		  textureColorBuffer(0) {
		postfxShader.loadShader("assets/shaders/postfx.vert", "assets/shaders/postfx.frag");

		glViewport(0, 0, config.windowRes.x, config.windowRes.y);

		glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_CULL_FACE); // Включаем отсечение задних граней
		glCullFace(GL_BACK); // Указываем, какие грани отсекать (задние)
		glFrontFace(GL_CCW); // Указываем порядок вершин для лицевых граней (CCW по умолчанию)

		// work with shadows
		ShaderPtr depthShader = std::make_shared<Render::Shader>("assets/shaders/depth.vert",
		                                                         "assets/shaders/depth.frag");
		spotShadow.shader = depthShader;
		spotShadow.resolution = config.shadowRes; // todo maybe i should split it
		spotShadow.generate();

		dirShadow.shader = depthShader;
		dirShadow.resolution = config.shadowRes;
		dirShadow.generate();

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.renderRes.x, config.renderRes.y, 0, GL_RGB, GL_UNSIGNED_BYTE,
		             NULL);
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
		glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBOMatrices, 0, 4 * sizeof(glm::mat4));
		// ====== LIGHTS ======
		glGenBuffers(1, &UBOLights);

		static_assert(sizeof(DirLight) == 80);
		static_assert(sizeof(PointLight) == 96);
		static_assert(sizeof(SpotLight) == 112);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOLights);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirLight) + sizeof(PointLight) + sizeof(SpotLight), NULL,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 1, UBOLights, 0,
		                  sizeof(DirLight) + sizeof(PointLight) + sizeof(SpotLight));
		// ====== DATA ======
		glGenBuffers(1, &UBOData);

		glBindBuffer(GL_UNIFORM_BUFFER, UBOData);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 2, UBOData, 0, sizeof(glm::vec3));
	}

	void Renderer::renderClear() {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::updateUBOMatrices(const glm::mat4& projection, const glm::mat4& view,
	                                 const glm::mat4& dirLightSpaceMatrix, const glm::mat4& spotLightSpaceMatrix) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4),
		                glm::value_ptr(dirLightSpaceMatrix));
		glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), sizeof(glm::mat4),
		                glm::value_ptr(spotLightSpaceMatrix));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::updateUBOLights(DirLight& dirLight, PointLight& pointLight, SpotLight& spotLight) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOLights);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirLight), &dirLight);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirLight), sizeof(PointLight), &pointLight);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirLight) + sizeof(PointLight), sizeof(spotLight), &spotLight);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::UpdateUBOData(const glm::vec3& viewPos) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOData);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(viewPos), glm::value_ptr(viewPos));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void Renderer::genShadowMaps(Scene& scene) {
		glCullFace(GL_FRONT);

		// light space matrix for directional light

		if (scene.dirLight.enable) {
			dirShadow.calculateLightSpaceMatrix(scene.dirLight.direction, 0.1f, config.renderDistance);

			// render scene from light's point of view
			glViewport(0, 0, config.shadowRes, config.shadowRes);
			glBindFramebuffer(GL_FRAMEBUFFER, dirShadow.shadowMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			dirShadow.shader->use();
			dirShadow.shader->setUniformMat4fv("u_LightSpaceMatrix", 1, false, dirShadow.lightSpaceMatrix);

			for (auto& model : scene.models) {
				if (model->castShadow) model->draw(dirShadow.shader);
			}
		}
		if (scene.spotLight.enable) {
			spotShadow.calculateLightSpaceMatrix(scene.spotLight.position, scene.spotLight.direction,
			                                     scene.spotLight.outerCutOff, 0.1f, config.renderDistance);


			// render scene from light's point of view
			glViewport(0, 0, config.shadowRes, config.shadowRes);
			glBindFramebuffer(GL_FRAMEBUFFER, spotShadow.shadowMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			spotShadow.shader->use();
			spotShadow.shader->setUniformMat4fv("u_LightSpaceMatrix", 1, false, spotShadow.lightSpaceMatrix);

			for (auto& model : scene.models) {
				if (model->castShadow) model->draw(spotShadow.shader);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, config.renderRes.x, config.renderRes.y);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);
	}

	void Renderer::drawScene(Scene& scene) {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // draw everything in custom framebuffer
		scene.camera.updatePosition();

		updateUBOLights(scene.dirLight, scene.pointLight, scene.spotLight);
		UpdateUBOData(scene.camera.position);
		updateUBOMatrices(
			glm::perspective(glm::radians(scene.camera.fov), (float)config.windowRes.x / (float)config.windowRes.y,
			                 0.1f, config.renderDistance),
			glm::lookAt(scene.camera.position, scene.camera.target, glm::vec3(0.0f, 1.0f, 0.0f)),
			dirShadow.lightSpaceMatrix, spotShadow.lightSpaceMatrix
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dirShadow.shadowMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, spotShadow.shadowMap);

		for (auto& model : scene.models) {
			model->draw();
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
		// glBindTexture(GL_TEXTURE_2D, spotShadow.shadowMap);
		// glBindTexture(GL_TEXTURE_2D, dirShadow.shadowMap);
		// use the color attachment texture as the texture of the quad plane
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);

		renderClear();
	}


	// void Renderer::updateShadowRes() {
	// 	glBindTexture(GL_TEXTURE_2D, dirShadowMap);
	// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
	// 	             config.shadowRes, config.shadowRes, 0,
	// 	             GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	//
	// 	// check FBO status
	// 	glBindFramebuffer(GL_FRAMEBUFFER, dirShadowMapFBO);
	// 	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	// 	if (status != GL_FRAMEBUFFER_COMPLETE) {
	// 		std::cerr << "Shadow FBO incomplete after resize! Status: 0x"
	// 			<< std::hex << status << std::dec << std::endl;
	// 	}
	// 	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// }

	void Renderer::updateRenderRes() {
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.renderRes.x, config.renderRes.y, 0, GL_RGB, GL_UNSIGNED_BYTE,
		             NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.renderRes.x, config.renderRes.y);
	}

	Renderer::~Renderer() {
		glDeleteFramebuffers(1, &FBO);
		glDeleteTextures(1, &textureColorBuffer);
		glDeleteRenderbuffers(1, &RBO);
	}
}
