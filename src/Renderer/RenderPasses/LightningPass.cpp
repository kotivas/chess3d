#include "LightningPass.hpp"

#include "AssetManager/AssetManager.hpp"
#include "Core/Logger.hpp"

namespace Renderer {
	void LightningPass::init() {
		createUboMatrices();
		createUboLights();
	}

	void LightningPass::createUboMatrices() {
		glGenBuffers(1, &_ubo_matrices);

		glBindBuffer(GL_UNIFORM_BUFFER, _ubo_matrices);
		glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, _ubo_matrices, 0, 4 * sizeof(glm::mat4));
	}

	void LightningPass::createUboLights() {
		glGenBuffers(1, &_ubo_lights);

		static_assert(sizeof(DirLight) == 80);
		static_assert(sizeof(PointLight) == 96);
		static_assert(sizeof(SpotLight) == 112);

		glBindBuffer(GL_UNIFORM_BUFFER, _ubo_lights);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirLight) + sizeof(PointLight) + sizeof(SpotLight), NULL,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, 1, _ubo_lights, 0,
		                  sizeof(DirLight) + sizeof(PointLight) + sizeof(SpotLight));
	}

	void LightningPass::updateUboMatrices(const glm::mat4& projection, const glm::mat4& view,
	                                      const glm::mat4& dir_light_space_matrix,
	                                      const glm::mat4& spot_light_space_matrix) const {
		glBindBuffer(GL_UNIFORM_BUFFER, _ubo_matrices);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4),
		                glm::value_ptr(dir_light_space_matrix));
		glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), sizeof(glm::mat4),
		                glm::value_ptr(spot_light_space_matrix));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void LightningPass::updateUboLights(const DirLight& dir_light, const PointLight& point_light,
	                                    const SpotLight& spot_light) const {
		glBindBuffer(GL_UNIFORM_BUFFER, _ubo_lights);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirLight), &dir_light);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirLight), sizeof(PointLight), &point_light);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(DirLight) + sizeof(PointLight), sizeof(spot_light), &spot_light);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void LightningPass::pass(const RenderContext& ctx) const {
		glViewport(0, 0, ctx.settings.renderResolution.x, ctx.settings.renderResolution.y);
		glBindFramebuffer(GL_FRAMEBUFFER, ctx.target_framebuffer);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		updateUboLights(ctx.light_info.directional, ctx.light_info.point, ctx.light_info.spot);
		updateUboMatrices(ctx.camera.projectionMatrix, ctx.camera.viewMatrix,
		                  ctx.dir_shadow.light_space, ctx.spot_shadow.light_space);

		glActiveTexture(GL_TEXTURE0);
		if (ctx.dir_shadow.shadow_map)
			glBindTexture(GL_TEXTURE_2D, ctx.dir_shadow.shadow_map);
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (ctx.spot_shadow.shadow_map)
			glBindTexture(GL_TEXTURE_2D, ctx.spot_shadow.shadow_map);
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE2);
		if (!ctx.point_shadows.empty())
			glBindTexture(GL_TEXTURE_CUBE_MAP, ctx.point_shadows[0].shadow_map);
		else
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		ShaderUniforms uniforms{
			.nearPlane = ctx.camera.nearPlane,
			.farPlane = ctx.camera.farPlane,
			.viewPos = ctx.camera.position,
			.parallaxScale = ctx.settings.parallaxScale,
		};

		for (const auto& item : ctx.render_items) drawRenderItem(item, uniforms);
	}

	void LightningPass::drawRenderItem(const RenderItem& item, const ShaderUniforms& unifs) const {
		if (!item.mesh->material) {
			Log::Error("Renderer::DrawMesh material is not exist");
			return;
		}

		const Shader* shader = AssetManager::g_ShaderManager.get(item.material->shader);

		if (!shader) {
			Log::Error("Shader {0} is not found", item.material->shader.id);
			return;
		}

		shader->use();

		shader->setUniform1f("farPlane", unifs.farPlane);
		shader->setUniform1f("nearPlane", unifs.nearPlane);
		shader->setUniform3f("viewPos", unifs.viewPos);

		shader->setUniform1f("parallaxScale", unifs.parallaxScale);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, item.material->diffuse);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, item.material->specular);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, item.material->normal);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, item.material->displacement);

		shader->setUniform1f("material.shininess", item.material->shininess);
		shader->setUniform3f("material.solidColor", item.material->solidColor);
		shader->setUniform1i("material.useDiffuse", item.material->useDiffuse);
		shader->setUniform1i("material.useSpecular", item.material->useSpecular);
		shader->setUniform1i("material.useNormal", item.material->useNormal);
		shader->setUniform1i("material.useDisplacement", item.material->useDisplacement);

		shader->setUniform1i("dirShadowMap", 0);
		shader->setUniform1i("spotShadowMap", 1);
		shader->setUniform1i("omniShadowMap", 2);
		shader->setUniform1i("material.diffuse", 3);
		shader->setUniform1i("material.specular", 4);
		shader->setUniform1i("material.normal", 5);
		shader->setUniform1i("material.displacement", 6);

		shader->setUniformMat4fv("uModel", GL_FALSE, item.world_transform);
		glBindVertexArray(item.mesh->VAO);
		glDrawElements(GL_TRIANGLES, item.mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}
}
