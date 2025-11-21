// it is what it is
#include <algorithm>
#include <array>
#include <string>
#include "Common/Config.hpp"
#include "Game/Scene.hpp"
#include "Renderer/Model.hpp"
#include "Renderer/Renderer.hpp"
#include "ResourceMgr/ResourceMgr.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./Input/Input.hpp"
#include "stb_image_write.h"
#include "UI/Text/MSDFText.hpp"
#include "UI/Console/Console.hpp"
#include "Common/Utils.hpp"
#include "Core/Backend.hpp"
#include "Core/CMDUtils.hpp"
#include "Core/CVar.hpp"
#include "Core/Logger.hpp"
#include "Game/CameraController.hpp"

// TODO make it json or smth
static Scene scene{
	.dirLight{
		.enable = true,
		.direction = {-0.5f, -0.2f, -1.f},
		.ambient = glm::vec3(0),
		.diffuse = glm::vec3(0.1, 0.1, 0.4),
		.specular = glm::vec3(0.1f)
	},
	.pointLight{
		.enable = true,
		.position = {0.f, 20.f, 0.f},

		.constant = 1.f,
		.linear = 0.05f,
		.quadratic = 0.01f,

		.ambient = {1.0f, 1.0f, 1.0f}, // Темно-оранжевый
		.diffuse = {1.0f, 1.5f, 1.0f}, // Оранжевый
		.specular = {1.f, 1.f, 1.f}
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

void SaveScreenshot(const char* filename) {
	// Выделение памяти под пиксели (формат
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
	if (Input::IsKeyPressed(Key::Escape) && Console::IsVisible()) Console::Toggle();
	if (Input::IsKeyPressed(Key::GraveAccent)) Console::Toggle();
	if (Input::IsKeyPressed(Key::F11)) SaveScreenshot("frame.png");
	if (Input::IsKeyPressed(Key::P) && !Console::IsVisible()) scene.pointLight.enable = !scene.pointLight.enable;

	if (Input::g_resizedHeight || Input::g_resizedWidth) {
		if (Input::g_resizedWidth == 0 && Input::g_resizedHeight == 0) return; // in case of minimizing
		g_config.sys_windowResolution = {Input::g_resizedWidth, Input::g_resizedHeight};
		// g_config.renderRes = {Input::g_resizedWidth, Input::g_resizedHeight};
		// Renderer::UpdateRenderRes();
		glViewport(0, 0, Input::g_resizedWidth, Input::g_resizedHeight);
	}
}

void setupScene() {
	const auto base_shader = ResourceMgr::GetShaderByName("LightingShader");
	if (!base_shader) {
		Log::Error("setupScene(): Unable to get base shader");
		return;
	}

	Renderer::MeshPtr cube = Utils::CreateCubeMesh("lightcube", 100);
	cube->transform = {.scale = glm::vec3(.25f)};
	cube->material->shader = ResourceMgr::GetShaderByName("LightCube");

	Renderer::MeshPtr plane = Utils::CreatePlaneMesh(100, "plane");
	plane->transform = {.scale = {500, 1, 500}};
	plane->material->useSolidColor = true;
	plane->material->solidColor = {0.3, 0.3, 0.3};
	plane->material->shader = base_shader;

	// loading models
	ResourceMgr::LoadModel("glock", "assets/models/glock/Glock-17gen5.fbx", base_shader);
	Renderer::ModelPtr glock = ResourceMgr::GetModelByName("glock");
	glock->transform = {
		.position = {0, 0, 0},
		.quaternion = glm::angleAxis(glm::radians(90.0f), glm::vec3(1,0,0)),
		.scale = glm::vec3(1),
	};

	if (plane) scene.objects.push_back(plane);
	if (cube) scene.objects.push_back(cube);
	if (glock) scene.objects.push_back(glock);
}

void LoadAll() {
	ResourceMgr::LoadMSDFFont("inconsolata_light", "assets/fonts/inconsolata/inconsolata_light.png",
	                          "assets/fonts/inconsolata/inconsolata_light.json");
	// ALL SHADERS
	ResourceMgr::LoadShader("screenfbo", "Shaders/PostEffects/PostFX.vert", "Shaders/2DTexture.frag");
	ResourceMgr::LoadShader("GaussianBlur", "Shaders/PostEffects/GaussianBlur.vert",
	                        "Shaders/PostEffects/GaussianBlur.frag");
	ResourceMgr::LoadShader("postfx", "Shaders/PostEffects/PostFX.vert", "Shaders/PostEffects/PostFX.frag");
	ResourceMgr::LoadShader("depth", "Shaders/Depth.vert", "Shaders/Depth.frag");
	ResourceMgr::LoadShader("LightCube", "Shaders/Light.vert", "Shaders/Light.frag");
	ResourceMgr::LoadShader("solidcolor", "Shaders/2DColor.vert", "Shaders/2DColor.frag");
	ResourceMgr::LoadShader("LightingShader", "Shaders/LightingShader.vert", "Shaders/LightingShader.frag");
	ResourceMgr::LoadShader("msdf_text", "Shaders/MSDFText.vert", "Shaders/MSDFText.frag");
	ResourceMgr::LoadShader("point_shadow_depth", "Shaders/point_shadow_depth.vert", "Shaders/point_shadow_depth.frag",
	                        "Shaders/point_shadow_depth.geom");
}

void RegisterCVars() {
	CMDUtils::Register("sensitivity", "Mouse responsivity (Float)", g_config.sensitivity, 0, 10);

	// --- Camera ---
	CMDUtils::Register("cam_position", "Camera position (Vec3f)", scene.camera.position);
	CMDUtils::Register("cam_yaw", "Camera yaw (Float)", scene.camera.yaw, -360.f, 360.f);
	CMDUtils::Register("cam_pitch", "Camera pitch (Float)", scene.camera.pitch, -90.f, 90.f);
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
	CMDUtils::Register("r_blurPasses", "Number of bloom blur passes", g_config.r_blurPasses, 0.f, 50.f);
	CMDUtils::Register("r_gamma", "Adjusts screen gamma correction (Float)", g_config.r_gamma, 0.f, 4.f);
	CMDUtils::Register("r_shadowRes", "Shadow map resolution (Integer)", g_config.r_shadowRes, 512.f, 8192.f);
	CMDUtils::Register("r_renderDistance", "Maximum render distance (Float)", g_config.r_renderDistance, 100.f,
	                   5000.f);
	CMDUtils::Register("r_vsync", "Vertical synchronization (Boolean)", g_config.r_vsync);
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
	CMDUtils::Register("fx_bloom", "Enable bloom effect (Boolean)", g_config.fx_bloom);
	CMDUtils::Register("fx_chromaticOffset", "Strength of chromatic abberation", g_config.fx_chromaticOffset);
	CMDUtils::Register("fx_exposure", "Exposure (Float)", g_config.fx_exposure, 0.1, 10);
	CMDUtils::Register("fx_autoExposureSpeed", "Speed of automatic exposure (Float)", g_config.fx_autoExposureSpeed,
	                   0.f, 1.f);
	CMDUtils::Register("fx_autoExposure",
	                   "Automatic adjustment of scene exposure to simulate eye adaptation from changes in brightness (Boolean)",
	                   g_config.fx_autoExposure);
	CMDUtils::Register("fx_quantization", "Enable color quantization (Boolean)",
	                   g_config.fx_quantization);
	CMDUtils::Register("fx_quantizationLevel", "Color quantization level (Integer)", g_config.fx_quantizationLevel, 2.f,
	                   16.f);
	CMDUtils::Register("fx_vignette", "Enable vignette effect (Boolean)", g_config.fx_vignette);
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
		.sensitivity = 0.1f,

		.sys_windowResolution = {1280, 720},

		.fx_bloom = true,
		.fx_chromaticOffset = 0.000f,
		.fx_quantization = false,
		.fx_quantizationLevel = 4,
		.fx_vignette = true,
		.fx_vignetteIntensity = 0.25f,

		.fx_exposure = 1.f,
		.fx_autoExposure = true,
		.fx_autoExposureSpeed = 0.02f,

		.fx_vignetteColor = {0, 0, 0},

		.r_blurPasses = 5,
		.r_gamma = 2.2,
		.r_resolution = {2560, 1440}, // 2x sampling
		.r_shadowRes = 2048,
		.r_renderDistance = 1000.f,
		.r_vsync = false,
		.r_fillColor = {0, 0, 0},

		.con_fontScale = 32,
		.con_maxVisibleLines = 20,
		.con_backgroundColor = {0, 0, 0, 0.9},
	};

	scene.camera = Camera::Camera(60, glm::vec3(0, 17, 0), 0.1, g_config.r_renderDistance);

	Log::Init();
	// Log::SetSeverity(Logger::Severity::Info);
	Camera::FPSCameraController cameraController(0.1f);


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
	while (!glfwWindowShouldClose(Renderer::g_window)) {
		const double dt = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		// UPDATE
		cameraController.update(scene.camera, dt);
		cameraController.handleControls(scene.camera);
		updateControls();
		Console::Update(dt);
		Input::PollEvents(); // always should be updated last

		// RENDER
		Renderer::GenShadowMaps(scene);
		Renderer::FrameBegin(scene);
		for (const auto& object : scene.objects) {
			if (object->name == "lightcube") object->transform.position = scene.pointLight.position;
			if (object->drawable) object->draw();
		}
		Renderer::ApplyPostProcess(dt);
		Console::Draw();
		Renderer::FrameEnd();
	}
	Renderer::Shutdown();
	// Input::Shutdown();

	glfwTerminate();
	return 0;
}
