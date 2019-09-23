#pragma once
#include "Lamp/Application.h"
#include "Lamp/Log.h"

extern Lamp::Application* Lamp::CreateApplication();

int main(int argc, char** argv)
{
	Lamp::Log::Init();
	LP_CORE_WARN("Initialized Log");
	LP_INFO("Initialized Log");

	auto pApp = Lamp::CreateApplication();
	pApp->Run();

	delete pApp;

	return 0;
}