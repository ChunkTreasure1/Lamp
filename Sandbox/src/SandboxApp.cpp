#include <Lamp.h>
#include "Lamp/Rendering/Camera2D.h"

class Sandbox : public Lamp::Application
{
public:

	Sandbox() 
	{
		m_pCamera = new Lamp::Camera2D(1280, 720);
	}
	~Sandbox() {}

private:
	Lamp::Camera2D* m_pCamera;
};

Lamp::Application* Lamp::CreateApplication() 
{
	return new Sandbox();
}