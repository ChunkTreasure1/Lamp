#pragma once

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <vector>

#include <Lamp/Rendering/Systems/GLSLProgram.h>
#include <Lamp/Rendering/Textures/Sprite.h>
#include <Lamp/Rendering/Textures/GLTexture.h>

#include <Lamp/Main/Window.h>
#include <Lamp/Rendering/Systems/Camera2D.h>
#include <Lamp/Rendering/Textures/SpriteBatch.h>

#include <Lamp/Main/InputManager.h>
#include <Lamp/Main/Timing.h>
#include <Lamp/Application.h>

enum class GameState
{
	eGS_Play,
	eGS_Exit
};

class SandboxApp : Lamp::Application
{
public:
	SandboxApp();
	~SandboxApp();

	//Functions
	void Run();


private:

	//Member functions
	void InitSystems();
	void GameLoop();
	void ProcessInput();

	void DrawGame();
	void InitShaders();

	//Member vars
	Lamp::Window m_Window;
	int m_WindowHeight;
	int m_WindowWidth;

	GameState m_GameState;
	Lamp::GLSLProgram m_ColorProgram;
	Lamp::FPSLimiter m_FPSLimiter;

	Lamp::Camera2D m_Camera;
	Lamp::SpriteBatch m_SpriteBatch;
	Lamp::InputManager m_InputManager;

	float m_Time;
	float m_FPS;
	float m_MaxFPS;
};