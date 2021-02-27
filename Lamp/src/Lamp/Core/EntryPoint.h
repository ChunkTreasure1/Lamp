#pragma once
#include "Lamp/Core/Application.h"
#include "Lamp/Core/Log.h"

extern Lamp::Application* Lamp::CreateApplication();

void t(int) {}

int main(int argc, char** argv)
{
	Lamp::Log::Initialize();
	
	LP_PROFILE_BEGIN_SESSION("Startup", "Profiling/LampProfile_Startup.json");
	auto pApp = Lamp::CreateApplication();
	LP_PROFILE_END_SESSION();

	LP_PROFILE_BEGIN_SESSION("Runtime", "Profiling/LampProfile_Runtime.json");
	pApp->Run();
	LP_PROFILE_END_SESSION();

	LP_PROFILE_BEGIN_SESSION("Shutdown", "Profiling/LampProfile_Shutdown.json");
	delete pApp;
	LP_PROFILE_END_SESSION();

	return 0;
}