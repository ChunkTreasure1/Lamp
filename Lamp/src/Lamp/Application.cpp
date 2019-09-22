#include "Application.h"
#include <SDL/SDL.h>

#include "Lamp/Rendering/Texture/GLTexture.h"
#include "Lamp/Input/ResourceManager.h"

namespace Lamp
{
	Renderer* Application::s_pRenderer = nullptr;

	Application::Application()
	{
		//Initialize SDL
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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