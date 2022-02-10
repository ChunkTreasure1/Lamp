#include "lppch.h"
#include "RigidbodyComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(RigidbodyComponent);

	RigidbodyComponent::RigidbodyComponent()
		: EntityComponent("RigidbodyComponent")
	{
	}

	void RigidbodyComponent::Initialize()
	{
	}

	void RigidbodyComponent::OnEvent(Event& e)
	{
	}

	void RigidbodyComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::Int, "Body type", RegisterData(&m_Specification.m_BodyType) },
			{ PropertyType::Int, "Collision type", RegisterData(&m_Specification.m_CollisionDetection) },
			{ PropertyType::Float, "Mass", RegisterData(&m_Specification.m_Mass) },
			{ PropertyType::Float, "Linear Drag", RegisterData(&m_Specification.m_LinearDrag) },
			{ PropertyType::Float, "Angular Drag", RegisterData(&m_Specification.m_AngularDrag) },
			{ PropertyType::Bool, "Disable gravity", RegisterData(&m_Specification.m_DisableGravity) },
			{ PropertyType::Bool, "Is Kinematic", RegisterData(&m_Specification.IsKinematic) },
			{ PropertyType::Int, "Layer", RegisterData(&m_Specification.m_Layer) }
		};
	}
}