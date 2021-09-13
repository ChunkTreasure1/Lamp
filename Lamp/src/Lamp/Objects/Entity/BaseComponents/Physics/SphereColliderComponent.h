#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"

namespace Lamp
{
	class SphereColliderComponent final : public EntityComponent
	{
	public:
		SphereColliderComponent()
			: EntityComponent("SphereColliderComponent")
		{
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override { return EventType::None; }
		//////////////

	public:
		static Ref<EntityComponent> Create() { return CreateRef<SphereColliderComponent>(); }
		static std::string GetFactoryName() { return "SphereColliderComponent"; }

	public:
		struct SphereColliderSpecification
		{
			float m_Radius = 0.5f;
			glm::vec3 m_Offset = { 0.f, 0.f, 0.f };
			bool m_IsTrigger = false;
			Ref<PhysicsMaterial> m_Material;
		};

		inline SphereColliderSpecification& GetSpecification() { return m_Specification; }
	private:
		SphereColliderSpecification m_Specification;
	};
}