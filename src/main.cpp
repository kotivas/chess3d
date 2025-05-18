#include <iostream>

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
#include "stb_image_write.h"

static Scene scene{
	.camera {
		.position = {0, 0, 0},
		.target = {5, 15, 0},

		.radius = 30.f,

		.yaw = -90.f,
		.pitch = 45.f,

		.lastx = 400,
		.lasty = 300,

		.sens = 0.2f,
		.fov = 45.f,

		.locked = true
	},
	.dirLight {
		.enable = false,
		.direction = {-0.5f, -1.0f, -0.5f},
		.ambient = glm::vec3(0.2f),
		.diffuse = glm::vec3(0.8f),
		.specular = glm::vec3(0.5f)
	},
	.pointLight{
		.enable = true,
		.position = {100.f, 100.f, 50.f},

		.constant = 1.f,
		.linear = 0.09f,
		.quadratic = 0.032f,

		.ambient = glm::vec3(0.1f),
		.diffuse = glm::vec3(0.8f),
		.specular = glm::vec3(1.f),
	},
	.spotLight{
		.enable = false,
		.cutOff = glm::radians(12.5f),
		.outerCutOff = glm::radians(17.5f),

		.constant = 1.f,
		.linear = 0.09f,
		.quadratic = 0.032f,

		.ambient = glm::vec3(0.f),
		.diffuse = glm::vec3(1.f),
		.specular = glm::vec3(1.f)
	}
};
static Render::PostEffects effects{
	.quantization = false,
	.quantizationLevel = 32,

	.vignette = true,
	.vignetteIntensity = 0.25f,
	.vignetteColor = glm::vec3(0)
};
static Config config{
	.windowRes = {1280, 720},
	.renderRes = {1920, 1080}, // 320, 240
	.shadowRes = 15360, // 16k

	// --- GRAPHICS ---
	.renderDistance = 1000.f,
	.vsync = true
};

static GLFWwindow* window = nullptr;
static Render::Renderer* renderer = nullptr;

void DrawIMGUI() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

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
		if (ImGui::InputInt("Shadow Resolution", &config.shadowRes)) {
			renderer->updateShadowRes();
		}
	}

	if (ImGui::CollapsingHeader("Camera Settings")) {

		ImGui::SliderFloat("Field Of View", &scene.camera.fov, 0, 100, "%.2f");
		ImGui::InputFloat3("Target", &scene.camera.target[0], "%.2f");
		ImGui::SliderFloat("Sensitivity", &scene.camera.sens, 0, 10, "%.1f");
	}

	if (ImGui::CollapsingHeader("Light Settings")) {
		if (ImGui::TreeNode("Directional Light")) {
			//ImGui::Checkbox("Enable", &scene.dirLight.enable);

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
			//ImGui::Checkbox("Enable", &scene.pointLight.enable);

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
			//ImGui::Checkbox("Enable", &scene.spotLight.enable);

			ImGui::BeginDisabled(!scene.spotLight.enable);

			//ImGui::ColorEdit3("Color", &scene.spotLight.color[0]);

			ImGui::InputFloat3("Ambient", &scene.spotLight.ambient[0]);
			ImGui::InputFloat3("Diffuse", &scene.spotLight.diffuse[0]);
			ImGui::InputFloat3("Specular", &scene.spotLight.specular[0]);

			ImGui::InputFloat("Constant", &scene.spotLight.constant);
			ImGui::InputFloat("Linear", &scene.spotLight.linear);
			ImGui::InputFloat("Quadratic", &scene.spotLight.quadratic);

			ImGui::SliderAngle("CutOff", &scene.spotLight.cutOff, 0, glm::degrees(scene.spotLight.outerCutOff) - 1);
			ImGui::SliderAngle("OuterCutOff", &scene.spotLight.outerCutOff, 0, 100);

			ImGui::EndDisabled();
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Objects Settings")) {
		for (const auto& model : scene.models) {
			if (ImGui::TreeNode(model->name.c_str())) {

				ImGui::InputFloat3("Scale", &model->transform.scale[0]);
				ImGui::InputFloat3("Position", &model->transform.position[0]);
				ImGui::InputFloat3("Rotation", &model->transform.rotation[0]);

				if (ImGui::TreeNode("Meshes")) {
					if (ImGui::Button("Disable all")) {
						for (const auto& mesh : model->meshes) { mesh->drawable = false; }
					}
					ImGui::SameLine();
					if (ImGui::Button("Enable all")) {
						for (const auto& mesh : model->meshes) { mesh->drawable = true; }
					}

					for (const auto& mesh : model->meshes) {
						if (ImGui::TreeNode(mesh->name.c_str())) {

							ImGui::Checkbox("Draw", &mesh->drawable);

							ImGui::BeginDisabled(!mesh->drawable);
							ImGui::Text(mesh->material->name.c_str());

							ImGui::InputFloat3("Position", &mesh->transform.position[0]);
							ImGui::SliderFloat3("Scale", &mesh->transform.scale[0], 1, 10, "%.1f");
							ImGui::InputFloat3("Rotation", &mesh->transform.rotation[0]);

							ImGui::EndDisabled();

							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}
	}

	ImGui::End(); // End general options

	// Rendering
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void SetStyleIMGUI() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Setting up a dark theme base
	colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.6f); // Semi-transparent dark background
	colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.4f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.8f);
	colors[ImGuiCol_Border] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);

	// Text and frames
	colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.5f); // Semi-transparent for frosted look
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.7f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);

	// Header
	colors[ImGuiCol_Header] = ImVec4(0.3f, 0.3f, 0.3f, 0.7f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.8f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);

	// Buttons
	colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 0.6f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.8f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	// Adjust window rounding and padding to enhance the glass look
	style.WindowRounding = 10.0f;
	style.FrameRounding = 5.0f;
	style.WindowPadding = ImVec2(10, 10);
	style.FramePadding = ImVec2(5, 5);
}
void InitIMGUI() {
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

	// Setup ImGui style
	ImGui::StyleColorsDark();
	SetStyleIMGUI();

	io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSans-Medium.ttf", 18.f);

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
	std::cout << "[info] Screenshot saved in " << filename << std::endl;
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
	}
}
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	scene.camera.mouseScrolled(yoffset);
}
void ResizeCallback(GLFWwindow* window, int width, int height) {
	if (width == 0 && height == 0) return; // in case of minimizing
	config.windowRes = { width, height };
	glViewport(0, 0, width, height);
}
void InitGLFW() {
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
}

Render::MeshPtr CreateSphereMesh(float radius, uint32_t stackCount, uint32_t sliceCount) {
	Render::MeshPtr mesh = std::make_shared<Render::Mesh>();
	mesh->name = "sphere";

	// Генерация вершин
	mesh->vertices.push_back({
		glm::vec3(0.0f, radius, 0.0f),    // position
		glm::vec3(0.0f, 1.0f, 0.0f),      // normal
		glm::vec2(0.5f, 0.0f)             // tex_coords
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
		glm::vec3(0.0f, -radius, 0.0f),   // position
		glm::vec3(0.0f, -1.0f, 0.0f),     // normal
		glm::vec2(0.5f, 1.0f)             // tex_coords
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
Render::ModelPtr CreateLightSphere() {
	Render::ModelPtr model = std::make_unique<Render::Model>();
	Render::MeshPtr mesh = CreateSphereMesh(8, 32, 32);
	Render::MaterialPtr mat = std::make_unique<Render::Material>();

	mat->name = "light";
	mat->diffuse[0] = ResourceManager::CreateDefaultTexture();
	mat->specular[0] = ResourceManager::CreateDefaultTexture();
	mat->shininess = 8;

	model->name = "light";
	model->meshes[0]->material = mat;
	model->meshes.push_back(mesh);

	return model;
}
Render::ModelPtr CreatePlane(const std::string& diffuse, const std::string& specular, float shininess, const std::string& name) {
	Render::MeshPtr mesh = std::make_unique<Render::Mesh>();;
	Render::ModelPtr model = std::make_unique<Render::Model>();;
	Render::MaterialPtr mat = std::make_unique<Render::Material>();

	// setup material

	mat->name = name;
	mat->diffuse[0] = ResourceManager::CreateTexture(diffuse);
	mat->specular[0] = ResourceManager::CreateTexture(specular);
	mat->shininess = shininess;

	// setup mesh

	mesh->name = name;

	mesh->vertices = {
	{ {-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
	{ { 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
	{ { 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
	{ {-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} } };

	mesh->indices = { 0, 2, 1, 0, 3, 2 };
	mesh->material = mat;

	mesh->setup();

	// setup model

	model->name = name;
	model->meshes.push_back(mesh);

	return model;
}

void setupScene(bool props) {
	// setup chess set

	Render::ShaderPtr shader = std::make_shared<Render::Shader>("assets/shaders/scene.vert", "assets/shaders/scene.frag");

	if (props) {

		//Render::ModelPtr chess = ResourceManager::LoadModel("assets/models/ChessSet.obj");
		//chess->setScale(20);
		//chess->setPosition(0, 0.81, -0.3);
		//chess->setRotation(0, -25, 0);
		//scene.models.push_back(chess);

		//// setup lamp

		//Render::ModelPtr lamp = ResourceManager::LoadModel("assets/models/lampa.obj");
		//lamp->setScale(0.2);
		//lamp->setPosition(-22, 101, -40);
		//lamp->setRotation(0, 45, 0);
		//scene.models.push_back(lamp);

		//// setup AK

		//Render::ModelPtr ak = ResourceManager::LoadModel("assets/models/ak.obj");

		//ak->findMeshByName("stock")->draw = false;
		//ak->findMeshByName("magazine")->rotation = { 30, 0, 0 };
		//ak->findMeshByName("magazine")->position = { 0, -0.3, -0.3 };

		//ak->setPosition(3.29, 0, 1.5);
		//ak->setRotation(0, -55, 90);
		//ak->setScale(5);
		//scene.models.push_back(ak);

		// setup desk  
		
		Render::ModelPtr desk = ResourceManager::LoadModel("assets/models/desk.obj", shader); 
		desk->transform.scale = { 20, 20, 20 };
		scene.models.push_back(desk); 

		Render::ModelPtr revolver = ResourceManager::LoadModel("assets/models/ColtPyton.obj", shader);
		revolver->transform.scale = { 2, 2, 2 };
		revolver->transform.position = { 0, 9, 0 };
		scene.models.push_back(revolver);

	}

	// create floor

	Render::ModelPtr floor = CreatePlane("assets/textures/woodFloor/wood_floor_diff.png", "assets/textures/woodFloor/wood_floor_rough.png", 8, "Floor");
	floor->transform.scale = { 100, 100, 100 };
	floor->meshes[0]->material->shader = shader;
	scene.models.push_back(floor);

}

int main() {
	InitGLFW();
	InitIMGUI();

	setupScene(true);

	renderer = new Render::Renderer(config);

	while (!glfwWindowShouldClose(window)) {

		renderer->generateShadowMap(scene);
		renderer->drawScene(scene);
		renderer->renderFrame(effects);

		DrawIMGUI();

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