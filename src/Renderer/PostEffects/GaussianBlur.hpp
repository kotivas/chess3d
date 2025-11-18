#pragma once
#include <cstdint>
#include "../Shader.hpp"
#include <glad/glad.h>

namespace Renderer::PostEffects {
	class GaussianBlur {
	public:
		GaussianBlur() = default;

		void init(int width, int height, uint32_t quadVAO, uint32_t quadVBO);
		uint32_t blur(const uint32_t& input, int passes) const;

		~GaussianBlur();

	private:
		unsigned int _pingpongFBO[2];
		unsigned int _pingpongBuffers[2];
		uint32_t _quadVAO;
		uint32_t _quadVBO;
	};
}
