#include <Lamp.h>

class Sandbox : public Lamp::Application
{
public:

	Sandbox() {}
	~Sandbox() {}
};

Lamp::Application* Lamp::CreateApplication() 
{
	return new Sandbox();
}