#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"
#include "Sandbox/SandboxLayer.h"

class SandboxApp : public Lamp::Application
{
public:
	SandboxApp()
	{
		PushLayer(new Sandbox::SandboxLayer());
	};
	~SandboxApp() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new SandboxApp();
}