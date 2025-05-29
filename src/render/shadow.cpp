#include "shadow.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glad/glad.h>

#include <iostream>

namespace Render {
	void DirShadowData::generate() {
		glGenFramebuffers(1, &shadowMapFBO);

		// generate texture
		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		             resolution, resolution, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		// setup texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// attach texture to the fbo
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		                       GL_TEXTURE_2D, shadowMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "unable to gen dirshadow");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DirShadowData::calculateLightSpaceMatrix(const glm::vec3& lightDir, const float nearPlane,
	                                              const float farPlane) {
		const glm::mat4 lightProjection = glm::ortho(-100.f, 100.f, -100.f, 100.f, nearPlane, farPlane);
		const glm::vec3 lightPos = -lightDir * glm::vec3(100);
		// HACK shadow casts incorrectly, if light falls vertically downward
		const glm::vec3 lightUp = glm::abs(lightDir.y) < 0.999 ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);

		lightSpaceMatrix = lightProjection * glm::lookAt(lightPos, lightPos + lightDir, lightUp);
	}

	DirShadowData::~DirShadowData() {
		glDeleteTextures(1, &shadowMap);
		glDeleteFramebuffers(1, &shadowMapFBO);
	}

	void SpotShadowData::generate() {
		glGenFramebuffers(1, &shadowMapFBO);

		// generate texture
		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		             resolution, resolution, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		// setup texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// attach texture to the fbo
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		                       GL_TEXTURE_2D, shadowMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "unable to gen spotshadow");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SpotShadowData::calculateLightSpaceMatrix(const glm::vec3& lightPos, const glm::vec3& lightDir, float fov,
	                                               float nearPlane, float farPlane) {
		glm::mat4 lightProjection = glm::perspective(fov, 1.f, 10.f, farPlane);
		glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, {0.f, 1.f, 0.f});


		lightSpaceMatrix = lightProjection * lightView;
	}

	SpotShadowData::~SpotShadowData() {
		glDeleteTextures(1, &shadowMap);
		glDeleteFramebuffers(1, &shadowMapFBO);
	}

	void OmniShadowData::generate() {
		glGenFramebuffers(1, &shadowCubemapFBO);
		// create depth cubemap texture
		glGenTextures(1, &shadowCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0,
			             GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowCubemapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "unable to gen omnishadow");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OmniShadowData::genTransformMatrixes(const glm::vec3 lightPos, float nearPlane, float farPlane) {
        const glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)resolution / (float)resolution, nearPlane, farPlane);
        transforms[0] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        transforms[1] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        transforms[2] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        transforms[3] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        transforms[4] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        transforms[5] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	}

	OmniShadowData::~OmniShadowData() {
		// delete cubemap
		glDeleteFramebuffers(1, &shadowCubemapFBO);
	}
}
