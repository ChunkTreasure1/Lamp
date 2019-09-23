#include "Application.h"

#include "GLFW/glfw3.h"
#include "Lamp/Rendering/Texture/GLTexture.h"
#include "Lamp/Input/ResourceManager.h"
#include "Lamp/Log.h"

namespace Lamp
{
	Renderer* Application::s_pRenderer = nullptr;

	Application::Application()
	{
		if (!glfwInit())
		{
			LP_CORE_ERROR("Could not initialize GLFW");
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