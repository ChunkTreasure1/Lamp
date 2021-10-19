#include "ControllerComponent.h"

#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

LP_REGISTER_COMPONENT(ControllerComponent);

void ControllerComponent::Initialize()
{
	m_pRigidbody = m_pEntity->GetComponent<Lamp::RigidbodyComponent>();
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
		m_pEntity->SetPosition(m_pEntity->GetPosition() - glm::vec3(0.f, 0.f, m_Speed * e.GetTimestep()));
	}

	if (Lamp::Input::IsKeyPressed(LP_KEY_S))
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() + glm::vec3(0.f, 0.f, m_Speed * e.GetTimestep()));
	}

	if (Lamp::Input::IsKeyPressed(LP_KEY_A))
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() - glm::vec3(m_Speed * e.GetTimestep(), 0.f, 0.f));
	}

	if (Lamp::Input::IsKeyPressed(LP_KEY_D))
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() + glm::vec3(m_Speed * e.GetTimestep(), 0.f, 0.f));
	}

	return false;
}
