#pragma once

namespace Lamp
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
	
	static Application* CreateApplication();
}