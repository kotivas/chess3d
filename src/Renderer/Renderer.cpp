#include "Renderer.hpp"

#include "Common/Utils.hpp"
#include "Common/Config.hpp"
#include "Common/GlUtils.hpp"
#include "Core/Logger.hpp"
#include "PostEffects/PostEffects.hpp"
#include "ResourceMgr/ResourceMgr.hpp"
#include "UI/Console/Console.hpp"

namespace Renderer {
	GLuint msdfvao, msdfvbo;

	void Init() {
		sceneFBO = 0;
		RBO = 0;
		quadVAO = 0;
		quadVBO = 0;
		sceneColorBufs[0] = 0;
		sceneColorBufs[1] = 0;
		screenColorBuf = 0;
		screenFBO = 0;

		quadShader = AssetManager::g_ShaderManager.getHandle("screenfbo");
		if (!quadShader) Log::Warning("Unable to get handle of shader screenfbo");
		solidShader = AssetManager::g_ShaderManager.getHandle("solidcolor");
		if (!solidShader) Log::Warning("Unable to get handle of shader solidcolor");
		textureShader = AssetManager::g_ShaderManager.getHandle("texture");
		if (!textureShader) Log::Warning("Unable to get handle of shader texture");
		postfxShader = AssetManager::g_ShaderManager.getHandle("postfx");
		if (!postfxShader) Log::Warning("Unable to get handle of shader postfx");
		msdfTextShader = AssetManager::g_ShaderManager.getHandle("msdf");
		if (!msdfTextShader) Log::Warning("Unable to get handle of shader msdf");

		SetGlDebugCallback(GlUtils::glDebugOutput);

		CreateSceneFBO();
		CreateScreenFBO();
		CreateQuadVAO();

		blur.init(g_config.r_resolution.x, g_config.r_resolution.y, quadVAO, quadVBO);
		shadowPass.init();
		shadowPass.addPointShadow();
		lightPass.init();

		glGenVertexArrays(1, &msdfvao);
		glGenBuffers(1, &msdfvbo);
	}

	bool SetGlDebugCallback(GLDEBUGPROC fn) {
		int flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(fn, nullptr);
			return true;
		}
		Log::Warning("Unable to set opengl debug callback");
		return false;
	}

	void GlInit() {
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

		// log gl info
		const std::string vendor{(char*)glGetString(GL_VENDOR)};
		const std::string device{(char*)glGetString(GL_RENDERER)};
		const std::string version{(char*)glGetString(GL_VERSION)};
		const std::string glsl_version{(char*)glGetString(GL_SHADING_LANGUAGE_VERSION)};
		Log::Debug("GL vendor: {0}", vendor);
		Log::Debug("GL device: {0}", device);
		Log::Debug("GL version: {0}", version);
		Log::Debug("GLSL version: {0}", glsl_version);
	}

	void CreateScreenFBO() {
		glGenFramebuffers(1, &screenFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

		glGenTextures(1, &screenColorBuf);

		glBindTexture(GL_TEXTURE_2D, screenColorBuf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_config.r_resolution.x, g_config.r_resolution.y, 0, GL_RGBA,
		             GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorBuf, 0);
	}

	void CreateSceneFBO() {
		glGenFramebuffers(1, &sceneFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		// create a color attachment texture

		glGenTextures(2, sceneColorBufs);
		for (int i = 0; i < 2; i++) {
			glBindTexture(GL_TEXTURE_2D, sceneColorBufs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_config.r_resolution.x, g_config.r_resolution.y, 0, GL_RGBA,
			             GL_FLOAT, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, sceneColorBufs[i], 0);
		}

		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_config.r_resolution.x,
		                      g_config.r_resolution.y);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CreateQuadVAO() {
		float quadVertices[] = {
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

	void RenderClear() {
		glClearColor(0, 0, 0, 1.0f);

		glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	float GetSceneAvgLuminance() {
		glBindTexture(GL_TEXTURE_2D, sceneColorBufs[0]);
		glGenerateMipmap(GL_TEXTURE_2D);

		// int mip_level = std::bit_width(std::max(g_config.r_resolution.x, g_config.r_resolution.y)) - 1; // for 1920x1080 mip level is 10
		float pixel[3] = {0.0f, 0.0f, 0.0f};
		glGetTexImage(GL_TEXTURE_2D, 10, GL_RGB, GL_FLOAT, &pixel);

		return 0.2126f * pixel[0] + 0.7152f * pixel[1] + 0.0722f * pixel[2];
	}

	void DrawSky(SkyPtr sky, const Camera::Camera& cam, float time) {
		Shader* shader = AssetManager::g_ShaderManager.get(sky->shader);

		shader->use();

		shader->setUniform1f("uTime", time);
		shader->setUniform3f("uSunDirection", sky->sun_direction);
		for (int i = 0; i < 10; i++) shader->setUniform3f("uAtmParams[" + std::to_string(i) + "]", sky->atm_params[i]);
		const glm::mat4 viewNoTrans = glm::mat4(glm::mat3(cam.viewMatrix));
		shader->setUniformMat4fv("uView", false, viewNoTrans);
		shader->setUniformMat4fv("uProjection", false, cam.projectionMatrix);

		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		glBindVertexArray(sky->vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_3D, 0);

		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

	void DrawDebug(int fps, float scale, float time, glm::vec3 cam_pos) {
		const MSDFText::FontPtr font = ResourceMgr::GetFontByName("inconsolata_light");
		const std::vector debug_lines = {
			"FPS: " + std::to_string(fps),
			std::format("Cam pos: {0:.2f} {1:.2f} {2:.2f}", cam_pos.x, cam_pos.y, cam_pos.z),
			std::format("Exp: {:.3f}", g_config.fx_exposure),
			std::format("Time: {:.1f}", time)
		};

		for (int i = 0; i < debug_lines.size(); i++) {
			const float lineHeight = font->lineHeight * scale;
			float x = g_config.r_resolution.x - font->getStringWidth(debug_lines[i], scale);
			float y = g_config.r_resolution.y - lineHeight - i * lineHeight;

			DrawText({
				debug_lines[i],
				scale,
				{x, y},
				{Color::WHITE, 1},
				font
			});
		}
	}

	void GenerateRenderItem(const std::shared_ptr<Drawable>& drawable, std::vector<RenderItem>& items) {
		if (drawable->type == Drawable::Mesh) {
			MeshPtr mesh = std::dynamic_pointer_cast<Mesh>(drawable);
			items.emplace_back(mesh, mesh->material, mesh->transform.getMatrix());
		} else if (drawable->type == Drawable::Model) {
			auto model = std::dynamic_pointer_cast<Model>(drawable);
			for (const auto& mesh : model->meshes)
				items.emplace_back(mesh, mesh->material, model->transform.getMatrix() * mesh->transform.getMatrix());
		}
	}

	void Render(Scene& scene, double dt) {
		RenderClear();

		RenderContext ctx{
			// OPTIMIZE do not use whole scene var
			.near = scene.camera.nearPlane,
			.far = scene.camera.farPlane,
			.render_width = g_config.r_resolution.x,
			.render_height = g_config.r_resolution.y,
			.scene = scene,
			.target_fbo = sceneFBO,
		};
		for (const auto& drawable : scene.objects) {
			if (drawable->drawable) GenerateRenderItem(drawable, ctx.render_items);
		}

		shadowPass.pass(ctx);

		ctx.target_fbo = sceneFBO;
		glViewport(0, 0, g_config.r_resolution.x, g_config.r_resolution.y);
		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		DrawSky(scene.sky, scene.camera, scene.time); // todo SkyPass

		lightPass.pass(ctx);

		glDisable(GL_CULL_FACE);

		ApplyPostProcess(dt); // todo PostFxPass
		// todo UIPass (UiElements)
		Console::Draw();
		DrawDebug(int(1 / dt), 25, scene.time, scene.camera.position);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// OUTPUT TO THE 0 BUFFER

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, g_config.sys_windowResolution.x, g_config.sys_windowResolution.y);

		AssetManager::g_ShaderManager.get(quadShader)->use();
		AssetManager::g_ShaderManager.get(quadShader)->setUniform1i("texture0", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screenColorBuf);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// debug
		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		AssetManager::g_ShaderManager.get("2DTexture")->use();
		AssetManager::g_ShaderManager.get("2DTexture")->setUniform1i("texture0", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ctx.dir_shadow.shadow_map);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		// end

		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(g_window);
	}

	void ApplyPostProcess(double dt) {
		uint32_t bloom = sceneColorBufs[1];

		// calc exposure
		if (g_config.fx_autoExposure) {
			g_config.fx_exposure = PostEffects::GetLerpExposure(g_config.fx_exposure, GetSceneAvgLuminance(),
			                                                    g_config.fx_autoExposureSpeed);
		}
		if (g_config.fx_bloom) bloom = blur.blur(sceneColorBufs[1], g_config.r_blurPasses);

		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
		glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

		Shader* shader = AssetManager::g_ShaderManager.get(postfxShader);

		shader->use();

		shader->setUniform2f("resolution", g_config.sys_windowResolution);
		shader->setUniform1f("time", glfwGetTime());

		shader->setUniform1i("effects.bloom", g_config.fx_bloom);
		shader->setUniform1f("effects.gamma", g_config.r_gamma);
		shader->setUniform1f("effects.chromaticOffset", g_config.fx_chromaticOffset);
		shader->setUniform1i("effects.quantization", g_config.fx_quantization);
		shader->setUniform1i("effects.quantizationLevel", g_config.fx_quantizationLevel);
		shader->setUniform1i("effects.vignette", g_config.fx_vignette);
		shader->setUniform1f("effects.vignetteIntensity", g_config.fx_vignetteIntensity);
		shader->setUniform3f("effects.vignetteColor", g_config.fx_vignetteColor);
		shader->setUniform1f("effects.exposure", g_config.fx_exposure);
		shader->setUniform1f("effects.saturation", g_config.fx_saturation);

		shader->setUniform1i("screenTexture", 0);
		shader->setUniform1i("bloomBlur", 1);

		glViewport(0, 0, g_config.r_resolution.x, g_config.r_resolution.y);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneColorBufs[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bloom); // BLOOM BLUR
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);
	}

	void DrawText(const MSDFText::Text& text) {
		struct Vtx {
			float x, y, u, v;
		};

		if (!text.font) return;

		std::vector<Vtx> verts;
		verts.reserve(text.string.size() * 6);

		float penX = text.position.x;
		float penY = text.position.y; // baseline offset (≈ ascender)

		for (const char cc : text.string) {
			const MSDFText::Glyph& g = text.font->getGlyph(cc);

			float gx0 = penX + g.planeLeft * text.scale;
			float gy0 = penY + g.planeBottom * text.scale;
			float gx1 = penX + g.planeRight * text.scale;
			float gy1 = penY + g.planeTop * text.scale;

			float u0 = g.uvLeft;
			float v0 = g.uvBottom;
			float u1 = g.uvRight;
			float v1 = g.uvTop;

			verts.push_back({gx0, gy0, u0, v0});
			verts.push_back({gx1, gy0, u1, v0});
			verts.push_back({gx1, gy1, u1, v1});
			verts.push_back({gx1, gy1, u1, v1});
			verts.push_back({gx0, gy1, u0, v1});
			verts.push_back({gx0, gy0, u0, v0});

			penX += g.advance * text.scale;
		}

		if (verts.empty()) return;

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(msdfvao);
		glBindBuffer(GL_ARRAY_BUFFER, msdfvbo);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vtx), verts.data(), GL_DYNAMIC_DRAW);

		GLint posLoc = 0;
		GLint uvLoc = 1;
		glEnableVertexAttribArray(posLoc);
		glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)0);
		glEnableVertexAttribArray(uvLoc);
		glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vtx), (void*)(2 * sizeof(float)));

		Shader* shader = AssetManager::g_ShaderManager.get(msdfTextShader);

		shader->use();

		glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(g_config.r_resolution.x), 0.0f,
		                            static_cast<float>(g_config.r_resolution.y));

		shader->setUniformMat4fv("uProjection", false, proj);
		shader->setUniform4f("uColor", text.color.r, text.color.g, text.color.b, text.color.a);
		shader->setUniform1f("uScale", text.scale);
		shader->setUniform1f("uPxRange", text.font->distanceRange);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, text.font->atlas);
		shader->setUniform1i("uAtlas", 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());

		glDisableVertexAttribArray(posLoc);
		glDisableVertexAttribArray(uvLoc);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
	}

	void DrawRectOnScreen(float x, float y, float w, float h, const Color::rgba_t& color) {
		Shader* shader = AssetManager::g_ShaderManager.get(solidShader);

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
		Shader* shader = AssetManager::g_ShaderManager.get(textureShader);

		shader->use();

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
		// todo delete all fbo, rbo and textures
		glDeleteFramebuffers(1, &screenFBO);
		glDeleteTextures(1, &screenColorBuf);

		glDeleteFramebuffers(1, &sceneFBO);
		glDeleteTextures(2, sceneColorBufs);
		glDeleteRenderbuffers(1, &RBO);
	}

	void UpdateRenderRes() {
		for (int i = 0; i < 2; i++) {
			glBindTexture(GL_TEXTURE_2D, sceneColorBufs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_config.r_resolution.x, g_config.r_resolution.y, 0, GL_RGBA,
			             GL_FLOAT,
			             NULL);
		}

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_config.r_resolution.x, g_config.r_resolution.y);
	}
}
