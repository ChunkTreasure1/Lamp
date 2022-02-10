#include "lppch.h"
#include "MeshColliderComponent.h"

namespace Lamp
{
	MeshColliderComponent::MeshColliderComponent()
		: EntityComponent("MeshColliderComponent")
	{}

	void MeshColliderComponent::Initialize()
	{
	}

	void MeshColliderComponent::OnEvent(Event& e)
	{
	}

	void MeshColliderComponent::SetComponentProperties()
	{
		m_componentProperties = 
		{
			{ PropertyType::Bool, "Is Trigger", RegisterData(&m_Specification.IsTrigger) }
		};
	}
}