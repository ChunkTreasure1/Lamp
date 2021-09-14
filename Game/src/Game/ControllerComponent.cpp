#include "ControllerComponent.h"

#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

LP_REGISTER_COMPONENT(ControllerComponent);

void ControllerComponent::Initialize()
{
}

void ControllerComponent::OnEvent(Lamp::Event& e)
{
	Lamp::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(ControllerComponent::OnUpdateEvent));
}

bool ControllerComponent::OnUpdateEvent(Lamp::AppUpdateEvent& e)
{
	if (Lamp::Input::IsKeyPressed(LP_KEY_W))
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() - glm::vec3(0.f, 0.f, 10.f * e.GetTimestep()));
	}

	if (Lamp::Input::IsKeyPressed(LP_KEY_S))
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() + glm::vec3(0.f, 0.f, 10.f * e.GetTimestep()));
	}

	return false;
}
