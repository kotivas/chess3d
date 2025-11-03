#pragma once
#include <bitset>
#include "../render/renderer.hpp"

namespace Input {

	void   Init();
	void   Update();
	void   ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void   ResizeCallback(GLFWwindow* window, int width, int height);

	bool   IsKeyDown(uint16_t key);
	bool   IsKeyPressed(uint16_t key);
	bool   IsLeftMouseDown();
	bool   IsMiddleMouseDown();
	bool   IsRightMouseDown();

	double GetMouseX();
	double GetMouseY();
	double GetScrollYOffset();

	extern double	g_mouseX;
	extern double	g_mouseY;
	extern double	g_scrollYOffset;
	extern bool		g_leftMouseDown;
	extern bool		g_rightMouseDown;
	extern bool		g_middleMouseDown;
	extern int		g_resizedWidth;
	extern int		g_resizedHeight;

	extern std::bitset<350> g_keydownmap;
	extern std::bitset<350> g_keypressedmap;
}
