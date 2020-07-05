#pragma once
#include "Lamp/Core/Application.h"
#include "Lamp/Core/Log.h"

extern Lamp::Application* Lamp::CreateApplication();

int main(int argc, char** argv)
{
	Lamp::Log::Init();
	LP_CORE_INFO("Initialized Log");

	auto pApp = Lamp::CreateApplication();
	pApp->Run();

	delete pApp;

	return 0;
}