#include "input.hpp"
#include <GLFW/glfw3.h>


namespace Input {
	double g_mouseX = 0.0;
	double g_mouseY = 0.0;
	double g_scrollYOffset = 0.0;
	bool g_leftMouseDown = false;
	bool g_rightMouseDown = false;
	bool g_middleMouseDown = false;
	int g_resizedWidth = 0;
	int g_resizedHeight = 0;

	std::bitset<350> g_keydownmap;
	std::bitset<350> g_keypressedmap;

	void Init() {
		glfwSetScrollCallback(Renderer::g_window, ScrollCallback);
		glfwSetFramebufferSizeCallback(Renderer::g_window, ResizeCallback);
	}

	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		g_scrollYOffset = yoffset;
	}

	void ResizeCallback(GLFWwindow* window, int width, int height) {
		g_resizedWidth = width;
		g_resizedHeight = height;
	}

	void Update() {
		g_scrollYOffset = 0;
		g_resizedWidth = 0;
		g_resizedHeight = 0;

		// Keyboard
		for (int i = 32; i < 349; i++) {
			if (glfwGetKey(Renderer::g_window, i) == GLFW_PRESS) {
				if (!g_keydownmap[i]) {
					g_keypressedmap[i] = true;
				} else {
					g_keypressedmap[i] = false;
				}

				g_keydownmap[i] = true;
			} else {
				g_keypressedmap[i] = false;
				g_keydownmap[i] = false;
			}
		}

		glfwGetCursorPos(Renderer::g_window, &g_mouseX, &g_mouseY);

		g_leftMouseDown = glfwGetMouseButton(Renderer::g_window, GLFW_MOUSE_BUTTON_LEFT);
		g_rightMouseDown = glfwGetMouseButton(Renderer::g_window, GLFW_MOUSE_BUTTON_RIGHT);
		g_middleMouseDown = glfwGetMouseButton(Renderer::g_window, GLFW_MOUSE_BUTTON_MIDDLE);
	}

	bool IsKeyDown(uint16_t key) {
		return key < 350 ? g_keydownmap[key] : false;
	}

	bool IsKeyPressed(uint16_t key) {
		return key < 350 ? g_keypressedmap[key] : false;
	}

	bool IsLeftMouseDown() {
		return g_leftMouseDown;
	}

	bool IsMiddleMouseDown() {
		return g_middleMouseDown;
	}

	bool IsRightMouseDown() {
		return g_rightMouseDown;
	}

	double GetMouseX() {
		return g_mouseX;
	}

	double GetMouseY() {
		return g_mouseY;
	}

	double GetScrollYOffset() {
		return g_scrollYOffset;
	}
}
