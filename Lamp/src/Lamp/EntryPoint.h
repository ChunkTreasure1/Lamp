#pragma once
#include "Lamp/Application.h"

extern Lamp::Application* Lamp::CreateApplication();

int main(int argc, char** argv)
{
	auto pApp = Lamp::CreateApplication();
	pApp->Run();

	delete pApp;

	return 0;
}