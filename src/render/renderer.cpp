#include "renderer.hpp"

#include "../com/util.hpp"
#include "../com/config.hpp"
#include "resourcemgr/resourcemgr.hpp"

namespace Renderer {
	void Init() {
		FBO = 0;
		RBO = 0;
		UBOMatrices = 0;
		quadVAO = 0;
		quadVBO = 0;
		textureColorBuffer = 0;

		InitGLFW();

		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(Util::glDebugOutput, nullptr);
		}

		glViewport(0, 0, g_config.sys_windowResolution.x, g_config.sys_windowResolution.y);

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_CULL_FACE); // Включаем отсечение задних граней
		glCullFace(GL_BACK); // Указываем, какие грани отсекать (задние)
		glFrontFace(GL_CCW); // Указываем порядок вершин для лицевых граней (CCW по умолчанию)

		// work with shadows
		spotShadow.resolution = g_config.r_shadowRes; // todo maybe i should split it
		spotShadow.generate();

		dirShadow.resolution = g_config.r_shadowRes;
		dirShadow.generate();

		pointShadow.resolution = g_config.r_shadowRes;
		pointShadow.generate();

		CreateFrameBuffer();
		CreateQuadVAO();
		CreateUBO();
	}


	void InitGLFW() {
		// Init GLFW
		glfwInit();
		// Set all the required options for GLFW
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		g_window = glfwCreateWindow(g_config.sys_windowResolution.x, g_config.sys_windowResolution.y, "chess3d",
		                            nullptr, nullptr);

		glfwMakeContextCurrent(g_window);
		glfwSwapInterval(g_config.r_vsync); // vsync 1 - on; 0 - off

		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	}


	void CreateFrameBuffer() {
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		// create a color attachment texture
		glGenTextures(1, &textureColorBuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.renderRes.x, config.renderRes.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_config.r_resolution.x, g_config.r_resolution.y, 0, GL_RGBA,
		             GL_FLOAT,
		             NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_config.r_resolution.x, g_config.r_resolution.y);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
		// now actually attach it
		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CreateQuadVAO() {
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

	void CreateUBO() {
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
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec3) + sizeof(float), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 2, UBOData, 0, sizeof(glm::vec3) + sizeof(float));
	}

	void RenderClear() {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClearColor(g_config.r_fillColor.r, g_config.r_fillColor.g, g_config.r_fillColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void UpdateUBOMatrices(const glm::mat4& projection, const glm::mat4& view,
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

	void UpdateUBOLights(DirLight& dirLight, PointLight& pointLight, SpotLight& spotLight) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOLights);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirLight), &dirLight);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirLight), sizeof(PointLight), &pointLight);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirLight) + sizeof(PointLight), sizeof(spotLight), &spotLight);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void UpdateUBOData(const glm::vec3& viewPos) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOData);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(viewPos), glm::value_ptr(viewPos));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec3), sizeof(float), &g_config.r_renderDistance);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void GenShadowMaps(Scene& scene) {
		glCullFace(GL_FRONT);

		// light space matrix for directional light

		if (scene.dirLight.enable) {
			dirShadow.calculateLightSpaceMatrix(scene.dirLight.direction, 0.1f, g_config.r_renderDistance);

			// render scene from light's point of view
			glViewport(0, 0, dirShadow.resolution, dirShadow.resolution);
			glBindFramebuffer(GL_FRAMEBUFFER, dirShadow.shadowMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			ResourceMgr::GetShaderByName("depth")->use();
			ResourceMgr::GetShaderByName("depth")->setUniformMat4fv("u_LightSpaceMatrix", false,
			                                                        dirShadow.lightSpaceMatrix);

			for (auto& object : scene.objects) {
				if (object->castShadow) object->draw(ResourceMgr::GetShaderByName("depth"), {});
			}
		}
		if (scene.spotLight.enable) {
			spotShadow.calculateLightSpaceMatrix(scene.spotLight.position, scene.spotLight.direction,
			                                     scene.spotLight.outerCutOff, 0.1f, g_config.r_renderDistance);

			// render scene from light's point of view
			glViewport(0, 0, spotShadow.resolution, spotShadow.resolution);
			glBindFramebuffer(GL_FRAMEBUFFER, spotShadow.shadowMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			ResourceMgr::GetShaderByName("depth")->use();
			ResourceMgr::GetShaderByName("depth")->setUniformMat4fv("u_LightSpaceMatrix", false,
			                                                        spotShadow.lightSpaceMatrix);

			for (auto& object : scene.objects) {
				if (object->castShadow) object->draw(ResourceMgr::GetShaderByName("depth"), {});
			}
		}
		if (scene.pointLight.enable) {
			pointShadow.genTransformMatrixes(scene.pointLight.position, 0.1f, g_config.r_renderDistance);

			// render scene from light's point of view
			glViewport(0, 0, pointShadow.resolution, pointShadow.resolution);
			glBindFramebuffer(GL_FRAMEBUFFER, pointShadow.shadowCubemapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			ResourceMgr::GetShaderByName("point_shadow_depth")->use();

			// TODO возможно я могу как то сделать, что бы за раз передавать 6 матриц
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniformMat4fv(
				"shadowMatrices[0]", false, pointShadow.transforms[0]);
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniformMat4fv(
				"shadowMatrices[1]", false, pointShadow.transforms[1]);
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniformMat4fv(
				"shadowMatrices[2]", false, pointShadow.transforms[2]);
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniformMat4fv(
				"shadowMatrices[3]", false, pointShadow.transforms[3]);
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniformMat4fv(
				"shadowMatrices[4]", false, pointShadow.transforms[4]);
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniformMat4fv(
				"shadowMatrices[5]", false, pointShadow.transforms[5]);

			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniform1f("far_plane", g_config.r_renderDistance);
			ResourceMgr::GetShaderByName("point_shadow_depth")->setUniform3f("lightPos", scene.pointLight.position);

			for (auto& object : scene.objects) {
				if (object->castShadow) object->draw(ResourceMgr::GetShaderByName("point_shadow_depth"), {});
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, g_config.r_resolution.x, g_config.r_resolution.y);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);
	}

	void FrameBegin(Scene& scene) {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // draw everything in custom framebuffer

		UpdateUBOLights(scene.dirLight, scene.pointLight, scene.spotLight);
		UpdateUBOData(scene.camera.position);
		UpdateUBOMatrices(
			glm::perspective(glm::radians(scene.camera.fov),
			                 (float)g_config.sys_windowResolution.x / (float)g_config.sys_windowResolution.y,
			                 0.1f, g_config.r_renderDistance), scene.camera.getViewMatrix(),
			dirShadow.lightSpaceMatrix, spotShadow.lightSpaceMatrix
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, dirShadow.shadowMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, spotShadow.shadowMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadow.shadowCubemap);
	}

	void FrameEnd() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

		glViewport(0, 0, g_config.sys_windowResolution.x, g_config.sys_windowResolution.y);


		Render::ShaderPtr postfxShader = ResourceMgr::GetShaderByName("postfx");

		postfxShader->use();

		postfxShader->setUniform1f("effects.gamma", g_config.r_gamma);

		postfxShader->setUniform1i("effects.quantization",g_config.fx_quantization);
		postfxShader->setUniform1i("effects.quantizationLevel", g_config.fx_quantizationLevel);
		postfxShader->setUniform2f("resolution", g_config.sys_windowResolution);

		postfxShader->setUniform1i("effects.vignette", g_config.fx_vignette);
		postfxShader->setUniform1f("effects.vignetteIntensity", g_config.fx_vignetteIntensity);
		postfxShader->setUniform3f("effects.vignetteColor", g_config.fx_vignetteColor);

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		// use the color attachment texture as the texture of the quad plane
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);
		RenderClear();

		glfwSwapBuffers(g_window);
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


	void DrawRectOnScreen(float x, float y, float w, float h, const Color::rgba_t& color) {
		Render::ShaderPtr shader = ResourceMgr::GetShaderByName("solidcolor");

		shader->use();

		// Устанавливаем матрицу модели
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(x, y, 0.0f));
		model = glm::scale(model, glm::vec3(w, h, 1.0f));

		shader->setUniformMat4fv("uModel", false, model);

		glm::mat4 projection = glm::ortho(0.0f, (float)g_config.r_resolution.x, (float)g_config.r_resolution.y, 0.0f);

		shader->setUniformMat4fv("uProjection", false, projection);

		shader->setUniform4f("color", color);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}

	void DrawTextureOnScreen(uint32_t texture, float x, float y, float w, float h) {
		Render::ShaderPtr shader = ResourceMgr::GetShaderByName("texture");

		shader->use();

		// Устанавливаем матрицу модели
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(x, y, 0.0f));
		model = glm::scale(model, glm::vec3(w, h, 1.0f));

		shader->setUniformMat4fv("uModel", false, model);

		glm::mat4 projection = glm::ortho(0.0f, (float)g_config.r_resolution.x, (float)g_config.r_resolution.y, 0.0f);

		shader->setUniformMat4fv("uProjection", false, projection);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);


		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}

	void Shutdown() {
		glDeleteFramebuffers(1, &FBO);
		glDeleteTextures(1, &textureColorBuffer);
		glDeleteRenderbuffers(1, &RBO);
	}

	void UpdateRenderRes() {
		glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_config.r_resolution.x, g_config.r_resolution.y, 0, GL_RGBA,
		             GL_FLOAT,
		             NULL);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_config.r_resolution.x, g_config.r_resolution.y);
	}
}
