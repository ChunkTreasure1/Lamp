#pragma once
#include "Lamp/Rendering/Window.h"
#include "Lamp/Rendering/Shader.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

	private:
		bool m_Running = true;

	private:
		static Application* s_pInstance;
		Shader* m_pShader;
		Window* m_pWindow;

		GLuint m_VertexBuffer;
	};

	Application* CreateApplication();
}