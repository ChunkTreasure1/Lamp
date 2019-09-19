#include "MainGame.h"

#include <Main/CactusEngine.h>
#include <iostream>
#include <Main/Errors.h>
#include <Rendering/Systems/ResourceManager.h>

MainGame::MainGame() 
	: m_Time(0), 
	m_WindowWidth(1024), 
	m_WindowHeight(768), 
	m_GameState(GameState::eGS_Play),
	m_MaxFPS(60.0f)
{
	m_Camera.Initialize(m_WindowWidth, m_WindowHeight);
}

MainGame::~MainGame()
{
}

void MainGame::Run()
{
	//Initializes all the systems such as SDL and OpenGL
	InitSystems();

	//Start the game loop
	GameLoop();
}

//Initializes all the necessary systems
void MainGame::InitSystems()
{
	CactusEngine::Initialize();

	//Creates a window
	m_Window.Initialize("Cactus Engine", m_WindowWidth, m_WindowHeight, 0);
	InitShaders();
	m_SpriteBatch.Initialize();

	m_FPSLimiter.Initialize(m_MaxFPS);
}

//The main game loop
void MainGame::GameLoop()
{
	while (m_GameState != GameState::eGS_Exit)
	{
		m_FPSLimiter.Begin();

		ProcessInput();
		m_Time += 0.01;
		m_Camera.Update();
		
		for (int i = 0; i < m_Bullets.size();)
		{
			if (m_Bullets[i].Update())
			{
				m_Bullets[i] = m_Bullets.back();
				m_Bullets.pop_back();
			}
			else
			{
				i++;
			}
		}
		DrawGame();

		m_FPS = m_FPSLimiter.End();

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

//Checks for input
void MainGame::ProcessInput()
{
	SDL_Event evnt;
	const float CAMERA_SPEED = 2.f;
	const float SCALE_SPEED = 0.1f;

	while (SDL_PollEvent(&evnt))
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			m_GameState = GameState::eGS_Exit;
			break;

		case SDL_MOUSEMOTION:
			m_InputManager.SetMouseCoords(evnt.motion.x, evnt.motion.y);
			break;

		case SDL_KEYDOWN:
			m_InputManager.KeyPressed(evnt.key.keysym.sym);
			break;

		case SDL_KEYUP:
			m_InputManager.KeyReleased(evnt.key.keysym.sym);
			break;

		case SDL_MOUSEBUTTONDOWN:
			m_InputManager.KeyPressed(evnt.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			m_InputManager.KeyReleased(evnt.button.button);
			break;
		}
	}

	//Checks for key presses
	if (m_InputManager.IsKeyPressed(SDLK_w))
	{
		m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec2(0.0f, CAMERA_SPEED));
	}
	if (m_InputManager.IsKeyPressed(SDLK_s))
	{
		m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec2(0.0f, -CAMERA_SPEED));
	}
	if (m_InputManager.IsKeyPressed(SDLK_a))
	{
		m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec2(-CAMERA_SPEED, 0.0f));
	}
	if (m_InputManager.IsKeyPressed(SDLK_d))
	{
		m_Camera.SetPosition(m_Camera.GetPosition() + glm::vec2(CAMERA_SPEED, 0.0f));
	}
	if (m_InputManager.IsKeyPressed(SDLK_q))
	{
		m_Camera.SetScale(m_Camera.GetScale() + SCALE_SPEED);
	}
	if (m_InputManager.IsKeyPressed(SDLK_e))
	{
		m_Camera.SetScale(m_Camera.GetScale() - SCALE_SPEED);
	}

	if (m_InputManager.IsKeyPressed(SDL_BUTTON_LEFT))
	{
		glm::vec2 mouseCoords = m_InputManager.GetMouseCoords();
		mouseCoords = m_Camera.ScreenToWorldCoords(mouseCoords);

		glm::vec2 playerPos(0.f);
		glm::vec2 dir = mouseCoords - playerPos;

		dir = glm::normalize(dir);
		m_Bullets.emplace_back(playerPos, dir, 1.f, 1000);

		std::cout << mouseCoords.x << ", " << mouseCoords.y << std::endl;
	}
}

//Renders the game
void MainGame::DrawGame()
{
	//Clear the screen
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_ColorProgram.Use();

	glActiveTexture(GL_TEXTURE0);

	//Set the texture sampler
	GLint textureLocation = m_ColorProgram.GetUniformLocation("textureSampler");
	glUniform1i(textureLocation, 0);
	
	//Set the camera matrix
	GLuint pLocation = m_ColorProgram.GetUniformLocation("P");
	glm::mat4 cameraMatrix = m_Camera.GetMatrix();
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));

	//Start drawing
	m_SpriteBatch.Begin();

	glm::vec4 pos(0.f, 0.f, 50.f, 50.f);
	glm::vec4 uv(0.f, 0.f, 1.f, 1.f);
	static CactusEngine::GLTexture texture = CactusEngine::ResourceManager::GetTexture("Textures/ff.png");
	
	CactusEngine::Color color;
	color.R = 255;
	color.G = 255;
	color.B = 255;
	color.A = 255;

	m_SpriteBatch.Draw(pos + glm::vec4(50, 50, 0, 0), uv, texture.Id, 0.f, color);

	for (int i = 0; i < m_Bullets.size(); i++)
	{
		m_Bullets[i].Draw(m_SpriteBatch);
	}

	//End drawing
	m_SpriteBatch.End();
	m_SpriteBatch.RenderBatches();

	glBindTexture(GL_TEXTURE_2D, 0);
	m_ColorProgram.Unuse();

	m_Window.SwapBuffer();
}

//Initalize the shaders
void MainGame::InitShaders()
{
	m_ColorProgram.CompileShaders("Shaders/colorShading.vert", "Shaders/colorShading.frag");

	m_ColorProgram.AddAttribute("vertexPosition");
	m_ColorProgram.AddAttribute("vertexColor");
	m_ColorProgram.AddAttribute("vertexUV");

	m_ColorProgram.LinkShaders();
}