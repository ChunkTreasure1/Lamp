#pragma once

#include "PhysXUtils.h"
#include <PhysX/PxPhysicsAPI.h>

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

		//static CookingResult CookMesh(Ref<MeshColl>)

	private:
	};
}