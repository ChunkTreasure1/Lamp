#include "lppch.h"
#include "BoxColliderComponent.h"

#include "Lamp/Physics/Components/RigidbodyComponent.h"
#include "Lamp/Physics/Rigidbody.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(BoxColliderComponent);

	BoxColliderComponent::BoxColliderComponent()
		: EntityComponent("BoxColliderComponent")
	{
		SetComponentProperties
		({
			{ PropertyType::Float3, "Size", RegisterData(&m_Scale) }
		});
	}

	void BoxColliderComponent::Initialize()
	{
		m_pShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
		if (auto& comp = m_pEntity->GetComponent<RigidbodyComponent>())
		{
			m_RigidbodyFound = true;
			comp->GetRigidbody()->SetCollisionShape(m_pShape, m_pEntity);
		}
	}

	void BoxColliderComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(BoxColliderComponent::OnPropertyChanged));
		dispatcher.Dispatch<EntityComponentAddedEvent>(LP_BIND_EVENT_FN(BoxColliderComponent::OnComponentAdded));
	}

	bool BoxColliderComponent::OnPropertyChanged(EntityPropertyChangedEvent& e)
	{
		m_pShape = new btBoxShape(btVector3(m_Scale.x / 2, m_Scale.y / 2, m_Scale.z / 2));
		if (auto& comp = m_pEntity->GetComponent<RigidbodyComponent>())
		{
			comp->GetRigidbody()->SetCollisionShape(m_pShape, m_pEntity);
		}
		return false;
	}

	bool BoxColliderComponent::OnComponentAdded(EntityComponentAddedEvent& e)
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