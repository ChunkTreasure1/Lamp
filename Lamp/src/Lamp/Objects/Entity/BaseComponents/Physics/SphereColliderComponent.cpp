#include "lppch.h"
#include "SphereColliderComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(SphereColliderComponent);

	SphereColliderComponent::SphereColliderComponent()
		: EntityComponent("SphereColliderComponent")
	{

	}

	void SphereColliderComponent::Initialize()
	{
	}

	void SphereColliderComponent::OnEvent(Event& e)
	{
	}

	void SphereColliderComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::Float, "Radius", RegisterData(&m_Specification.m_Radius) },
			{ PropertyType::Float3, "Offset", RegisterData(&m_Specification.m_Offset) },
			{ PropertyType::Bool, "Is Trigger", RegisterData(&m_Specification.m_IsTrigger) }
		};
	}
}