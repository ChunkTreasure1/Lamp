#include "lppch.h"
#include "CapsuleColliderComponent.h"

#include "Lamp/Physics/Components/RigidbodyComponent.h"
#include "Lamp/Physics/Rigidbody.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(CapsuleColliderComponent);

	CapsuleColliderComponent::CapsuleColliderComponent()
		: EntityComponent("CapsuleColliderComponent")
	{
		SetComponentProperties
		({
			{ PropertyType::Float, "Radius", RegisterData(&m_Radius) },
			{ PropertyType::Float, "Height", RegisterData(&m_Height) }
		});
	}

	void CapsuleColliderComponent::Initialize()
	{
		m_pShape = new btCapsuleShape(btScalar(m_Radius), btScalar(m_Height));
		if (auto& comp = m_pEntity->GetComponent<RigidbodyComponent>())
		{
			m_RigidbodyFound = true;
			comp->GetRigidbody()->SetCollisionShape(m_pShape, m_pEntity);
		}
	}

	void CapsuleColliderComponent::OnEvent(Event& e)
	{

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(CapsuleColliderComponent::OnPropertyChanged));
		dispatcher.Dispatch<EntityComponentAddedEvent>(LP_BIND_EVENT_FN(CapsuleColliderComponent::OnComponentAdded));
	}

	bool CapsuleColliderComponent::OnPropertyChanged(EntityPropertyChangedEvent& e)
	{
		m_pShape = new btCapsuleShape(btScalar(m_Radius), btScalar(m_Height));
		if (auto& comp = m_pEntity->GetComponent<RigidbodyComponent>())
		{
			comp->GetRigidbody()->SetCollisionShape(m_pShape, m_pEntity);
		}
		return false;
	}

	bool CapsuleColliderComponent::OnComponentAdded(EntityComponentAddedEvent& e)
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