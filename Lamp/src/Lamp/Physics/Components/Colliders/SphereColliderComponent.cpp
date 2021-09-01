#include "lppch.h"
#include "SphereColliderComponent.h"

#include "Lamp/Physics/Components/RigidbodyComponent.h"
#include "Lamp/Physics/Rigidbody.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(SphereColliderComponent);

	SphereColliderComponent::SphereColliderComponent()
		: EntityComponent("SphereColliderComponent")
	{
		SetComponentProperties
		({
			{ PropertyType::Float, "Radius", RegisterData(&m_Radius) }
		});
	}

	void SphereColliderComponent::Initialize()
	{
		m_pShape = new btSphereShape(btScalar(0.5f));
		if (auto& comp = m_pEntity->GetComponent<RigidbodyComponent>())
		{
			m_RigidbodyFound = true;
			comp->GetRigidbody()->SetCollisionShape(m_pShape, m_pEntity);
		}
	}

	void SphereColliderComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(SphereColliderComponent::OnPropertyChanged));
		dispatcher.Dispatch<EntityComponentAddedEvent>(LP_BIND_EVENT_FN(SphereColliderComponent::OnComponentAdded));
	}

	bool SphereColliderComponent::OnPropertyChanged(EntityPropertyChangedEvent& e)
	{
		m_pShape->setUnscaledRadius(btScalar(m_Radius));
		return false;
	}

	bool SphereColliderComponent::OnComponentAdded(EntityComponentAddedEvent& e)
	{
		if (m_RigidbodyFound)
		{
			return false;
		}

		if (auto& comp = m_pEntity->GetComponent<RigidbodyComponent>())
		{
			m_RigidbodyFound = true;
			comp->GetRigidbody()->SetCollisionShape(m_pShape, m_pEntity);
		}

		return false;
	}
}