#pragma once
#include "Window.h"
#include "Lamp/Rendering/Camera2D.h"
#include "Lamp/Rendering/SpriteBatch.h"
#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Timing.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void Initialize();
		void DrawGame();

	private:
		static const bool m_Running = true;
		
		Window* m_pWindow;
		Camera2D* m_pCamera;
		SpriteBatch* m_pSpritebatch;
		Shader* m_pShader;
		FPSLimiter* m_pFPSLimiter;

		float m_Time = 0;
		float m_FPS = 0;
	};
	
	static Application* CreateApplication();
}