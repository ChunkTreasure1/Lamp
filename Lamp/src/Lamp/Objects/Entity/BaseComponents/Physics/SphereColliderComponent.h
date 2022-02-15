#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Objects/Entity/ComponentRegistry.h"

namespace Lamp
{
	class SphereColliderComponent final : public EntityComponent
	{
	public:
		SphereColliderComponent();

		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		//////////////

		static Ref<EntityComponent> Create() { return CreateRef<SphereColliderComponent>(); }
		static std::string GetFactoryName() { return "SphereColliderComponent"; }

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