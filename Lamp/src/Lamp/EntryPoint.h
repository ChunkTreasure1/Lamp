#pragma once

#ifdef LP_PLATFORM_WINDOWS

extern Lamp::Application* Lamp::CreateApplication();

void main(int argc, char** argv)
{
	auto* pApp = Lamp::CreateApplication();
	pApp->Run();

	delete pApp;
}

#endif