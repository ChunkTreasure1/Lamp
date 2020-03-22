#include "lppch.h"
#include "RigidbodyComponent.h"

#include "Lamp/Physics/Physics.h"

namespace Lamp
{
	bool RigidbodyComponent::s_Registered = LP_REGISTER_COMPONENT(RigidbodyComponent);

	void RigidbodyComponent::Initialize()
	{
	}

	void RigidbodyComponent::Update(Lamp::Timestep someTS)
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() - glm::vec3(0.f, Physics::GetGravity(), 0.f));
	}

	void RigidbodyComponent::OnEvent(Lamp::Event& someE)
	{
	}

	void RigidbodyComponent::Draw()
	{
	}
}