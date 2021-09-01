#include "lppch.h"
#include "RigidbodyComponent.h"

#include "Lamp/Physics/Rigidbody.h"
#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(RigidbodyComponent);

	RigidbodyComponent::RigidbodyComponent()
		: EntityComponent("RigidbodyComponent")
	{
		SetComponentProperties
		({
			{ PropertyType::Bool, "Is Static", RegisterData(&m_IsStatic) },
			{ PropertyType::Float, "Mass", RegisterData(&m_Mass) },
		});
	}

	void RigidbodyComponent::Initialize()
	{
		m_pRigidbody = PhysicsEngine::Get()->CreateRigidBody(m_pEntity);
		m_pRigidbody->GetRigidbody()->setUserPointer(this);
		m_pRigidbody->SetStatic(m_IsStatic);
		m_pRigidbody->SetMass(m_Mass);

		m_pEntity->SetRigidbody(m_pRigidbody);
	}

	void RigidbodyComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(RigidbodyComponent::OnUpdate));
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(RigidbodyComponent::OnEntityPropertyChanged));
	}

	bool RigidbodyComponent::OnUpdate(AppUpdateEvent& e)
	{
		btTransform& tr = m_pRigidbody->GetTransform();
		m_pEntity->SetPhysicsPosition({ tr.getOrigin().getX(), tr.getOrigin().getY(), tr.getOrigin().getZ() });

		return false;
	}

	bool RigidbodyComponent::OnEntityPropertyChanged(EntityPropertyChangedEvent& e)
	{
		m_pRigidbody->SetStatic(m_IsStatic);
		m_pRigidbody->SetMass(m_Mass);
		return false;
	}
}