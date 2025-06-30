// it is what it is
#include <iostream>
#include <array>
#include <algorithm>

#include "render/shader.hpp"
#include "render/model.hpp"
#include "render/renderer.hpp"
#include "game/scene.hpp"
#include "game/resource_manager.hpp"
#include "game/config.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <numeric>

#include "stb_image_write.h"
#include "util.hpp"

// TODO make it json or smth
static Scene scene{
	.camera{
		.position = {0, 0, 0},
		.target = {0, 17, 0},

		.radius = 30.f,

		.yaw = 90.f,
		.pitch = 45.f,

		.lastx = 400,
		.lasty = 300,

		.sens = 0.2f,
		.fov = 45.f,

		.locked = true
	},
	.dirLight{
		.enable = false,
		.direction = {-0.5f, -1.0f, -0.5f},
		.ambient = glm::vec3(0.3f),
		.diffuse = glm::vec3(0.8f),
		.specular = glm::vec3(0.5f)
	},
	.pointLight{
		.enable = false,
		.position = {0.f, 35.f, 0.f},

		.constant = 1.f,
		// .linear = 0.09f,
		// .quadratic = 0.032f,
		.linear = 0,
		.quadratic = 0.004f,

		.ambient = {0.8f, 0.8f, 1.f},
		.diffuse = {0.6f, 0.6f, 1.f},
		.specular = {1.f, 1.f, 1.f},
	},
	.spotLight{
		.enable = false,
		.position = {3, 45, 30},
		.direction = {0, -0.5, -0.5},

		.cutOff = glm::cos(glm::radians(12.5f)), // cos
		.outerCutOff = glm::cos(glm::radians(17.5f)), // cos

		.constant = 1.f,
		.linear = 0.09f,
		.quadratic = 0.001f,

		.ambient = glm::vec3(0.f),
		.diffuse = glm::vec3(1.f),
		.specular = glm::vec3(1.f),
	}
};
static Render::PostEffects effects{
	.quantization = false,
	.quantizationLevel = 4,

	.vignette = true,
	.vignetteIntensity = 0.25f,
	.vignetteColor = glm::vec3(0)
};
static Config config{
	.windowRes = {1280, 720},
	.renderRes = {1920, 1080}, // 320, 240
	.shadowRes = 2048,

	// --- GRAPHICS ---
	.renderDistance = 1000.f,
	.vsync = true,
	.fillColor = {0.3, 0.3, 0.3}
};

static bool FLASHLIGHT = false;

static GLFWwindow* window = nullptr;
static Render::Renderer* renderer = nullptr;

static std::array<float, 256> FPS = {0};
static bool DEBUG_INFO = false;

void DrawDebugInfo() {
	ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_NoTitleBar);

	const float largest_num = *(std::ranges::max_element(FPS));
	const float avg_num = std::accumulate(FPS.begin(), FPS.end(), 0.0) / FPS.size();

	ImGui::PlotLines(" ", FPS.data(), FPS.size(), 0, ("avg: " + std::to_string(avg_num)).c_str(), 0, largest_num + 200,
	                 {300, 70});

	ImGui::End(); // End general options
}

void DrawOptions() {
	// General Options
	ImGui::Begin("General Options");

	if (ImGui::CollapsingHeader("Post Processing")) {
		ImGui::Checkbox("Quantization", &effects.quantization);
		ImGui::InputInt("Level", &effects.quantizationLevel);
		ImGui::SeparatorText("");
		ImGui::Checkbox("Vignette", &effects.vignette);
		ImGui::SliderFloat("Intensity", &effects.vignetteIntensity, 0, 10, "%.2f");
		ImGui::ColorEdit3("Color", &effects.vignetteColor[0]);
	}

	if (ImGui::CollapsingHeader("Render Settings")) {
		//ImGui::ColorEdit3("Void Color", &voidColor[0]);
		if (ImGui::InputInt2("Render Resolution", &config.renderRes[0])) {
			renderer->updateRenderRes();
		}
		ImGui::ColorPicker3("Fill color", &config.fillColor[0]);
		// if (ImGui::InputInt("Shadow Resolution", &config.shadowRes)) {
		// renderer->updateShadowRes();
		// }
	}

	if (ImGui::CollapsingHeader("Camera Settings")) {
		ImGui::Text("Position: x: %f, y: %f, z: %f", scene.camera.position.x, scene.camera.position.y,
		            scene.camera.position.z);
		ImGui::SliderFloat("Field Of View", &scene.camera.fov, 0, 100, "%.2f");
		ImGui::InputFloat3("Target", &scene.camera.target[0], "%.2f");
		ImGui::SliderFloat("Sensitivity", &scene.camera.sens, 0, 10, "%.1f");
	}

	if (ImGui::CollapsingHeader("Light Settings")) {
		if (ImGui::TreeNode("Directional Light")) {
			ImGui::SliderInt("Enable", &scene.dirLight.enable, 0, 1);

			ImGui::BeginDisabled(!scene.dirLight.enable);

			//ImGui::ColorEdit3("Color", &scene.dirLight.color[0]);
			ImGui::InputFloat3("Direction", &scene.dirLight.direction[0]);
			ImGui::InputFloat3("Ambient", &scene.dirLight.ambient[0]);
			ImGui::InputFloat3("Diffuse", &scene.dirLight.diffuse[0]);
			ImGui::InputFloat3("Specular", &scene.dirLight.specular[0]);

			ImGui::EndDisabled();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Point Light")) {
			ImGui::SliderInt("Enable", &scene.pointLight.enable, 0, 1);

			ImGui::BeginDisabled(!scene.pointLight.enable);

			//ImGui::ColorEdit3("Color", &scene.pointLight.color[0]);

			ImGui::InputFloat3("Position", &scene.pointLight.position[0]);
			ImGui::InputFloat3("Ambient", &scene.pointLight.ambient[0]);
			ImGui::InputFloat3("Diffuse", &scene.pointLight.diffuse[0]);
			ImGui::InputFloat3("Specular", &scene.pointLight.specular[0]);

			ImGui::InputFloat("Constant", &scene.pointLight.constant);
			ImGui::InputFloat("Linear", &scene.pointLight.linear);
			ImGui::InputFloat("Quadratic", &scene.pointLight.quadratic);

			ImGui::EndDisabled();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Spot Light")) {
			ImGui::SliderInt("Enable", &scene.spotLight.enable, 0, 1);

			ImGui::BeginDisabled(!scene.spotLight.enable);

			//ImGui::ColorEdit3("Color", &scene.spotLight.color[0]);

			ImGui::InputFloat3("Position", &scene.spotLight.position[0]);
			ImGui::InputFloat3("Direction", &scene.spotLight.direction[0]);

			ImGui::InputFloat3("Ambient", &scene.spotLight.ambient[0]);
			ImGui::InputFloat3("Diffuse", &scene.spotLight.diffuse[0]);
			ImGui::InputFloat3("Specular", &scene.spotLight.specular[0]);

			ImGui::InputFloat("Constant", &scene.spotLight.constant);
			ImGui::InputFloat("Linear", &scene.spotLight.linear);
			ImGui::InputFloat("Quadratic", &scene.spotLight.quadratic);

			ImGui::SliderAngle("CutOff", &scene.spotLight.cutOff, 0, 180);
			ImGui::SliderAngle("OuterCutOff", &scene.spotLight.outerCutOff, 0,
			                   glm::degrees(scene.spotLight.cutOff) - 1);

			ImGui::EndDisabled();
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Objects Settings")) {
		for (const auto& model : scene.objects) {
			if (ImGui::TreeNode(model->name.c_str())) {
				ImGui::Checkbox("Cast Shadow", &model->castShadow);
				ImGui::InputFloat3("Scale", &model->transform.scale[0]);
				ImGui::InputFloat3("Position", &model->transform.position[0]);
				ImGui::InputFloat3("Rotation", &model->transform.rotation[0]);
				//
				// if (ImGui::TreeNode("Meshes")) {
				// 	if (ImGui::Button("Disable all")) {
				// 		for (const auto& mesh : model->meshes) { mesh->drawable = false; }
				// 	}
				// 	ImGui::SameLine();
				// 	if (ImGui::Button("Enable all")) {
				// 		for (const auto& mesh : model->meshes) { mesh->drawable = true; }
				// 	}
				//
				// 	for (const auto& mesh : model->meshes) {
				// 		if (ImGui::TreeNode(mesh->name.c_str())) {
				// 			ImGui::Checkbox("Draw", &mesh->drawable);
				//
				// 			ImGui::BeginDisabled(!mesh->drawable);
				// 			ImGui::Text("Mat: %s", mesh->material->name.c_str());
				//
				// 			ImGui::InputFloat3("Position", &mesh->transform.position[0]);
				// 			ImGui::SliderFloat3("Scale", &mesh->transform.scale[0], 1, 10, "%.1f");
				// 			ImGui::InputFloat3("Rotation", &mesh->transform.rotation[0]);
				//
				// 			ImGui::EndDisabled();
				//
				// 			ImGui::TreePop();
				// 		}
				// 	}
				// 	ImGui::TreePop();
				// }
				ImGui::TreePop();
			}
		}
	}

	ImGui::End(); // End general options
}

void SetStyleIMGUI() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Setting up a dark theme base
	// colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.6f); // Semi-transparent dark background
	colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f); // Semi-transparent dark background
	colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.8f);
	colors[ImGuiCol_Border] = ImVec4(0.8f, 0.8f, 0.8f, 0.f);

	// Text and frames
	colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.2f); // Semi-transparent for frosted look
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.7f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);

	// Header
	colors[ImGuiCol_Header] = ImVec4(0.3f, 0.3f, 0.3f, 0.1f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.1f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.1f);

	// Buttons
	colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 0.6f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.8f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	colors[ImGuiCol_PlotLines] = ImVec4(0.5, 0.5, 0.5, 1.0f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1, 1, 1, 1.0f);

	colors[ImGuiCol_TitleBgActive] = ImVec4(0, 0, 0, 0.3);
	colors[ImGuiCol_TitleBg] = ImVec4(0, 0, 0, 0.3);

	colors[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0.3);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0, 0, 0, 0.5);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0, 0, 0, 0.6);

	// Adjust window rounding and padding to enhance the glass look
	style.WindowRounding = 10.0f;
	style.FrameRounding = 5.0f;
	style.WindowPadding = ImVec2(10, 10);
	// style.FramePadding = ImVec2(5, 5);
}

void InitIMGUI() {
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

	// Setup ImGui style
	ImGui::StyleColorsDark();
	SetStyleIMGUI();

	// io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSans-Medium.ttf", 18.f);

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void SaveScreenshot(const char* filename) {
	// Выделение памяти под пиксели (формат RGB)
	std::vector<unsigned char> pixels(config.windowRes.x * config.windowRes.y * 3);

	// Настройка параметров чтения пикселей
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // Убираем выравнивание
	glReadBuffer(GL_FRONT); // Читаем из переднего буфера (для двойной буферизации)

	// Чтение пикселей из буфера
	glReadPixels(0, 0, config.windowRes.x, config.windowRes.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	// Переворот изображения по вертикали (OpenGL хранит пиксели снизу вверх)
	std::vector<unsigned char> flippedPixels(pixels.size());
	for (int y = 0; y < config.windowRes.y; ++y) {
		for (int x = 0; x < config.windowRes.x; ++x) {
			for (int c = 0; c < 3; ++c) {
				flippedPixels[(config.windowRes.y - 1 - y) * config.windowRes.x * 3 + x * 3 + c] =
					pixels[y * config.windowRes.x * 3 + x * 3 + c];
			}
		}
	}

	// Сохранение в PNG
	stbi_write_png(filename, config.windowRes.x, config.windowRes.y, 3, flippedPixels.data(), config.windowRes.x * 3);
	std::cout << OUT_INFO << "Screenshot saved as " << filename << std::endl;
}

void MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
	scene.camera.mouseMoved(xpos, ypos);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		scene.camera.locked = false;
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		scene.camera.locked = true;
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	} else if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		scene.camera = Camera();
	} else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		SaveScreenshot("frame.png");
	} else if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
		DEBUG_INFO = !DEBUG_INFO;
	} else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		std::cout << OUT_DEBUG << "Flashlight: " << (FLASHLIGHT ? "on" : "off") << std::endl;
		FLASHLIGHT = !FLASHLIGHT;
		scene.spotLight.enable = FLASHLIGHT;
	} else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		scene.pointLight.enable = !scene.pointLight.enable;
	}
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	scene.camera.mouseScrolled(yoffset, config.renderDistance);
}

void ResizeCallback(GLFWwindow* window, int width, int height) {
	if (width == 0 && height == 0) return; // in case of minimizing
	config.windowRes = {width, height};
	glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                            const char* message, const void* userParam) {
	// ignore non-significant error/warning codes
	// if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
	if (id == 131204 || id == 131185) return;

	std::cout << OUT_DEBUG << "(";

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "\033[31mhigh\033[0m";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "\033[33mmedium\033[0m";
		break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "\033[32mlow\033[0m";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "\033[34mnotification\033[0m";
		break;
	}
	std::cout << "; ";

	switch (source) {
	case GL_DEBUG_SOURCE_API: std::cout << "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER: std::cout << "Other";
		break;
	}
	std::cout << "; ";

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: std::cout << "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Deprecated Behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Undefined Behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER: std::cout << "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "Other";
		break;
	}

	std::cout << "): " << message << " (" << id << ")" << std::endl;
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

	window = glfwCreateWindow(config.windowRes.x, config.windowRes.y, "chess3d", nullptr, nullptr);

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MousePosCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetFramebufferSizeCallback(window, ResizeCallback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(config.vsync); // vsync 1 - on; 0 - off

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

Render::MeshPtr CreateSphereMesh(float radius, uint32_t stackCount, uint32_t sliceCount) {
	Render::MeshPtr mesh = std::make_shared<Render::Mesh>();
	mesh->name = "sphere";

	// Генерация вершин
	mesh->vertices.push_back({
		glm::vec3(0.0f, radius, 0.0f), // position
		glm::vec3(0.0f, 1.0f, 0.0f), // normal
		glm::vec2(0.5f, 0.0f) // tex_coords
	});

	const float pi = glm::pi<float>();
	for (uint32_t stack = 1; stack < stackCount; ++stack) {
		float theta = stack * pi / stackCount;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);

		for (uint32_t slice = 0; slice <= sliceCount; ++slice) {
			float phi = slice * 2.0f * pi / sliceCount;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			Render::Vertex v;
			v.pos = glm::vec3(
				radius * sinTheta * cosPhi,
				radius * cosTheta,
				radius * sinTheta * sinPhi
			);
			v.normal = glm::normalize(v.pos);
			v.texCoords = glm::vec2(
				static_cast<float>(slice) / sliceCount,
				static_cast<float>(stack) / stackCount
			);

			mesh->vertices.push_back(v);
		}
	}

	mesh->vertices.push_back({
		glm::vec3(0.0f, -radius, 0.0f), // position
		glm::vec3(0.0f, -1.0f, 0.0f), // normal
		glm::vec2(0.5f, 1.0f) // tex_coords
	});

	// Генерация индексов (counter-clockwise порядок)
	const uint32_t poleStart = 0;
	const uint32_t ringVertexCount = sliceCount + 1;

	// Верхний полюс
	for (uint32_t slice = 0; slice < sliceCount; ++slice) {
		mesh->indices.push_back(poleStart);
		mesh->indices.push_back(1 + (slice + 1) % sliceCount);
		mesh->indices.push_back(1 + slice);
	}

	// Основные кольца
	for (uint32_t stack = 0; stack < stackCount - 2; ++stack) {
		uint32_t ringStart = 1 + stack * ringVertexCount;
		uint32_t nextRingStart = ringStart + ringVertexCount;

		for (uint32_t slice = 0; slice < sliceCount; ++slice) {
			// Первый треугольник квада (counter-clockwise)
			mesh->indices.push_back(ringStart + slice);
			mesh->indices.push_back(ringStart + slice + 1);
			mesh->indices.push_back(nextRingStart + slice);

			// Второй треугольник квада (counter-clockwise)
			mesh->indices.push_back(nextRingStart + slice);
			mesh->indices.push_back(ringStart + slice + 1);
			mesh->indices.push_back(nextRingStart + slice + 1);
		}
	}

	// Нижний полюс
	const uint32_t bottomPoleIndex = static_cast<uint32_t>(mesh->vertices.size() - 1);
	const uint32_t lastRingStart = 1 + (stackCount - 2) * ringVertexCount;

	for (uint32_t slice = 0; slice < sliceCount; ++slice) {
		mesh->indices.push_back(bottomPoleIndex);
		mesh->indices.push_back(lastRingStart + slice);
		mesh->indices.push_back(lastRingStart + slice + 1);
	}

	mesh->setup();

	return mesh;
}

Render::MeshPtr CreatePlaneMesh(float shininess, const std::string& name) {
	Render::MeshPtr mesh = std::make_shared<Render::Mesh>();;
	Render::MaterialPtr mat = std::make_shared<Render::Material>();

	// setup material

	mat->name = name;
	mat->diffuse[0] = ResourceManager::CreateDefaultTexture({200, 200, 200}, {200, 200, 200}); // TODO solid color
	mat->shininess = shininess;

	// setup mesh

	mesh->name = name;

	mesh->vertices = {
		{{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
	};

	mesh->indices = {0, 2, 1, 0, 3, 2};
	mesh->material = mat;

	mesh->setup();

	return mesh;
}

void setupScene(bool props) {
	// setup chess set

	Render::ShaderPtr shader = std::make_shared<Render::Shader>("assets/shaders/scene.vert",
	                                                            "assets/shaders/scene.frag");

	if (props) {
		// Render::ModelPtr lightBulb = CreateLightSphere(0.1, 32, 32);
		Render::MeshPtr lightBulb = CreateSphereMesh(0.1, 32, 32);
		lightBulb->castShadow = false;
		lightBulb->name = "LightBulb";
		lightBulb->material = std::make_shared<Render::Material>();
		lightBulb->material->shader = std::make_shared<Render::Shader>("assets/shaders/light.vert",
		                                                               "assets/shaders/light.frag");
		lightBulb->material->shininess = 255;
		scene.objects.push_back(lightBulb);

		Render::ModelPtr toy = ResourceManager::LoadModel("assets/models/slayer_toy.obj", shader);
		toy->transform = {
			.scale = glm::vec3(0.3f),
			.rotation = {-90, 0, -45},
			.position = {-5, 16, 0}
		};
		scene.objects.push_back(toy);

		// Render::ModelPtr chess = ResourceManager::LoadModel("assets/models/chess.fbx", shader);
		// chess->transform = {
		// .position = {-7, 16.28, 0},
		// .scale = glm::vec3(0.3)
		// };
		// scene.objects.push_back(chess);

		Render::ModelPtr desk = ResourceManager::LoadModel("assets/models/desk.obj", shader);
		desk->transform.scale = {20, 20, 20};
		scene.objects.push_back(desk);
	}

	// create floor

	Render::MeshPtr floor = CreatePlaneMesh(8, "floor_plane");
	floor->transform.scale = glm::vec3(config.renderDistance);
	floor->material->shader = shader;
	floor->castShadow = false;
	scene.objects.push_back(floor);
}

int main(int argc, char** argv) {
	InitGLFW();
	InitIMGUI();

 	// TODO make loading screen w/ progresbar
	setupScene(true);

	renderer = new Render::Renderer(config);

	double lastTime = glfwGetTime();
	int frameCount = 0;
	uint8_t indexFPS = 0;

	renderer->genShadowMaps(scene);
	while (!glfwWindowShouldClose(window)) {
		if (DEBUG_INFO) {
			frameCount++;
			FPS[indexFPS] = frameCount / (glfwGetTime() - lastTime);
			indexFPS++;
			frameCount = 0;
			lastTime = glfwGetTime();
		}
		if (scene.pointLight.enable) {
			// scene.pointLight.position.z = static_cast<float>(sin(glfwGetTime()) * 3.0) * 3; // 3.0 speed, 3 radius
			// scene.pointLight.position.x = static_cast<float>(cos(glfwGetTime()) * 3.0) * 3;
			// scene.models[0]->meshes[0]->drawable = true;
			// scene.models[0]->transform.position = scene.pointLight.position;
			renderer->genShadowMaps(scene); // some sort of optimization
		} else {
			// scene.objects[0]->meshes[0]->drawable = false;
		}

		if (FLASHLIGHT) {
			scene.spotLight.position = scene.camera.position;
			scene.spotLight.position.y -= 2;
			scene.spotLight.position.x -= 2;

			scene.spotLight.direction = glm::normalize(scene.camera.target - scene.camera.position);
			renderer->genShadowMaps(scene); // some sort of optimization
		}

		scene.camera.updatePosition();
		renderer->drawScene(scene);
		renderer->renderFrame(effects);

		// render imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		DrawOptions();
		if (DEBUG_INFO) DrawDebugInfo();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	delete renderer;

	glfwTerminate();
	return 0;
}
