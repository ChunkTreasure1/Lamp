#pragma once

#include "PhysXUtils.h"
#include <PhysX/PxPhysicsAPI.h>

#include "Lamp/Objects/Entity/BaseComponents/Physics/MeshColliderComponent.h"

namespace Lamp
{
	struct MeshColliderData
	{
		uint8_t* Data;
		glm::mat4 Transform;
		uint32_t Size;
	};

	class CookingFactory
	{
	public:
		static void Initialize();
		static void Shutdown();

		static CookingResult CookMesh(Ref<MeshColliderComponent> component, bool invalidateOld = false, std::vector<MeshColliderData>& outData = std::vector<MeshColliderData>());

		static CookingResult CookConvexMesh(const Ref<Mesh>& mesh, std::vector<MeshColliderData>& outData);
		static CookingResult CookTriangleMesh(const Ref<Mesh>& mesh, std::vector<MeshColliderData>& outData);

	private:
		static void GenerateDebugMesh(Ref<MeshColliderComponent> component, const MeshColliderData& colliderData);
	};
}