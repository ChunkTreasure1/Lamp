#pragma once
#include "Lamp/Core/Application.h"
#include "Lamp/Core/Log.h"

extern Lamp::Application* Lamp::CreateApplication();

int main(int argc, char** argv)
{
	Lamp::Log::Initialize();

	auto pApp = Lamp::CreateApplication();
	pApp->Run();

	delete pApp;

	return 0;
}