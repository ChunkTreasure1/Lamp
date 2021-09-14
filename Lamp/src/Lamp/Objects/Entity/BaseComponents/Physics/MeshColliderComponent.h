#pragma once

#include "Lamp/Physics/PhysicsMaterial.h"
#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Meshes/Mesh.h"

namespace Lamp
{
	class MeshColliderComponent final : public EntityComponent
	{
	public:
		MeshColliderComponent()
			: EntityComponent("MeshColliderComponent")
		{
			SetComponentProperties
			({
				{ PropertyType::Bool, "Is Trigger", RegisterData(&m_Specification.IsTrigger) }
			});
		}

		/////Base/////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint32_t GetSize() { return sizeof(*this); }
		virtual uint64_t GetEventMask() override { return EventType::None; }
		//////////////

	public:
		static Ref<EntityComponent> Create() { return CreateRef<MeshColliderComponent>(); }
		static std::string GetFactoryName() { return "MeshColliderComponent"; }

	public:
		struct MeshColliderSpecification
		{
			Ref<Mesh> CollisionMesh;
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