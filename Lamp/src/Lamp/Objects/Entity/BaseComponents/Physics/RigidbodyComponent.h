#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"

#include "Lamp/Event/EntityEvent.h"

namespace Lamp
{
	class RigidbodyComponent final : public EntityComponent
	{
	public:
		RigidbodyComponent()
			: EntityComponent("RigidbodyComponent")
		{
			SetComponentProperties
			({
				{ PropertyType::Int, "Body type", RegisterData(&m_Specification.m_BodyType) },
				{ PropertyType::Int, "Collision type", RegisterData(&m_Specification.m_CollisionDetection) },
				{ PropertyType::Float, "Mass", RegisterData(&m_Specification.m_Mass) },
				{ PropertyType::Float, "Linear Drag", RegisterData(&m_Specification.m_LinearDrag) },
				{ PropertyType::Float, "Angular Drag", RegisterData(&m_Specification.m_AngularDrag) },
				{ PropertyType::Bool, "Disable gravity", RegisterData(&m_Specification.m_DisableGravity) },
				{ PropertyType::Bool, "Is Kinematic", RegisterData(&m_Specification.IsKinematic) },
				{ PropertyType::Int, "Layer", RegisterData(&m_Specification.m_Layer) }
			});
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		//////////////

	public:
		static Ref<EntityComponent> Create() { return CreateRef<RigidbodyComponent>(); }
		static std::string GetFactoryName() { return "RigidbodyComponent"; }

	public:
		enum class Type : uint32_t
		{
			Static, Dynamic
		};

		enum class CollisionDetectionType : uint32_t
		{
			Discrete, Continuous, ContinuousSpeculative
		};

		struct RigidbodySpecification
		{
			Type m_BodyType = Type::Dynamic;
			CollisionDetectionType m_CollisionDetection = CollisionDetectionType::Discrete;

			float m_Mass = 1.f;
			float m_LinearDrag = 1.f;
			float m_AngularDrag = 1.f;

			bool m_DisableGravity = false;
			bool IsKinematic = false;
			uint32_t m_Layer = 0;
		};

	public:
		inline RigidbodySpecification& GetSpecification() { return m_Specification; }

	private:
		RigidbodySpecification m_Specification;
	};
}