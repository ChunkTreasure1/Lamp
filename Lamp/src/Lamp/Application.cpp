#include "Application.h"
#include <SDL/SDL.h>
#include <iostream>

#include "Lamp/Rendering/Texture/GLTexture.h"
#include "Lamp/Rendering/ResourceManager.h"

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
		m_pFPSLimiter = new FPSLimiter(60.f);
	}

	Application::~Application()
	{
		delete m_pFPSLimiter;
		delete m_pShader;
		delete m_pSpritebatch;
		delete m_pWindow;
		delete m_pCamera;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			m_pFPSLimiter->Begin();

			m_Time += 0.01;
			m_pCamera->Update();
			DrawGame();

			m_FPS = m_pFPSLimiter->End();
			//Print the fps every 10 frames
			static int frameCounter = 0;
			frameCounter++;

			if (frameCounter == 10000)
			{
				std::cout << m_FPS << std::endl;
				frameCounter = 0;
			}
		}
	}

	void Application::Initialize()
	{
	}

	void Application::DrawGame()
	{
		//Clear the screen
		glClearDepth(1.0);
		glClearColor(0.1, 0.1, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Bind the shader
		m_pShader->Bind();

		glActiveTexture(GL_TEXTURE0);

		//Set the texture sampler
		GLint textureLocation = m_pShader->GetUniformLocation("textureSampler");
		glUniform1i(textureLocation, 0);

		//Set the camera matrix
		GLuint location = m_pShader->GetUniformLocation("P");
		glm::mat4 cameraMatrix = m_pCamera->GetMatrix();
		glUniformMatrix4fv(location, 1, GL_FALSE, &(cameraMatrix[0][0]));

		//Start drawing
		m_pSpritebatch->Begin();

		glm::vec4 pos(0.f, 0.f, 50.f, 50.f);
		glm::vec4 uv(0.f, 0.f, 1.f, 1.f);
		static GLTexture texture = ResourceManager::GetTexture("Textures/ff.png");

		Color color;
		color.R = 255;
		color.G = 255;
		color.B = 255;
		color.A = 255;

		m_pSpritebatch->Draw(pos + glm::vec4(50, 50, 0, 0), uv, texture.Id, 0.f, color);

		//End drawing
		m_pSpritebatch->End();
		m_pSpritebatch->RenderBatches();

		glBindTexture(GL_TEXTURE_2D, 0);
		m_pShader->Unbind();
		m_pWindow->SwapBuffer();
	}
}