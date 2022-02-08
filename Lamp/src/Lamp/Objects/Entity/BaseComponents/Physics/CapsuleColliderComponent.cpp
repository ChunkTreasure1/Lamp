#include "lppch.h"
#include "CapsuleColliderComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(CapsuleColliderComponent);

	CapsuleColliderComponent::CapsuleColliderComponent()
		: EntityComponent("CapsuleColliderComponent")
	{

	}

	void CapsuleColliderComponent::Initialize()
	{
	}

	void CapsuleColliderComponent::OnEvent(Event& e)
	{
	}

	void CapsuleColliderComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::Float, "Radius", RegisterData(&m_Specification.m_Radius) },
			{ PropertyType::Float, "Height", RegisterData(&m_Specification.m_Height) },
			{ PropertyType::Float3, "Offset", RegisterData(&m_Specification.m_Offset) },
			{ PropertyType::Bool, "Is Trigger", RegisterData(&m_Specification.m_IsTrigger) }
		};
	}
}