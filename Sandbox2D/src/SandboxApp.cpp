#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"
#include "Sandbox2D.h"

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		PushLayer(new Sandbox2D::Sandbox2D());
	}
	~Sandbox() {}
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}