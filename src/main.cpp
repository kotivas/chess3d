// it is what it is
#include <algorithm>
#include <array>
#include <iostream>

#include "game/config.hpp"
#include "game/resource_manager.hpp"
#include "game/scene.hpp"
#include "render/model.hpp"
#include "render/renderer.hpp"
#include "render/shader.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <numeric>

#include "./input/input.hpp"

#include "stb_image_write.h"
#include "util.hpp"
#include "./render/text/MSDFText.hpp"

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
		.enable = true,
		.direction = {-0.5f, -0.2f, -1.f},
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
	.gamma = 2.2,

	.quantization = false,
	.quantizationLevel = 4,

	.vignette = true,
	.vignetteIntensity = 0.25f,
	.vignetteColor = glm::vec3(0)
};

static bool FLASHLIGHT = false;

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

void DrawKeymap() {
	ImGui::Begin("Input");

	const int cols = 30; // how many bits per row
	const float size = 10.0f; // button size in pixels

	for (size_t i = 32; i < 350; ++i) {
		ImGui::PushID(static_cast<int>(i));

		bool state = Input::g_keydownmap[i];
		ImVec4 color = state ? ImVec4(0.3f, 0.8f, 0.3f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

		ImGui::Button(" ", ImVec2(size, size));

		ImGui::PopStyleColor(3);
		ImGui::PopID();

		if ((i + 1) % cols != 0)
			ImGui::SameLine();
	}

	ImGui::Separator();
	ImGui::Text("Offset scroll y %f", Input::g_scrollYOffset);

	ImGui::End();
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
		ImGui::SliderFloat("gamma", &effects.gamma, 0, 10, "%.1f");
		if (ImGui::InputInt2("Render Resolution", &g_config.renderRes[0])) {
			Renderer::UpdateRenderRes();
		}
		ImGui::ColorPicker3("Fill color", &g_config.fillColor[0]);
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

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(Renderer::g_window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void SaveScreenshot(const char* filename) {
	// Выделение памяти под пиксели (формат RGB)
	std::vector<unsigned char> pixels(g_config.windowRes.x * g_config.windowRes.y * 3);

	// Настройка параметров чтения пикселей
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // Убираем выравнивание
	glReadBuffer(GL_FRONT); // Читаем из переднего буфера (для двойной буферизации)

	// Чтение пикселей из буфера
	glReadPixels(0, 0, g_config.windowRes.x, g_config.windowRes.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	// Переворот изображения по вертикали (OpenGL хранит пиксели снизу вверх)
	std::vector<unsigned char> flippedPixels(pixels.size());
	for (int y = 0; y < g_config.windowRes.y; ++y) {
		for (int x = 0; x < g_config.windowRes.x; ++x) {
			for (int c = 0; c < 3; ++c) {
				flippedPixels[(g_config.windowRes.y - 1 - y) * g_config.windowRes.x * 3 + x * 3 + c] =
					pixels[y * g_config.windowRes.x * 3 + x * 3 + c];
			}
		}
	}

	// Сохранение в PNG
	stbi_write_png(filename, g_config.windowRes.x, g_config.windowRes.y, 3, flippedPixels.data(),
	               g_config.windowRes.x * 3);
	std::cout << OUT_INFO << "Screenshot saved as " << filename << std::endl;
}

void updateControls() {
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(Renderer::g_window, GL_TRUE);
	} else if (Input::IsKeyPressed(GLFW_KEY_F11)) {
		SaveScreenshot("frame.png");
	} else if (Input::IsKeyPressed(GLFW_KEY_F12)) {
		DEBUG_INFO = !DEBUG_INFO;
	} else if (Input::IsKeyPressed(GLFW_KEY_F)) {
		std::cout << OUT_DEBUG << "Flashlight: " << (FLASHLIGHT ? "on" : "off") << std::endl;
		FLASHLIGHT = !FLASHLIGHT;
		scene.spotLight.enable = FLASHLIGHT;
	} else if (Input::IsKeyPressed(GLFW_KEY_P)) {
		scene.pointLight.enable = !scene.pointLight.enable;
	}

	if (Input::IsRightMouseDown()) {
		scene.camera.locked = false;
	} else {
		scene.camera.locked = true;
	}

	if (Input::g_resizedHeight || Input::g_resizedWidth) {
		if (Input::g_resizedWidth == 0 && Input::g_resizedHeight == 0) return; // in case of minimizing
		g_config.windowRes = {Input::g_resizedWidth, Input::g_resizedHeight};
		g_config.renderRes = {Input::g_resizedWidth, Input::g_resizedHeight};
		Renderer::UpdateRenderRes();
		glViewport(0, 0, Input::g_resizedWidth, Input::g_resizedHeight);
	}

	scene.camera.mouseMoved(Input::GetMouseX(), Input::GetMouseY());
	scene.camera.mouseScrolled(Input::GetScrollYOffset(), g_config.renderDistance);
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                            const char* message, const void* userParam) {
	// ignore non-significant error/warning codes
	// if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
	if (id == 131204 || id == 131185) return;

	std::cout << OUT_DEBUG << "(";

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "high";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "medium";
		break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "info";
		break;
	default: std::cout << "unknown";
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
	default: std::cout << "unknown";
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
	default: std::cout << "unknown";
	}

	std::cout << "): " << message << " (" << id << ")" << std::endl;
}

void setupScene(bool props) {
	// setup chess set

	Render::ShaderPtr shader = std::make_shared<Render::Shader>("shaders/scene.vert",
	                                                            "shaders/scene.frag");

	if (props) {
		// Render::ModelPtr lightBulb = CreateLightSphere(0.1, 32, 32);
		Render::MeshPtr lightBulb = Util::CreateSphereMesh(0.1, 32, 32);
		lightBulb->castShadow = false;
		lightBulb->name = "LightBulb";
		lightBulb->material = std::make_shared<Render::Material>();
		lightBulb->material->shader = std::make_shared<Render::Shader>("shaders/light.vert",
		                                                               "shaders/light.frag");
		lightBulb->material->shininess = 255;
		scene.objects.push_back(lightBulb);

		Render::ModelPtr toy = ResourceManager::LoadModel("assets/models/slayer_toy.obj", shader);
		toy->transform = {
			.position = {-5, 16, 0},
			.rotation = {-90, 0, -45},
			.scale = glm::vec3(0.3f)
		};
		scene.objects.push_back(toy);

		Render::ModelPtr desk = ResourceManager::LoadModel("assets/models/desk.obj", shader);
		desk->transform.scale = {20, 20, 20};
		scene.objects.push_back(desk);
	}

	// create floor

	Render::MeshPtr floor = Util::CreatePlaneMesh(8, "floor_plane");
	floor->transform.scale = glm::vec3(g_config.renderDistance);
	floor->material->shader = shader;
	floor->castShadow = false;
	scene.objects.push_back(floor);
}

void setGlDebugOutput() {
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
	}
}

int main(int argc, char** argv) {
	g_config = {
		.windowRes = {1280, 720},
		.renderRes = {1920, 1080}, // 320, 240
		.shadowRes = 2048,
		// --- GRAPHICS ---
		.renderDistance = 1000.f,
		.vsync = true,
		.fillColor = {0.3, 0.3, 0.3}
	};

	Renderer::Init();
	Input::Init();
	InitIMGUI();
	MSDFText::Init();

	// TODO make loading screen w/ progresbar
	setupScene(true);


	double lastTime = glfwGetTime();
	int frameCount = 0;
	uint8_t indexFPS = 0;

	MSDFText::FontPtr font = ResourceManager::LoadMSDFFont("assets/fonts/inconsolata.png",
	                                                       "assets/fonts/inconsolata.json");

	while (!glfwWindowShouldClose(Renderer::g_window)) {
		if (DEBUG_INFO) {
			frameCount++;
			FPS[indexFPS] = frameCount / (glfwGetTime() - lastTime);
			indexFPS++;
			frameCount = 0;
			lastTime = glfwGetTime();
		}

		if (FLASHLIGHT) {
			scene.spotLight.position = scene.camera.position;
			scene.spotLight.position.y -= 2;
			scene.spotLight.direction = glm::normalize(scene.camera.target - scene.camera.position);
		}

		scene.camera.updatePosition();
		updateControls();
		Input::Update();

		Renderer::GenShadowMaps(scene);

		Renderer::FrameBegin(scene);
		for (const auto& object : scene.objects) object->draw({});
		MSDFText::DrawText("dev test", font, 0, g_config.renderRes.y - font->lineHeight * 48, 32, glm::vec4(1));
		/* не ну вроде нормально вишло даже 😎🤙🏿
		приготовить и гамму пофиксить
		 */
		Renderer::FrameEnd(effects);

		// render imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		DrawOptions();
		DrawKeymap();
		if (DEBUG_INFO) DrawDebugInfo();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(Renderer::g_window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	Renderer::Shutdown();
	// Input::Shutdown();

	glfwTerminate();
	return 0;
}
