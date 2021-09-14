#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"

namespace Lamp
{
	class BoxColliderComponent final : public EntityComponent
	{
	public:
		BoxColliderComponent()
			: EntityComponent("BoxColliderComponent")
		{
			SetComponentProperties
			({
				{ PropertyType::Float3, "Size", RegisterData(&m_Specification.m_Size) },
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
		static Ref<EntityComponent> Create() { return CreateRef<BoxColliderComponent>(); }
		static std::string GetFactoryName() { return "BoxColliderComponent"; }

	public:
		struct BoxColliderSpecification
		{
			glm::vec3 m_Size = { 1.f, 1.f, 1.f };
			glm::vec3 m_Offset = { 0.f, 0.f, 0.f };
			bool m_IsTrigger = false;
			Ref<PhysicsMaterial> m_Material;
		};

		inline BoxColliderSpecification& GetSpecification() { return m_Specification; }

	private:
		BoxColliderSpecification m_Specification;
	};
}