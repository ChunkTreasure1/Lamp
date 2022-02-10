#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Objects/Entity/ComponentRegistry.h"

namespace Lamp
{
	class CapsuleColliderComponent final : public EntityComponent
	{
	public:
		CapsuleColliderComponent();

		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		//////////////

		static Ref<EntityComponent> Create() { return CreateRef<CapsuleColliderComponent>(); }
		static std::string GetFactoryName() { return "CapsuleColliderComponent"; }

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