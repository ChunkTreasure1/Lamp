#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Lamp/Entity/Base/EntityManager.h"
#include "Sandbox2D.h"

#include <glm/gtc/type_ptr.hpp>

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		PushLayer(new Sandbox2D::Sandbox2D());
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}