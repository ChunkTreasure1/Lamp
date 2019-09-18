#include "lppch.h"
#include "Window.h"

#include "Lamp/Core.h"
#include "Lamp/Log.h"

namespace Lamp
{
	Window::Window(WindowProps & props)
	{
		m_Data = props;
		if (!Create())
		{
			LP_CORE_ASSERT(false, "Could not create window!");
		}
	}

	Window::~Window()
	{
		glfwTerminate();
	}

	
	void Window::SetVSync(bool state)
	{
	}

	inline const bool Window::IsVSync() const
	{
		return false;
	}

	bool Window::Create()
	{
		if (!glfwInit())
		{
			LP_CORE_ERROR("Could not initialize GLFW!");
			return false;
		}

		m_pWindow = glfwCreateWindow(m_Data.Width, m_Data.Height, 
									 m_Data.Title.c_str(), NULL, NULL);
		if (!m_pWindow)
		{
			glfwTerminate();
			LP_CORE_ERROR("Could not create GLFW window!");

			return false;
		}

		m_pContext = new OpenGLContext(m_pWindow);
		return true;
	}
}