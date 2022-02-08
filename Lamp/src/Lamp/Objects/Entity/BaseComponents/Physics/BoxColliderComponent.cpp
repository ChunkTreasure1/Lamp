#include "lppch.h"
#include "BoxColliderComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(BoxColliderComponent);

	BoxColliderComponent::BoxColliderComponent()
		: EntityComponent("BoxColliderComponent")
	{
	}

	void BoxColliderComponent::Initialize()
	{
	}

	void BoxColliderComponent::OnEvent(Event& e)
	{
	}

	void BoxColliderComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::Float3, "Size", RegisterData(&m_Specification.m_Size) },
			{ PropertyType::Float3, "Offset", RegisterData(&m_Specification.m_Offset) },
			{ PropertyType::Bool, "Is Trigger", RegisterData(&m_Specification.m_IsTrigger) }
		};
	}
}