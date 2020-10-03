#include <Lamp.h>

#include "Lamp/Core/EntryPoint.h"
#include "Sandbox3D/Sandbox3D.h"
#include "Sandbox2D/Sandbox2D.h"

#include "DXTesting/DXTesting.h"

#define LP_SANDBOX_3D

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
//#if defined(LP_SANDBOX_3D)
//		PushLayer(new Sandbox3D::Sandbox3D());
//#elif defined(LP_SANDBOX_2D)
//		PushLayer(new Sandbox2D::Sandbox2D());
//#else
//		LP_CORE_ASSERT(false, "No Sandbox defined!");
//#endif

		PushLayer(new DXTesting::DXTesting());
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}