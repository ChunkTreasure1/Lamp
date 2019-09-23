#include "Application.h"

#include "GLFW/glfw3.h"
#include "Lamp/Rendering/Texture/GLTexture.h"
#include "Lamp/Input/ResourceManager.h"

namespace Lamp
{
	Renderer* Application::s_pRenderer = nullptr;

	Application::Application()
	{
		if (!glfwInit())
		{
			//LOG
		}

		//Create the window
		m_pWindow = new Window();
		s_pRenderer = new Renderer(m_pWindow);
	}

	Application::~Application()
	{
		delete m_pWindow;
	}

	void Application::Run()
	{
		while (m_sRunning)
		{
			s_pRenderer->Draw();
		}
	}
}