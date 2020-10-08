#include "lppch.h"
#include "DXTesting.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Rendering/RenderCommand.h"

namespace DXTesting
{
	DXTesting::DXTesting()
		: Lamp::Layer("DXTesting")
	{
	}
	void DXTesting::OnImGuiRender(Lamp::Timestep ts)
	{
	}
	void DXTesting::OnEvent(Lamp::Event& e)
	{
		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(DXTesting::OnUpdate));
	}

	bool DXTesting::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		Lamp::RenderCommand::SetClearColor({ 0.3f, 0.3f, 0.3f, 1.f });
		Lamp::RenderCommand::Clear();

		Lamp::Renderer3D::TestDraw();

		return false;
	}
}