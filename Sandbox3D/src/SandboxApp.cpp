#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Lamp/Entity/Base/EntityManager.h"
#include "Sandbox3D.h"

#include <glm/gtc/type_ptr.hpp>

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