#include "Window.h"
#include "Lamp/Log.h"

namespace Lamp
{
	Window::Window(WindowProps& props)
	{
		//Create the window
		m_pWindow = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
		if (!m_pWindow)
		{
			glfwTerminate();
			LP_CORE_ERROR("Could not create window!");
		}

		//Set the current context
		glfwMakeContextCurrent(m_pWindow);

		//Initialize GLEW and error check it
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			LP_CORE_ERROR("Could not initialize GLEW!");
		}

		SetIsVSync(props.IsVSync);

		//Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.IsVSync = props.IsVSync;
	}

	Window::~Window()
	{
		delete m_pWindow;
		glfwTerminate();
	}

	//Swaps the rendering buffer
	void Window::SwapBuffer()
	{
		//Swap the windows
		glfwSwapBuffers(m_pWindow);
	}

	inline void Window::SetIsVSync(bool state)
	{
		if (state)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}

		m_Data.IsVSync = state;
	}
}
