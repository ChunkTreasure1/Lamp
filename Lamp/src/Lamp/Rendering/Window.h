#pragma once
#include "GLFW/glfw3.h"
#include "Lamp/Rendering/OpenGLContext.h"

#include <string>

namespace Lamp
{
	struct WindowProps
	{
		WindowProps(uint32_t width = 1280, uint32_t height = 720, std::string title = "Lamp", bool isVSync = true)
			: Width(width), Height(height), Title(title), IsVSync(isVSync)
		{
		}

		uint32_t Width;
		uint32_t Height;
		std::string Title;
		bool IsVSync;
	};

	class Window
	{
	public:
		Window(WindowProps& props = WindowProps());
		~Window();

		//Setting
		void SetVSync(bool state);

		//Getting
		inline const bool IsVSync() const;
		inline GLFWwindow* GetNativeWindow() { return m_pWindow; }

		//Public methods
		bool Create();

	private:
		GLFWwindow* m_pWindow;
		OpenGLContext* m_pContext;
		WindowProps m_Data;
	};
}