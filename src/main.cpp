// it is what it is
#include <chrono>
#include <filesystem>

#include "Common/Config.hpp"
#include "Common/Utils.hpp"
#include "Common/GlUtils.hpp"
#include "Game/Scene.hpp"
#include "Game/CameraController.hpp"
#include "Renderer/Model.hpp"
#include "Renderer/Renderer.hpp"
#include "ResourceMgr/ResourceMgr.hpp"
#include "UI/Text/MSDFText.hpp"
#include "UI/Console/Console.hpp"
#include "Core/Backend.hpp"
#include "Core/CMDUtils.hpp"
#include "Core/CVar.hpp"
#include "Core/Logger.hpp"
#include "./Input/Input.hpp"
#include "AssetManager/AssetManager.hpp"

static bool g_isPaused = false;

void setupScene(Scene& scene) {
	const auto base_shader = AssetManager::g_ShaderManager.getHandle("LightingShader");
	if (!base_shader) {
		Log::Fatal("setupScene(): Unable to get base shader");
		return;
	}

	// Renderer::MeshPtr cube = Utils::CreateCubeMesh("lightcube", 100);
	// cube->transform = {.scale = glm::vec3(.25f)};
	// cube->material->shader = ResourceMgr::GetShaderByName("LightCube");

	if (const Renderer::MeshPtr plane = Utils::CreatePlaneMesh("plane")) {
		scene.objects.push_back(plane);
		plane->transform = {.scale = {1000, 1, 1000}};
		plane->castShadow = false;
		plane->material->useDiffuse = false;
		plane->material->solidColor = {0.25, 0.25, 0.25};
		plane->material->shader = base_shader;
		plane->material->shininess = 100;
	}

	const auto sky = std::make_shared<Renderer::Sky>();
	sky->setup();

	sky->shader = AssetManager::g_ShaderManager.getHandle("sky");
	sky->atm_turbidity = 4.f;
	scene.sky = sky;

	const Renderer::MeshPtr cube = Utils::CreateCubeMesh("cube");
	cube->transform = {
		.position = {0, 5, 0},
		.scale = glm::vec3(.50f),
	};
	cube->material->solidColor = {0.039f, 0.196f, 0.667f};
	cube->material->shininess = 100;
	cube->material->shader = base_shader;
	scene.objects.push_back(cube);


	ResourceMgr::LoadModel("glock", "assets/models/glock/Glock-17gen5.fbx", base_shader);
	if (const Renderer::ModelPtr glock = ResourceMgr::GetModelByName("glock")) {
		glock->transform = {
			.position = {-35, 10, 0},
			.quaternion = glm::quat(glm::radians(glm::vec3(0, 45, 0))),
			.scale = glm::vec3(0.1),
		};
		scene.objects.push_back(glock);
	}

	// ResourceMgr::LoadModel("zdanie", "assets/models/zdanie/zdanie.fbx", base_shader);
	if (const Renderer::ModelPtr zdanie = ResourceMgr::GetModelByName("zdanie")) {
		zdanie->transform = {.scale = glm::vec3(0.1)};
		scene.objects.push_back(zdanie);
	}
}

void LoadResources() {
	// ResourceMgr::LoadTexture("noise3d", Renderer::TextureType::Tex3D, Renderer::TextureWrapMode::Repeat,
	//                          "assets/textures/noise3d.raw");
	ResourceMgr::LoadMSDFFont("inconsolata_light", "assets/fonts/inconsolata/inconsolata_light.png",
	                          "assets/fonts/inconsolata/inconsolata_light.json");
}

void ReloadShaders() {
	AssetManager::g_ShaderManager.reload("sky");
	AssetManager::g_ShaderManager.reload("screenfbo");
	AssetManager::g_ShaderManager.reload("GaussianBlur");
	AssetManager::g_ShaderManager.reload("postfx");
	AssetManager::g_ShaderManager.reload("depth");
	AssetManager::g_ShaderManager.reload("LightCube");
	AssetManager::g_ShaderManager.reload("solidcolor");
	AssetManager::g_ShaderManager.reload("LightingShader");
	AssetManager::g_ShaderManager.reload("msdf");
	AssetManager::g_ShaderManager.reload("point_shadow_depth");
}

void LoadAllShaders() {
	AssetManager::g_ShaderManager.load("sky", "Shaders/Sky.vert", "Shaders/Sky.frag");
	AssetManager::g_ShaderManager.load("screenfbo", "Shaders/PostEffects/PostFX.vert", "Shaders/2DTexture.frag");
	AssetManager::g_ShaderManager.load("GaussianBlur", "Shaders/PostEffects/GaussianBlur.vert",
	                                   "Shaders/PostEffects/GaussianBlur.frag");
	AssetManager::g_ShaderManager.load("postfx", "Shaders/PostEffects/PostFX.vert", "Shaders/PostEffects/PostFX.frag");
	AssetManager::g_ShaderManager.load("depth", "Shaders/Depth.vert", "Shaders/Depth.frag");
	AssetManager::g_ShaderManager.load("LightCube", "Shaders/LightCube.vert", "Shaders/LightCube.frag");
	AssetManager::g_ShaderManager.load("solidcolor", "Shaders/2DColor.vert", "Shaders/2DColor.frag");
	AssetManager::g_ShaderManager.load("2DTexture", "Shaders/2DTexture.vert", "Shaders/2DTexture.frag");
	AssetManager::g_ShaderManager.load("LightingShader", "Shaders/LightingShader.vert", "Shaders/LightingShader.frag");
	AssetManager::g_ShaderManager.load("msdf", "Shaders/MSDFText.vert", "Shaders/MSDFText.frag");
	AssetManager::g_ShaderManager.load("point_shadow_depth", "Shaders/point_shadow_depth.vert",
	                                   "Shaders/point_shadow_depth.frag",
	                                   "Shaders/point_shadow_depth.geom");
}

void RegisterCVars(Scene& scene) {
	CMDUtils::Register("sensitivity", "Mouse responsivity (Float)", g_config.sensitivity, 0, 10);

	CMDUtils::Register("time", "Day time in seconds", scene.time);

	// --- Camera ---
	CMDUtils::Register("cam_position", "Camera position (Vec3f)", scene.camera.position);
	CMDUtils::Register("cam_yaw", "Camera yaw (Float)", scene.camera.yaw, -360.f, 360.f);
	CMDUtils::Register("cam_pitch", "Camera pitch (Float)", scene.camera.pitch, -90.f, 90.f);
	CMDUtils::Register("cam_fov", "Camera FOV (Float)", scene.camera.fov, 1.f, 180.f);

	CMDUtils::Register("dir_enable", "Directional light enabled (Boolean)", scene.dir_light.enable);
	CMDUtils::Register("dir_dir", "Directional light direction (Vec3f)", scene.dir_light.direction);
	CMDUtils::Register("dir_ambient", "Directional light ambient (Vec3f)", scene.dir_light.ambient);
	CMDUtils::Register("dir_diffuse", "Directional light diffuse (Vec3f)", scene.dir_light.diffuse);
	CMDUtils::Register("dir_specular", "Directional light specular (Vec3f)", scene.dir_light.specular);

	CMDUtils::Register("pt_enable", "Point light enabled (Boolean)", scene.point_light.enable);
	CMDUtils::Register("pt_pos", "Point light position (Vec3f)", scene.point_light.position);
	CMDUtils::Register("pt_constant", "Point light constant (Float)", scene.point_light.constant);
	CMDUtils::Register("pt_linear", "Point light linear (Float)", scene.point_light.linear);
	CMDUtils::Register("pt_quadratic", "Point light quadratic (Float)", scene.point_light.quadratic);
	CMDUtils::Register("pt_ambient", "Point light ambient (Vec3f)", scene.point_light.ambient);
	CMDUtils::Register("pt_diffuse", "Point light diffuse (Vec3f)", scene.point_light.diffuse);
	CMDUtils::Register("pt_specular", "Point light specular (Vec3f)", scene.point_light.specular);

	CMDUtils::Register("spot_enable", "Spot light enabled (Boolean)", scene.spot_light.enable);
	CMDUtils::Register("spot_pos", "Spot light position (Vec3f)", scene.spot_light.position);
	CMDUtils::Register("spot_dir", "Spot light direction (Vec3f)", scene.spot_light.direction);
	CMDUtils::Register("spot_cutOff", "Spot light inner cutoff (Float)", scene.spot_light.cutOff);
	CMDUtils::Register("spot_outerCutOff", "Spot light outer cutoff (Float)", scene.spot_light.outerCutOff);
	CMDUtils::Register("spot_constant", "Spot light constant (Float)", scene.spot_light.constant);
	CMDUtils::Register("spot_linear", "Spot light linear (Float)", scene.spot_light.linear);
	CMDUtils::Register("spot_quadratic", "Spot light quadratic (Float)", scene.spot_light.quadratic);
	CMDUtils::Register("spot_ambient", "Spot light ambient (Vec3f)", scene.spot_light.ambient);
	CMDUtils::Register("spot_diffuse", "Spot light diffuse (Vec3f)", scene.spot_light.diffuse);
	CMDUtils::Register("spot_specular", "Spot light specular (Vec3f)", scene.spot_light.specular);

	// --- Rendering ---
	CMDUtils::Register("r_blurPasses", "Number of bloom blur passes", g_config.r_blurPasses, 0.f, 50.f);
	CMDUtils::Register("r_gamma", "Adjusts screen gamma correction (Float)", g_config.r_gamma, 0.f, 4.f);
	CMDUtils::Register("r_shadowRes", "Shadow map resolution (Integer)", g_config.r_shadowRes, 512.f, 8192.f);
	CMDUtils::Register("r_renderDistance", "Maximum render distance (Float)", g_config.r_renderDistance, 1.f, 5000.f);
	CMDUtils::Register("r_vsync", "Vertical synchronization (Boolean)", g_config.r_vsync);
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
	CMDUtils::Register("fx_saturation", "Intensity of the color (Float)", g_config.fx_saturation, 0.0, 100);
	CMDUtils::Register("fx_chromaticOffset", "Strength of chromatic abberation (Float)", g_config.fx_chromaticOffset);
	CMDUtils::Register("fx_exposure", "Exposure (Float)", g_config.fx_exposure, 0.1, 10);
	CMDUtils::Register("fx_autoExposureSpeed", "Speed of automatic exposure (Float)", g_config.fx_autoExposureSpeed,
	                   0.f, 1.f);
	CMDUtils::Register("fx_autoExposure",
	                   "Automatic adjustment of scene exposure to simulate eye adaptation from changes in brightness (Boolean)",
	                   g_config.fx_autoExposure);
	CMDUtils::Register("fx_quantization", "Enable color quantization (Boolean)", g_config.fx_quantization);
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

void updateControls(Scene& scene) {
	if (Input::IsKeyPressed(Key::Escape) && Console::IsVisible()) Console::Toggle();
	if (Input::IsKeyPressed(Key::GraveAccent)) Console::Toggle();
	if (Input::IsKeyPressed(Key::F11)) GlUtils::SaveFrame("screenshot");
	if (Input::IsKeyPressed(Key::L) && !Console::g_isVisible) scene.spot_light.enable = !scene.spot_light.enable;
	if (Input::IsKeyPressed(Key::R)) ReloadShaders();
	if (Input::IsKeyPressed(Key::Pause)) g_isPaused = !g_isPaused;

	if (Input::g_resizedHeight || Input::g_resizedWidth) {
		if (Input::g_resizedWidth == 0 && Input::g_resizedHeight == 0) return; // in case of minimizing
		g_config.sys_windowResolution = {Input::g_resizedWidth, Input::g_resizedHeight};
		// g_config.r_resolution = {Input::g_resizedWidth, Input::g_resizedHeight};
		// Renderer::UpdateRenderRes();
	}
}

int main(int argc, char** argv) {
	g_config = {
		.sensitivity = 0.1f,

		.sys_windowResolution = {1280, 720},

		.fx_bloom = false,
		.fx_chromaticOffset = 0.000f,
		.fx_quantization = false,
		.fx_quantizationLevel = 4,
		.fx_vignette = true,
		.fx_vignetteIntensity = 0.25f,
		.fx_saturation = 1.f,
		.fx_exposure = 1.f,
		.fx_autoExposure = true,
		.fx_autoExposureSpeed = 0.02f,
		.fx_vignetteColor = {0, 0, 0},

		.r_blurPasses = 3,
		.r_gamma = 1,
		.r_resolution = {1920, 1080},
		.r_shadowRes = 1024,
		.r_renderDistance = 1000.f,
		.r_vsync = false,

		.con_fontScale = 25,
		.con_maxVisibleLines = 15,
		.con_backgroundColor = {0, 0, 0, 0.9},
	};
	Scene scene{
		.camera = Camera::Camera(75, glm::vec3(0, 17, 0), 0.1, g_config.r_renderDistance),
		.dir_light = {
			.enable = 1,
			.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)),
			.ambient = glm::vec3(0.1f), // subtle ambient
			.diffuse = glm::vec3(0.95f), // strong diffuse
			.specular = glm::vec3(0.2f) // modest specular
		},
		.point_light = {
			.enable = 0,
			.position = glm::vec3(0.0f, 3.0f, 0.0f),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
			.ambient = glm::vec3(0.02f), // small ambient
			.diffuse = glm::vec3(1.0f), // bright bulb
			.specular = glm::vec3(0.8f)
		},
		.spot_light = {
			.enable = 0,
			.position = glm::vec3(0.0f, 4.0f, 0.0f),
			.direction = glm::normalize(glm::vec3(0.0f, -1.0f, -0.2f)), // pointing down-ish
			.cutOff = glm::cos(glm::radians(12.5f)), // inner cone
			.outerCutOff = glm::cos(glm::radians(17.5f)), // outer (soft edge)
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.008f,
			.ambient = glm::vec3(0.0f),
			.diffuse = glm::vec3(1.0f),
			.specular = glm::vec3(1.0f)
		}

	};

	Log::Init();
	// Log::SetSeverity(Logger::Severity::Info);
	Camera::FPSCameraController camera_controller(0.1f);

	Renderer::GlInit();
	LoadAllShaders();
	Renderer::Init();
	LoadResources();
	Input::Init();
	Console::Init();
	MSDFText::Init();
	Backend::Init();
	setupScene(scene);
	RegisterCVars(scene);

	scene.time = 30000;
	double last_time = glfwGetTime();
	while (!glfwWindowShouldClose(Renderer::g_window)) {
		const double dt = glfwGetTime() - last_time;
		last_time = glfwGetTime();

		// UPDATE
		if (!g_isPaused) {
			scene.time += dt * 100;
			scene.time = fmod(scene.time, 86400);

			camera_controller.update(scene.camera, dt);
			camera_controller.handleControls(scene.camera);
			Console::Update(dt);
			float dayFraction = glm::fract(scene.time / 86400); // 0..1
			scene.sky->calculateSun({dayFraction * glm::two_pi<float>(), 0.f});

			scene.dir_light.diffuse = scene.sky->sun_color;
			scene.dir_light.specular = scene.sky->sun_color;
			scene.dir_light.direction = -scene.sky->sun_direction;

			if (scene.spot_light.enable) {
				scene.spot_light.direction = scene.camera.forward;
				scene.spot_light.position = scene.camera.position;
			}
		}

		// RENDER
		Renderer::Render(scene, dt);
		Input::PollEvents(); // always should be updated last
		updateControls(scene);
	}
	Renderer::Shutdown();
	glfwTerminate();
	return 0;
}
