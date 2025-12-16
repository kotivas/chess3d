#pragma once
#include "AssetManager/Handle.hpp"
#include "Renderer/Shader.hpp"

namespace Renderer {
	struct Material {
		std::string name{"undefined"};

		uint32_t diffuse{0};
		bool useDiffuse{false};
		uint32_t specular{0};
		bool useSpecular{false};
		uint32_t normal{0};
		bool useNormal{false};

		float shininess{0};
		AssetManager::ShaderHandle shader;
		glm::vec3 solidColor{0, 0, 0};
		//GLuint dissolve;  // 1 == opaque; 0 == fully transparent
	};

	using MaterialPtr = std::shared_ptr<Material>;
}
