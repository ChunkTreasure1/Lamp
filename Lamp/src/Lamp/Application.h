#pragma once

namespace Lamp
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

	private:
		static Application* s_pInstance;
	};

	Application* CreateApplication();
}