// it is what it is
#include <algorithm>
#include <array>
#include <string>
#include "com/config.hpp"
#include "game/scene.hpp"
#include "render/model.hpp"
#include "render/renderer.hpp"
#include "resourcemgr/resourcemgr.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./input/input.hpp"
#include "stb_image_write.h"
#include "./text/MSDFText.hpp"
#include "com/util.hpp"
#include "console/console.hpp"
#include "core/backend.hpp"
#include "core/cmdutils.hpp"
#include "core/cvar.hpp"
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

static bool g_isFlashlight = false;

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
	} else if (Input::IsKeyPressed(GLFW_KEY_F) && !Console::IsVisible()) {
		g_isFlashlight = !g_isFlashlight;
		Log::Debug("Flashlight: " + std::string(g_isFlashlight ? "on" : "off"));
		scene.spotLight.enable = g_isFlashlight;
	} else if (Input::IsKeyPressed(GLFW_KEY_P) && !Console::IsVisible()) {
		scene.pointLight.enable = !scene.pointLight.enable;
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

void setupScene() {
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

void RegisterCVars() {
	// --- Camera ---
	CMDUtils::Register("cam_position", "Camera position (Vec3f)", scene.camera.position);
	CMDUtils::Register("cam_target", "Camera target (Vec3f)", scene.camera.target);
	CMDUtils::Register("cam_radius", "Camera radius (Float)", scene.camera.radius, 0.f, 1000.f);
	CMDUtils::Register("cam_yaw", "Camera yaw (Float)", scene.camera.yaw, -360.f, 360.f);
	CMDUtils::Register("cam_pitch", "Camera pitch (Float)", scene.camera.pitch, -90.f, 90.f);
	CMDUtils::Register("cam_sens", "Camera sensitivity (Float)", scene.camera.sens, 0.f, 5.f);
	CMDUtils::Register("cam_fov", "Camera FOV (Float)", scene.camera.fov, 1.f, 180.f);

	CMDUtils::Register("dir_enable", "Directional light enabled (Boolean)", scene.dirLight.enable);
	CMDUtils::Register("dir_dir", "Directional light direction (Vec3f)", scene.dirLight.direction);
	CMDUtils::Register("dir_ambient", "Directional light ambient (Vec3f)", scene.dirLight.ambient);
	CMDUtils::Register("dir_diffuse", "Directional light diffuse (Vec3f)", scene.dirLight.diffuse);
	CMDUtils::Register("dir_specular", "Directional light specular (Vec3f)", scene.dirLight.specular);

	CMDUtils::Register("pt_enable", "Point light enabled (Boolean)", scene.pointLight.enable);
	CMDUtils::Register("pt_pos", "Point light position (Vec3f)", scene.pointLight.position);
	CMDUtils::Register("pt_constant", "Point light constant (Float)", scene.pointLight.constant);
	CMDUtils::Register("pt_linear", "Point light linear (Float)", scene.pointLight.linear);
	CMDUtils::Register("pt_quadratic", "Point light quadratic (Float)", scene.pointLight.quadratic);
	CMDUtils::Register("pt_ambient", "Point light ambient (Vec3f)", scene.pointLight.ambient);
	CMDUtils::Register("pt_diffuse", "Point light diffuse (Vec3f)", scene.pointLight.diffuse);
	CMDUtils::Register("pt_specular", "Point light specular (Vec3f)", scene.pointLight.specular);

	CMDUtils::Register("spot_enable", "Spot light enabled (Boolean)", scene.spotLight.enable);
	CMDUtils::Register("spot_pos", "Spot light position (Vec3f)", scene.spotLight.position);
	CMDUtils::Register("spot_dir", "Spot light direction (Vec3f)", scene.spotLight.direction);
	CMDUtils::Register("spot_cutOff", "Spot light inner cutoff (Float)", scene.spotLight.cutOff);
	CMDUtils::Register("spot_outerCutOff", "Spot light outer cutoff (Float)", scene.spotLight.outerCutOff);
	CMDUtils::Register("spot_constant", "Spot light constant (Float)", scene.spotLight.constant);
	CMDUtils::Register("spot_linear", "Spot light linear (Float)", scene.spotLight.linear);
	CMDUtils::Register("spot_quadratic", "Spot light quadratic (Float)", scene.spotLight.quadratic);
	CMDUtils::Register("spot_ambient", "Spot light ambient (Vec3f)", scene.spotLight.ambient);
	CMDUtils::Register("spot_diffuse", "Spot light diffuse (Vec3f)", scene.spotLight.diffuse);
	CMDUtils::Register("spot_specular", "Spot light specular (Vec3f)", scene.spotLight.specular);


	// --- Rendering ---
	CMDUtils::Register("r_gamma", "Adjusts screen gamma correction (Float)", g_config.r_gamma, 0.f, 4.f);
	CMDUtils::Register("r_shadowRes", "Shadow map resolution (Integer)", g_config.r_shadowRes, 512.f, 8192.f);
	CMDUtils::Register("r_renderDistance", "Maximum render distance (Float)", g_config.r_renderDistance, 100.f,
	                   5000.f);
	CMDUtils::Register("r_vsync", "Vertical synchronization (1 = on, 0 = off) (Boolean)", g_config.r_vsync);
	CMDUtils::Register(CVar::cvar_t(
		"r_fillColor", static_cast<std::array<float, 3>>(g_config.r_fillColor),
		[](const CVar::cvar_t& cvar) {
			g_config.r_fillColor = std::get<std::array<float, 3>>(cvar.val);
		}, "Render fill color"
	));
	// --- Console ---

	CMDUtils::Register("con_fontScale", "Console font size scale (Float)", g_config.con_fontScale, 8.f, 128.f);
	CMDUtils::Register("con_maxVisibleLines", "Maximum visible console lines (Integer)", g_config.con_maxVisibleLines,
	                   5.f, 100.f);
	CMDUtils::Register(CVar::cvar_t(
		"con_backgroundColor",
		static_cast<std::array<float, 4>>(g_config.con_backgroundColor),
		[](const CVar::cvar_t& cvar) {
			g_config.con_backgroundColor = std::get<std::array<float, 4>>(cvar.val);
		},
		"Console background color (RGBA)"
	));
	// --- Post-processing / Effects ---
	CMDUtils::Register("fx_chromaticOffset", "Strength of chromatic abberation", g_config.fx_chromaticOffset);

	CMDUtils::Register("fx_quantization", "Enable color quantization (1 = on, 0 = off) (Boolean)",
	                   g_config.fx_quantization);
	CMDUtils::Register("fx_quantizationLevel", "Color quantization level (Integer)", g_config.fx_quantizationLevel, 2.f,
	                   16.f);
	CMDUtils::Register("fx_vignette", "Enable vignette effect (1 = on, 0 = off) (Boolean)", g_config.fx_vignette);
	CMDUtils::Register("fx_vignetteIntensity", "Vignette intensity (Float)", g_config.fx_vignetteIntensity, 0.f, 1.f);
	CMDUtils::Register(CVar::cvar_t(
		"fx_vignetteColor",
		static_cast<std::array<float, 3>>(g_config.fx_vignetteColor),
		[](const CVar::cvar_t& cvar) {
			g_config.fx_vignetteColor = std::get<std::array<float, 3>>(cvar.val);
		},
		"Vignette color (RGB)"
	));
}

int main(int argc, char** argv) {
	g_config = {
		.sys_windowResolution = {1280, 720},

		.fx_chromaticOffset = 0.002f,

		.fx_quantization = false,
		.fx_quantizationLevel = 4,
		.fx_vignette = true,
		.fx_vignetteIntensity = 0.25f,
		.fx_vignetteColor = {0, 0, 0},

		.r_gamma = 2.2,
		.r_resolution = {2560, 1440}, // 2x sampling
		.r_shadowRes = 2048,
		.r_renderDistance = 1000.f,
		.r_vsync = false,
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
	MSDFText::Init();
	Backend::Init();
	RegisterCVars();

	// TODO make loading screen w/ progresbar
	setupScene();
	double lastTime = glfwGetTime();
	int fps = 0;
	double timer = 0;
	while (!glfwWindowShouldClose(Renderer::g_window)) {
		const double dt = glfwGetTime() - lastTime;

		timer += dt;
		if (timer > 0.5) {
			timer -= 0.5;
			fps = 1 / dt;
		}

		lastTime = glfwGetTime();
		if (g_isFlashlight) {
			scene.spotLight.position = scene.camera.position;
			scene.spotLight.position.y -= 2;
			scene.spotLight.direction = glm::normalize(scene.camera.target - scene.camera.position);
		}

		// UPDATE
		scene.camera.updatePosition();
		updateControls();
		Console::Update(dt);
		Input::PollEvents(); // always should be updated last

		// RENDER
		Renderer::GenShadowMaps(scene);
		Renderer::FrameBegin(scene);
		for (const auto& object : scene.objects) object->draw({});
		Console::Draw();
		MSDFText::DrawText(std::to_string(fps), ResourceMgr::GetFontByName("inconsolata_light"),
		                   g_config.r_resolution.x - 60,
		                   g_config.r_resolution.y - 32, 32,
		                   {Color::WHITE, 1});
		Renderer::FrameEnd();
	}
	Renderer::Shutdown();
	// Input::Shutdown();

	glfwTerminate();
	return 0;
}
