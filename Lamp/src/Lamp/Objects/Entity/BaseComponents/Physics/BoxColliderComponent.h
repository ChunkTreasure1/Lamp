#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Objects/Entity/ComponentRegistry.h"

namespace Lamp
{
	class BoxColliderComponent final : public EntityComponent
	{
	public:
		BoxColliderComponent();


		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		//////////////

		static Ref<EntityComponent> Create() { return CreateRef<BoxColliderComponent>(); }
		static std::string GetFactoryName() { return "BoxColliderComponent"; }

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