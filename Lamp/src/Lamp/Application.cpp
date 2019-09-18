#include "lppch.h"
#include "Application.h"

#include "Lamp/Core.h"
#include "GL/glew.h"

namespace Lamp 
{
	//Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		m_pWindow = new Window();
	}

	Application::~Application()
	{
		m_Running = false;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			if (glfwWindowShouldClose(m_pWindow->GetNativeWindow()))
			{
				m_Running = false;
			}

			glClear(GL_COLOR_BUFFER_BIT);

			glfwSwapBuffers(m_pWindow->GetNativeWindow());
			glfwPollEvents();

			//Updates all the layers
		}

		glfwTerminate();
	}
}