#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"
#include "Sandbox/Sandbox.h"

class SandboxLayer : public Lamp::Application
{
public:
	SandboxLayer()
	{
		PushLayer(new Sandbox::Sandbox());
	};
	~SandboxLayer() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new SandboxLayer();
}