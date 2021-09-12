#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"
#include "Sandbox3D/Sandbox3D.h"

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		PushLayer(new Sandbox3D::Sandbox3D());
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}