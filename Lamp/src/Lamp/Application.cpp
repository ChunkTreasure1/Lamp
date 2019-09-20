#include "Application.h"
#include <SDL/SDL.h>

namespace Lamp
{
	Application::Application()
	{
		//Create the camera
		m_pCamera = new Camera2D();

		//Initialize SDL
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		//Create the window
		m_pWindow = new Window();

		//Move to renderer
		m_pShader = new Shader("Shaders/colorShading.vert", "Shaders/colorShading.frag");

		m_pShader->AddAttribute("vertexPosition");
		m_pShader->AddAttribute("vertexColor");
		m_pShader->AddAttribute("vertexUV");

		m_pShader->LinkShaders();

		m_pSpritebatch = new SpriteBatch();
	}

	Application::~Application()
	{
		delete m_pShader;
		delete m_pSpritebatch;
		delete m_pWindow;
		delete m_pCamera;
	}

	void Application::Run()
	{
		while (m_Running)
		{

		}
	}

	void Application::Initialize()
	{
	}
}