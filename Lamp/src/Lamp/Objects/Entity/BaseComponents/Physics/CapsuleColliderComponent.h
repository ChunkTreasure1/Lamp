#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"

namespace Lamp
{
	class CapsuleColliderComponent final : public EntityComponent
	{
	public:
		CapsuleColliderComponent()
			: EntityComponent("CapsuleColliderComponent")
		{
			SetComponentProperties
			({
				{ PropertyType::Float, "Radius", RegisterData(&m_Specification.m_Radius) },
				{ PropertyType::Float, "Height", RegisterData(&m_Specification.m_Height) },
				{ PropertyType::Float3, "Offset", RegisterData(&m_Specification.m_Offset) },
				{ PropertyType::Bool, "Is Trigger", RegisterData(&m_Specification.m_IsTrigger) }
			});
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint32_t GetSize() { return sizeof(*this); }
		virtual uint64_t GetEventMask() override { return EventType::None; }
		//////////////

	public:
		static Ref<EntityComponent> Create() { return CreateRef<CapsuleColliderComponent>(); }
		static std::string GetFactoryName() { return "CapsuleColliderComponent"; }

	public:
		struct CapsuleColliderSpecification
		{
			float m_Radius = 0.5f;
			float m_Height = 1.f;
			glm::vec3 m_Offset = { 0.f, 0.f, 0.f };
			bool m_IsTrigger = false;
			Ref<PhysicsMaterial> m_Material;
		};

		inline CapsuleColliderSpecification& GetSpecification() { return m_Specification; }

	private:
		CapsuleColliderSpecification m_Specification;
	};
}