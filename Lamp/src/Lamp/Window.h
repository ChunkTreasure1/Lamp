#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
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

	class Window
	{
	public:
		Window(WindowProps& props = WindowProps());
		~Window();

		//Functions
		void SwapBuffer();

		//Getting
		inline const uint32_t GetWidth() const { return m_Data.Width; }
		inline const uint32_t GetHeight() const { return m_Data.Height; }
		inline const bool GetIsVSync() const { return m_Data.IsVSync; }

		//Setting
		inline void SetIsVSync(bool state);

	private:
		//Member vars
		GLFWwindow* m_pWindow;
		WindowProps m_Data;
	};
}


