#pragma once
#include "Window.h"
#include "Lamp/Rendering/Camera2D.h"
#include "Lamp/Rendering/SpriteBatch.h"
#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void Initialize();

	private:
		static const bool m_Running = true;
		
		Window* m_pWindow;
		Camera2D* m_pCamera;
		SpriteBatch* m_pSpritebatch;
		Shader* m_pShader;
	};
	
	static Application* CreateApplication();
}