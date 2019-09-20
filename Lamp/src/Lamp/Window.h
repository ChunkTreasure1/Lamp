#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <string>

namespace Lamp
{
	struct WindowProps
	{
		WindowProps(std::string title = "Lamp", uint32_t width = 1280, uint32_t height = 720, bool isVSync = true)
			: Title(title), Width(width), Height(height), IsVSync(isVSync)
		{}

		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool IsVSync;
	};

	enum WindowFlags
	{
		WINDOW_INVISIBLE = 0x1,
		WINDOW_FULLSCREEN = 0x2,
		WINDOW_BORDERLESS = 0x4
	};

	class Window
	{
	public:
		Window(WindowProps& props = WindowProps(), uint32_t currentFlags = 0);
		~Window();

		//Functions
		int Initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight, uint32_t currentFlags);
		void SwapBuffer();

		//Getting
		inline const uint32_t GetWidth() const { return m_Data.Width; }
		inline const uint32_t GetHeight() const { return m_Data.Height; }
		inline const bool GetIsVSync() const { return m_Data.IsVSync; }

		//Setting
		inline void SetIsVSync(bool state);

	private:
		//Member vars
		SDL_Window* m_pWindow;
		WindowProps m_Data;
	};
}


