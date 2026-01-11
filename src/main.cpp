// it is what it is
#include <chrono>
#include <filesystem>

#include "./Input/Input.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Common/Config.hpp"
#include "Common/GlUtils.hpp"
#include "Common/Utils.hpp"
#include "Core/Backend.hpp"
#include "Core/CMDUtils.hpp"
#include "Core/Logger.hpp"
#include "Game/CameraController.hpp"
#include "Game/Scene.hpp"
#include "Renderer/Model.hpp"
#include "Renderer/Renderer.hpp"
#include "ResourceMgr/ResourceMgr.hpp"
#include "UI/Console/Console.hpp"
#include "UI/Text/MSDFText.hpp"

static bool gIsPaused = false;

void setupScene(Scene &scene) {
    const auto baseShader = AssetManager::g_shaderManager.getHandle("LightingShader");
    if (!baseShader) {
        Log::Fatal("setupScene(): Unable to get lightning shader");
        return;
    }

    ResourceMgr::LoadTexture(
        "snow_diffuse", Renderer::TextureType::Tex2D, Renderer::TextureWrapMode::Repeat,
        "assets/textures/snow/snow_color.png");
    ResourceMgr::LoadTexture(
        "snow_normal", Renderer::TextureType::Tex2D, Renderer::TextureWrapMode::Repeat,
        "assets/textures/snow/snow_normal.png");
    ResourceMgr::LoadTexture(
        "snow_displacement", Renderer::TextureType::Tex2D, Renderer::TextureWrapMode::Repeat,
        "assets/textures/snow/snow_displacement.png");
    if (const Renderer::MeshPtr plane = Utils::CreatePlaneMesh("plane", 500.f, 64)) {
        scene.objects.push_back(plane);
        plane->castShadow = true;

        plane->material->useDiffuse = true;
        plane->material->diffuse = ResourceMgr::GetTextureByName("snow_diffuse");
        plane->material->useNormal = true;
        plane->material->normal = ResourceMgr::GetTextureByName("snow_normal");
        plane->material->useDisplacement = true;
        plane->material->displacement = ResourceMgr::GetTextureByName("snow_displacement");

        plane->material->shader = baseShader;
        plane->material->shininess = 50;

        std::cout << plane->material->diffuse << std::endl;
    }


    const auto sky = std::make_shared<Renderer::Sky>();
    sky->setup();

    sky->shader = AssetManager::g_shaderManager.getHandle("sky");
    sky->atmTurbidity = 3.f;
    scene.sky = sky;

    ResourceMgr::LoadModel("makarov", "assets/models/makarov/pm.fbx", baseShader);
    if (const Renderer::ModelPtr makarov = ResourceMgr::GetModelByName("makarov")) {
        makarov->meshes[1]->material->normal = ResourceMgr::CreateTexture2D(
            "assets/models/makarov/T_pt_ptm_skin_monolith_N.png", Renderer::TextureWrapMode::ClampToBorder);
        makarov->meshes[1]->material->useNormal = true;
        makarov->transform = {
            .position = {0, 5, 0},
            .quaternion = glm::quat(glm::radians(glm::vec3(-45, 0, 12))),
            .scale = glm::vec3(0.005),
        };
        scene.objects.push_back(makarov);
    }

    ResourceMgr::LoadMSDFFont(
        "inconsolata_light", "assets/fonts/inconsolata/inconsolata_light.png",
        "assets/fonts/inconsolata/inconsolata_light.json");
}

void reloadShaders() {
    AssetManager::g_shaderManager.reload("sky");
    AssetManager::g_shaderManager.reload("screenfbo");
    AssetManager::g_shaderManager.reload("GaussianBlur");
    AssetManager::g_shaderManager.reload("postfx");
    AssetManager::g_shaderManager.reload("depth");
    AssetManager::g_shaderManager.reload("LightCube");
    AssetManager::g_shaderManager.reload("solidcolor");
    AssetManager::g_shaderManager.reload("LightingShader");
    AssetManager::g_shaderManager.reload("msdf");
    AssetManager::g_shaderManager.reload("point_shadow_depth");
}

void loadShaders() {
    AssetManager::g_shaderManager.load("sky", "Shaders/Sky.vert", "Shaders/Sky.frag");
    AssetManager::g_shaderManager.load("screenfbo", "Shaders/PostEffects/PostFX.vert", "Shaders/2DTexture.frag");
    AssetManager::g_shaderManager.load(
        "GaussianBlur", "Shaders/PostEffects/GaussianBlur.vert", "Shaders/PostEffects/GaussianBlur.frag");
    AssetManager::g_shaderManager.load("postfx", "Shaders/PostEffects/PostFX.vert", "Shaders/PostEffects/PostFX.frag");
    AssetManager::g_shaderManager.load("depth", "Shaders/Depth.vert", "Shaders/Depth.frag");
    AssetManager::g_shaderManager.load("solidcolor", "Shaders/2DColor.vert", "Shaders/2DColor.frag");
    AssetManager::g_shaderManager.load("2DTexture", "Shaders/2DTexture.vert", "Shaders/2DTexture.frag");
    AssetManager::g_shaderManager.load("LightingShader", "Shaders/LightingShader.vert", "Shaders/LightingShader.frag");
    AssetManager::g_shaderManager.load("msdf", "Shaders/MSDFText.vert", "Shaders/MSDFText.frag");
    AssetManager::g_shaderManager.load(
        "point_shadow_depth", "Shaders/point_shadow_depth.vert", "Shaders/point_shadow_depth.frag",
        "Shaders/point_shadow_depth.geom");
}

void registerCVars(Scene &scene) {
    CMDUtils::Register("time", "Day time in seconds", scene.time, 0, 86400);

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

    // ========================= cl_ (Client / Input) =========================
    CMDUtils::Register("cl_sens", "Mouse sensitivity multiplier.", g_config.sensitivity, 0.01f, 10.0f);
    // ========================= vid_ (Window / Video) =========================
    CMDUtils::Register("vid_resolution", "Window resolution (width height).", g_config.windowResolution);
    CMDUtils::Register("vid_vsync", "Enable vertical synchronization.", Renderer::settings.vsync);
    // ========================= con_ (Console / UI) =========================
    CMDUtils::Register("con_fontscale", "Console font scale in pixels.", g_config.consoleFontScale, 8, 200);
    CMDUtils::Register("con_lines", "Number of visible console lines.", g_config.consoleLines, 1, 200);
    CMDUtils::Register("con_color", "Console background color (RGBA).", g_config.consoleColor);
    // ========================= sv_ (World / Simulation) =========================
    CMDUtils::Register("sv_time", "World time in seconds (0..86400).", scene.time, 0.0f, 86400.0f);
    // ========================= r_atm_ (Atmosphere / Sky) =========================
    CMDUtils::Register(
        "r_atm_turbidity", "Atmospheric turbidity (higher = hazier sky).", scene.sky->atmTurbidity, 1.0f, 50.0f);
    // ========================= r_ (Post-processing / FX) =========================
    CMDUtils::Register("r_bloom", "Enable bloom post-process.", Renderer::settings.FX.bloom);
    CMDUtils::Register(
        "r_chromatic_aberration", "Chromatic aberration offset.", Renderer::settings.FX.chromaticOffset, 0.0f, 0.2f);
    CMDUtils::Register("r_color_quantization", "Enable color quantization.", Renderer::settings.FX.quantization);
    CMDUtils::Register(
        "r_color_levels", "Color quantization levels.", Renderer::settings.FX.quantizationLevel, 2.0f, 256.0f);
    CMDUtils::Register("r_vignette", "Enable vignette effect.", Renderer::settings.FX.vignette);
    CMDUtils::Register(
        "r_vignette_intensity", "Vignette intensity.", Renderer::settings.FX.vignetteIntensity, 0.0f, 2.0f);
    CMDUtils::Register("r_vignette_color", "Vignette color (RGB).", Renderer::settings.FX.vignetteColor);
    CMDUtils::Register("r_saturation", "Color saturation multiplier.", Renderer::settings.FX.saturation, 0.0f, 3.0f);
    CMDUtils::Register("r_exposure", "Manual exposure multiplier.", Renderer::settings.FX.exposure, 0.01f, 10.0f);
    CMDUtils::Register("r_gamma", "Gamma correction value.", Renderer::settings.FX.gamma, 0.5f, 4.0f);
    // ========================= mat_ (Materials / Surface) =========================
    CMDUtils::Register(
        "mat_parallax_scale", "Parallax mapping height scale.", Renderer::settings.parallaxScale, 0.0f, 1.0f);
    // ========================= r_shadow_ (Shadows) =========================
    CMDUtils::Register(
        "r_shadow_distance", "Maximum distance where shadows are rendered.", Renderer::settings.shadowDistance, 0.0f,
        5000.0f);
    CMDUtils::Register(
        "r_shadow_dir_res", "Directional light shadow map resolution.", Renderer::settings.dirShadowRes, 256, 16384);
    CMDUtils::Register(
        "r_shadow_point_res", "Point light shadow cubemap resolution.", Renderer::settings.pointShadowRes, 128, 4096);
    CMDUtils::Register(
        "r_shadow_spot_res", "Spot light shadow map resolution.", Renderer::settings.spotShadowRes, 128, 8192);
    // ========================= r_ (Rendering / Culling) =========================
    CMDUtils::Register(
        "r_render_distance", "World render distance (culling / dir light projection).",
        Renderer::settings.renderDistance, 16.0f, 10000.0f);
    CMDUtils::Register("r_render_resolution", "Internal rendering resolution.", Renderer::settings.renderResolution);
    CMDUtils::Register(
        "r_blur_passes", "Number of blur passes for post-processing.", Renderer::settings.blurPasses, 0, 8);
}

void updateControls(Scene &scene) {
    if (Input::IsKeyPressed(Key::Escape) && Console::IsVisible()) Console::Toggle();
    if (Input::IsKeyPressed(Key::GraveAccent)) Console::Toggle();
    if (Input::IsKeyPressed(Key::F11)) GlUtils::SaveFrame("screenshot");
    if (Input::IsKeyPressed(Key::L) && !Console::g_isVisible) scene.spot_light.enable = !scene.spot_light.enable;
    if (Input::IsKeyPressed(Key::R) && !Console::g_isVisible) reloadShaders();
    if (Input::IsKeyPressed(Key::Pause)) gIsPaused = !gIsPaused;

    if (Input::g_resizedHeight || Input::g_resizedWidth) {
        if (Input::g_resizedWidth == 0 && Input::g_resizedHeight == 0) return; // in case of minimizing
        g_config.windowResolution = {Input::g_resizedWidth, Input::g_resizedHeight};
        // g_config.r_resolution = {Input::g_resizedWidth, Input::g_resizedHeight};
        // Renderer::UpdateRenderRes();
    }
}

int main(int argc, char **argv) {
    g_config = {
        .sensitivity = 0.1f,
        .windowResolution = {1280, 720},
        .autoExposure = true,
        .autoExposureSpeed = 0.02f,
        .consoleFontScale = 25,
        .consoleLines = 15,
        .consoleColor = {0, 0, 0, 0.9},
    };

    Renderer::settings = {
        .FX =
            {
                .bloom = false,
                .chromaticOffset = 0.f,
                .quantization = false,
                .quantizationLevel = 4.f,
                .vignette = false,
                .vignetteIntensity = 0.25f,
                .vignetteColor = {0, 0, 0},
                .saturation = 1.f,
                .exposure = 1.f,
                .gamma = 1,
            },

        .parallaxScale = 0.05,
        .shadowDistance = 100.f,
        .dirShadowRes = 8192,
        .pointShadowRes = 1024,
        .spotShadowRes = 2048,
        .renderDistance = 1000.f,
        .renderResolution = {1920, 1080},
        .blurPasses = 4,
        .vsync = false,
    };

    Scene scene{
        .camera = Camera::CameraInfo(75, glm::vec3(0, 17, 0), 0.1, Renderer::settings.renderDistance),
        .time = 3000,
        .dir_light =
            {
                .enable = 1,
                .direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)),
                .ambient = glm::vec3(0.2f),  // subtle ambient
                .diffuse = glm::vec3(0.95f), // strong diffuse
                .specular = glm::vec3(0.2f)  // modest specular
            },
        .point_light =
            {.enable = 0,
             .position = glm::vec3(0.0f, 3.0f, 0.0f),
             .constant = 1.0f,
             .linear = 0.09f,
             .quadratic = 0.032f,
             .ambient = glm::vec3(0.02f), // small ambient
             .diffuse = glm::vec3(1.0f),  // bright bulb
             .specular = glm::vec3(0.8f)},
        .spot_light =
            {.enable = 0,
             .position = glm::vec3(0.0f, 4.0f, 0.0f),
             .direction = glm::normalize(glm::vec3(0.0f, -1.0f, -0.2f)), // pointing down-ish
             .cutOff = glm::cos(glm::radians(12.5f)),                    // inner cone
             .outerCutOff = glm::cos(glm::radians(17.5f)),               // outer (soft edge)
             .constant = 1.0f,
             .linear = 0.09f,
             .quadratic = 0.008f,
             .ambient = glm::vec3(0.0f),
             .diffuse = glm::vec3(1.0f),
             .specular = glm::vec3(1.0f)},
    };

    Log::Init();
    // Log::SetSeverity(Logger::Severity::Info);
    Camera::FPSCameraController cameraController(0.1f);

    Renderer::GlInit();
    loadShaders();
    Renderer::Init();
    Input::Init();
    Console::Init();
    MsdfText::Init();
    Backend::Init();
    setupScene(scene);
    registerCVars(scene);

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(Renderer::g_window)) {
        const double dt = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();

        // UPDATE
        if (!gIsPaused) {
            scene.time += dt * 60;
            scene.time = fmod(scene.time, 86400.f);

            cameraController.update(scene.camera, dt);
            cameraController.handleControls(scene.camera);
            Console::Update(dt);

            const float dayFraction = glm::fract(scene.time / 86400); // 0..1
            scene.sky->update({dayFraction * glm::two_pi<float>(), 0.f});

            scene.dir_light.diffuse = scene.sky->sunColor;
            scene.dir_light.specular = scene.sky->sunColor;
            scene.dir_light.ambient = scene.sky->skyColor;
            scene.dir_light.direction = -scene.sky->sunDirection;

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
