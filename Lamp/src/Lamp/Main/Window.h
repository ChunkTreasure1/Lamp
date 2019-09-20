#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <string>

namespace Lamp
{
	enum WindowFlags
	{
		WINDOW_INVISIBLE = 0x1,
		WINDOW_FULLSCREEN = 0x2,
		WINDOW_BORDERLESS = 0x4
	};

	class Window
	{
	public:
		Window();
		~Window();

		//Functions
		int Initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight, uint32_t currentFlags);
		void SwapBuffer();

		//Getting
		int GetWindowWidth() { return m_WindowWidth; }
		int GetWindowHeight() { return m_WindowHeight; }

	private:
		//Member vars
		SDL_Window* m_pWindow;
		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;

	};
}


