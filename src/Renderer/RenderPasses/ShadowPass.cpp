#include "ShadowPass.hpp"

#include "LightningPass.hpp"
#include "AssetManager/AssetManager.hpp"
#include "AssetManager/ShaderManager.hpp"
#include "Common/Config.hpp"
#include "Core/Logger.hpp"

namespace Renderer {
	void ShadowPass::pass(RenderContext& ctx) {
		glEnable(GL_DEPTH_TEST);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		// glFrontFace(GL_CCW);

		if (ctx.scene.dir_light.enable) drawDirectionalShadow(ctx);
		if (ctx.scene.spot_light.enable) drawSpotShadow(ctx);
		if (ctx.scene.point_light.enable) drawPointShadow(ctx, 0);
	}

	glm::mat4 ShadowPass::calcDirLightSpace(const glm::vec3& light_dir, const float near, const float far) {
		// const glm::mat4 light_projection = glm::ortho(-far, far, -far, far, near, far);
		const glm::mat4 light_projection = glm::ortho(-100.f, 100.f, -100.f, 100.f, near, far);
		const glm::vec3 light_pos = -light_dir * glm::vec3(far);
		// HACK shadow casts incorrectly, if light falls vertically downward
		const glm::vec3 light_up = glm::abs(light_dir.y) < 0.999 ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);

		return light_projection * glm::lookAt(light_pos, light_pos + light_dir, light_up);
	}

	glm::mat4 ShadowPass::calcSpotLightSpace(const glm::vec3& light_pos, const glm::vec3& light_dir, const float fov,
	                                         const float near, const float far) {
		glm::mat4 lightProjection = glm::perspective(fov, 1.f, near, far);
		glm::mat4 lightView = glm::lookAt(light_pos, light_pos + light_dir, {0.f, 1.f, 0.f});

		return lightProjection * lightView;
	}

	std::array<glm::mat4, 6> ShadowPass::calcPointLightSpace(const glm::vec3& light_pos, const float near,
	                                                         const float far, const float resolution) {
		const glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), (float)resolution / (float)resolution,
		                                               near, far);

		std::array<glm::mat4, 6> transforms{};

		transforms[0] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f),
		                                          glm::vec3(0.0f, -1.0f, 0.0f));
		transforms[1] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f),
		                                          glm::vec3(0.0f, -1.0f, 0.0f));
		transforms[2] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f),
		                                          glm::vec3(0.0f, 0.0f, 1.0f));
		transforms[3] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f),
		                                          glm::vec3(0.0f, 0.0f, -1.0f));
		transforms[4] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f),
		                                          glm::vec3(0.0f, -1.0f, 0.0f));
		transforms[5] = shadow_proj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f),
		                                          glm::vec3(0.0f, -1.0f, 0.0f));

		return transforms;
	}

	void ShadowPass::drawRenderItem(const RenderItem& item, const Shader* shader) {
		if (!shader) return;

		shader->setUniformMat4fv("uModel", GL_FALSE, item.world_transform);
		glBindVertexArray(item.mesh->VAO);
		glDrawElements(GL_TRIANGLES, item.mesh->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void ShadowPass::drawDirectionalShadow(RenderContext& ctx) {
		ctx.dir_shadow.light_space = calcDirLightSpace(ctx.scene.dir_light.direction, ctx.near, ctx.far);

		// render scene from light's point of view
		glViewport(0, 0, _directional_shadow.resolution, _directional_shadow.resolution);
		glBindFramebuffer(GL_FRAMEBUFFER, _directional_shadow.fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		const auto shader = AssetManager::g_ShaderManager.get(_directional_shadow.shader);

		if (!shader) {
			Log::Error("ShadowPass::drawDirectionalShadow: Unable to get shader {0}", _directional_shadow.shader.id);
			return;
		}

		shader->use();
		shader->setUniformMat4fv("uLightSpaceMatrix", false, ctx.dir_shadow.light_space);

		for (const auto& item : ctx.render_items) {
			if (item.mesh->castShadow) drawRenderItem(item, shader);
		}

		ctx.dir_shadow.shadow_map = _directional_shadow.map;
	}

	void ShadowPass::drawSpotShadow(RenderContext& ctx) {
		ctx.spot_shadow.light_space = calcSpotLightSpace(ctx.scene.spot_light.position, ctx.scene.spot_light.direction,
		                                                 ctx.scene.camera.fov, ctx.near, ctx.far);

		// render scene from light's point of view
		glViewport(0, 0, _spot_shadow.resolution, _spot_shadow.resolution);
		glBindFramebuffer(GL_FRAMEBUFFER, _spot_shadow.fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		const auto shader = AssetManager::g_ShaderManager.get(_spot_shadow.shader);
		if (!shader) {
			Log::Error("ShadowPass::drawSpotShadow: Unable to get shader {0}", _directional_shadow.shader.id);
			return;
		}

		shader->use();
		shader->setUniformMat4fv("uLightSpaceMatrix", false, ctx.spot_shadow.light_space);
		for (const auto& item : ctx.render_items) {
			if (item.mesh->castShadow) drawRenderItem(item, shader);
		}
		ctx.spot_shadow.shadow_map = _spot_shadow.map;
	}

	void ShadowPass::drawPointShadow(RenderContext& ctx, uint8_t index) {
		ctx.point_shadows.emplace_back();

		ctx.point_shadows[index].transforms = calcPointLightSpace(ctx.scene.point_light.position, ctx.near, ctx.far,
		                                                          _point_shadows[index].resolution);

		glViewport(0, 0, _point_shadows[index].resolution, _point_shadows[index].resolution);
		glBindFramebuffer(GL_FRAMEBUFFER, _point_shadows[index].fbo);
		glClear(GL_DEPTH_BUFFER_BIT);


		const auto shader = AssetManager::g_ShaderManager.get(_point_shadows[index].shader);
		if (!shader) {
			Log::Error("ShadowPass::drawPointShadow: Unable to get shader {0}", _directional_shadow.shader.id);
			return;
		}

		shader->use();

		for (int i = 0; i < 6; i++) {
			shader->setUniformMat4fv("shadowMatrices[" + std::to_string(i) + "]", false,
			                         _point_shadows[index].transforms[i]);
		}
		//
		shader->setUniform3f("lightPos", ctx.scene.point_light.position);
		shader->setUniform1f("far_plane", ctx.far);

		for (const auto& item : ctx.render_items) {
			if (item.mesh->castShadow) drawRenderItem(item, shader);
		}
		ctx.point_shadows[index].shadow_map = _point_shadows[index].cubemap;
	}

	void ShadowPass::init() {
		_directional_shadow.resolution = g_config.r_shadowRes;
		_directional_shadow.shader = AssetManager::g_ShaderManager.getHandle("depth");
		generateMap(_directional_shadow.map, _directional_shadow.fbo, _directional_shadow.resolution);

		_spot_shadow.resolution = g_config.r_shadowRes;
		_spot_shadow.shader = AssetManager::g_ShaderManager.getHandle("depth");
		generateMap(_spot_shadow.map, _spot_shadow.fbo, _spot_shadow.resolution);
	}

	void ShadowPass::addPointShadow() {
		OmniShadowData shadow;
		shadow.resolution = g_config.r_shadowRes;
		shadow.shader = AssetManager::g_ShaderManager.getHandle("point_shadow_depth");
		generateCubeMap(shadow.cubemap, shadow.fbo, shadow.resolution);
		_point_shadows.push_back(shadow);
	}

	void ShadowPass::generateMap(uint32_t& map, uint32_t& fbo, uint32_t resolution) {
		glGenFramebuffers(1, &fbo);

		// generate texture
		glGenTextures(1, &map);
		glBindTexture(GL_TEXTURE_2D, map);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		             resolution, resolution, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		// setup texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// attach texture to the fbo
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		                       GL_TEXTURE_2D, map, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "unable to gen spotshadow");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowPass::generateCubeMap(uint32_t& cube_map, uint32_t& fbo, uint32_t resolution) {
		glGenFramebuffers(1, &fbo);
		// create depth cubemap texture
		glGenTextures(1, &cube_map);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0,
			             GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cube_map, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "unable to gen omnishadow");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	ShadowPass::~ShadowPass() {
		// glDeleteTextures(1, &map);
		// glDeleteFramebuffers(1, &fbo);
	}
}
