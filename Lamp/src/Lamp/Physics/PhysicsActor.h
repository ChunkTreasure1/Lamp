#pragma once

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "PhysXUtils.h"
#include "Lamp/Objects/Entity/BaseComponents/RigidbodyComponent.h"

namespace Lamp
{
	class PhysicsActor
	{
	public:
		PhysicsActor(Entity* pEnt);
		~PhysicsActor();

		glm::vec3 GetTranslation() const {}

	private:
		Entity* m_pEntity;
		Ref<RigidbodyComponent> m_RigidbodyData;

		uint32_t m_LockFlags = 0;

		physx::PxRigidActor* m_pRigidActor;

	private:
		friend class PhysicsScene;
	};
}