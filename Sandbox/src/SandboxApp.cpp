#include <Lamp.h>

class ExampleLayer : public Lamp::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{ }

	void Update() override
	{
	}
};

class Sandbox : public Lamp::Application
{
public:

	Sandbox() 
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {}
};

Lamp::Application* Lamp::CreateApplication() 
{
	return new Sandbox();
}