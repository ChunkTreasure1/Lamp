#pragma once

#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Objects/Entity/ComponentRegistry.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Physics/PhysicsMaterial.h"

namespace Lamp
{
	class MeshColliderComponent final : public EntityComponent
	{
	public:
		MeshColliderComponent();

		/////Base/////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		//////////////

		static Ref<EntityComponent> Create() { return CreateRef<MeshColliderComponent>(); }
		static std::string GetFactoryName() { return "MeshColliderComponent"; }

		struct MeshColliderSpecification
		{
			Ref<Mesh> CollisionMesh;
			std::vector<Ref<Mesh>> ProcessedMeshes;
			bool IsConvex = false;
			bool IsTrigger = false;
			bool OverrideMesh = false;
			Ref<PhysicsMaterial> m_Material;
		};

		inline MeshColliderSpecification& GetSpecification() { return m_Specification; }

	private:
		MeshColliderSpecification m_Specification;
	};
}