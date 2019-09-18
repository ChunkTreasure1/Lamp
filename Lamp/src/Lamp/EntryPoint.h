#pragma once

#ifdef LP_PLATFORM_WINDOWS

extern Lamp::Application* Lamp::CreateApplication();

void main(int argc, char** argv)
{
	//Lamp::Log::Init();
	//LP_CORE_INFO("Initialized log!");

	auto* pApp = Lamp::CreateApplication();
	pApp->Run();

	delete pApp;
}

#endif