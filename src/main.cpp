// it is what it is
#include <algorithm>
#include <array>
#include <string>

#include "game/config.hpp"
#include "resourcemgr/resourcemgr.hpp"
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
#include "./text/MSDFText.hpp"
#include "console/console.hpp"
#include "core/logger.hpp"

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

static bool g_isFlashlight = false;
static char g_imguilog[256];
static std::array<float, 256> g_fpsarray = {0};
static bool g_isDebugInfo = false;

void DrawDebugInfo() {
	ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_NoTitleBar);

	const float largest_num = *(std::ranges::max_element(g_fpsarray));
	const float avg_num = std::accumulate(g_fpsarray.begin(), g_fpsarray.end(), 0.0) / g_fpsarray.size();

	ImGui::PlotLines(" ", g_fpsarray.data(), g_fpsarray.size(), 0, ("avg: " + std::to_string(avg_num)).c_str(), 0,
	                 largest_num + 200,
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
		ImGui::SliderFloat("gamma", &effects.gamma, 0, 10, "%.1f");
		if (ImGui::InputInt2("Render Resolution", &g_config.r_resolution[0])) {
			Renderer::UpdateRenderRes();
		}
		ImGui::ColorPicker3("Fill color", &g_config.r_fillColor[0]);
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

	if (ImGui::InputText("Log::Info", g_imguilog, 256)) {
		Log::Info(g_imguilog);
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
	std::vector<unsigned char> pixels(g_config.sys_windowResolution.x * g_config.sys_windowResolution.y * 3);

	// Настройка параметров чтения пикселей
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // Убираем выравнивание
	glReadBuffer(GL_FRONT); // Читаем из переднего буфера (для двойной буферизации)

	// Чтение пикселей из буфера
	glReadPixels(0, 0, g_config.sys_windowResolution.x, g_config.sys_windowResolution.y, GL_RGB, GL_UNSIGNED_BYTE,
	             pixels.data());

	// Переворот изображения по вертикали (OpenGL хранит пиксели снизу вверх)
	std::vector<unsigned char> flippedPixels(pixels.size());
	for (int y = 0; y < g_config.sys_windowResolution.y; ++y) {
		for (int x = 0; x < g_config.sys_windowResolution.x; ++x) {
			for (int c = 0; c < 3; ++c) {
				flippedPixels[(g_config.sys_windowResolution.y - 1 - y) * g_config.sys_windowResolution.x * 3 + x * 3 +
						c] =
					pixels[y * g_config.sys_windowResolution.x * 3 + x * 3 + c];
			}
		}
	}

	// Сохранение в PNG
	stbi_write_png(filename, g_config.sys_windowResolution.x, g_config.sys_windowResolution.y, 3, flippedPixels.data(),
	               g_config.sys_windowResolution.x * 3);
	Log::Info("Screenshot saved as " + std::string(filename));
}

void updateControls() {
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(Renderer::g_window, GL_TRUE);
	} else if (Input::IsKeyPressed(GLFW_KEY_F11)) {
		SaveScreenshot("frame.png");
	} else if (Input::IsKeyPressed(GLFW_KEY_F12)) {
		g_isDebugInfo = !g_isDebugInfo;
	} else if (Input::IsKeyPressed(GLFW_KEY_F)) {
		g_isFlashlight = !g_isFlashlight;
		Log::Debug("Flashlight: " + std::string(g_isFlashlight ? "on" : "off"));
		scene.spotLight.enable = g_isFlashlight;
	} else if (Input::IsKeyPressed(GLFW_KEY_P)) {
		scene.pointLight.enable = !scene.pointLight.enable;
		Log::Info(std::string(200, '#'));
	}

	if (Input::IsKeyPressed(GLFW_KEY_GRAVE_ACCENT)) Console::Toggle();

	if (Input::IsRightMouseDown()) {
		scene.camera.locked = false;
	} else {
		scene.camera.locked = true;
	}

	if (Input::g_resizedHeight || Input::g_resizedWidth) {
		if (Input::g_resizedWidth == 0 && Input::g_resizedHeight == 0) return; // in case of minimizing
		g_config.sys_windowResolution = {Input::g_resizedWidth, Input::g_resizedHeight};
		// g_config.renderRes = {Input::g_resizedWidth, Input::g_resizedHeight};
		// Renderer::UpdateRenderRes();
		glViewport(0, 0, Input::g_resizedWidth, Input::g_resizedHeight);
	}

	scene.camera.mouseMoved(Input::GetMouseX(), Input::GetMouseY());
	if (!Console::IsVisible()) scene.camera.mouseScrolled(Input::GetScrollYOffset(), g_config.r_renderDistance);
}

void setupScene(bool props) {
	// setup chess set

	if (props) {
		// Render::ModelPtr lightBulb = CreateLightSphere(0.1, 32, 32);
		// Render::MeshPtr lightBulb = Util::CreateSphereMesh(0.1, 32, 32);
		// lightBulb->castShadow = false;
		// lightBulb->name = "LightBulb";
		// lightBulb->material = std::make_shared<Render::Material>();
		// lightBulb->material->shader = std::make_shared<Render::Shader>("shaders/light.vert",
		//                                                                "shaders/light.frag");
		// lightBulb->material->shininess = 255;
		// scene.objects.push_back(lightBulb);

		Render::ModelPtr toy = ResourceMgr::GetModelByName("slayer_toy");
		toy->transform = {
			.position = {-5, 16, 0},
			.rotation = {-90, 0, -45},
			.scale = glm::vec3(0.3f)
		};
		scene.objects.push_back(toy);

		Render::ModelPtr desk = ResourceMgr::GetModelByName("desk");
		desk->transform.scale = {20, 20, 20};
		scene.objects.push_back(desk);
	}

	// create floor

	Render::MeshPtr floor = Util::CreatePlaneMesh(8, "floor_plane");
	floor->transform.scale = glm::vec3(g_config.r_renderDistance);
	floor->material->shader = ResourceMgr::GetShaderByName("scene");
	floor->castShadow = false;
	scene.objects.push_back(floor);
}

void LoadAll() {
	ResourceMgr::LoadMSDFFont("inconsolata_light", "assets/fonts/inconsolata_light.png",
	                          "assets/fonts/inconsolata_light.json");

	// ALL SHADERS
	ResourceMgr::LoadShader("postfx", "shaders/postfx.vert",
	                        "shaders/postfx.frag");
	ResourceMgr::LoadShader("depth", "shaders/depth.vert",
	                        "shaders/depth.frag");
	ResourceMgr::LoadShader("point_shadow_depth", "shaders/point_shadow_depth.vert", "shaders/point_shadow_depth.frag",
	                        "shaders/point_shadow_depth.geom");
	ResourceMgr::LoadShader("msdf_text", "shaders/msdf_text.vert", "shaders/msdf_text.frag");
	ResourceMgr::LoadShader("scene", "shaders/scene.vert", "shaders/scene.frag");
	ResourceMgr::LoadShader("solidcolor", "shaders/solidcolor.vert", "shaders/solidcolor.frag");

	// ALL MODELS

	ResourceMgr::LoadModel("slayer_toy", "assets/models/slayer_toy.obj", ResourceMgr::GetShaderByName("scene"));
	ResourceMgr::LoadModel("desk", "assets/models/desk.obj", ResourceMgr::GetShaderByName("scene"));
}

int main(int argc, char** argv) {
	g_config = {
		.sys_windowResolution = {1280, 720},

		.r_resolution = {2560, 1440}, // 2x sampling
		.r_shadowRes = 2048,
		.r_renderDistance = 1000.f,
		.r_vsync = true,
		.r_fillColor = {0.3, 0.3, 0.3},

		.con_fontScale = 32,
		.con_maxVisibleLines = 20,
		.con_backgroundColor = {0, 0, 0, 0.9},
	};
	Log::Init();
	Log::SetSeverity(Logger::Severity::Debug);

	Renderer::Init();
	LoadAll();
	Input::Init();
	Console::Init();
	InitIMGUI();
	MSDFText::Init();

	// TODO make loading screen w/ progresbar
	setupScene(true);

	double lastTime = glfwGetTime();
	int frameCount = 0;
	uint8_t indexFPS = 0;

	while (!glfwWindowShouldClose(Renderer::g_window)) {
		if (g_isDebugInfo) {
			frameCount++;
			g_fpsarray[indexFPS] = frameCount / (glfwGetTime() - lastTime);
			indexFPS++;
			frameCount = 0;
			lastTime = glfwGetTime();
		}

		if (g_isFlashlight) {
			scene.spotLight.position = scene.camera.position;
			scene.spotLight.position.y -= 2;
			scene.spotLight.direction = glm::normalize(scene.camera.target - scene.camera.position);
		}

		scene.camera.updatePosition();
		updateControls();
		Console::Update();
		Input::Update();

		Renderer::GenShadowMaps(scene);

		Renderer::FrameBegin(scene);
		for (const auto& object : scene.objects) object->draw({});
		Console::Draw();
		Renderer::FrameEnd(effects);

		// render imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		DrawOptions();
		if (g_isDebugInfo) DrawDebugInfo();
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
