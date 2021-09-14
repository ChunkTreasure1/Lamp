#include "lppch.h"
#include "ContactListener.h"

#include "PhysicsActor.h"
#include "Lamp/Level/Level.h"
#include "Lamp/Event/EntityEvent.h"

namespace Lamp
{
	void ContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		PX_UNUSED(constraints);
		PX_UNUSED(count);
	}

	void ContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxActor& physxActor = *actors[i];
			PhysicsActor* actor = (PhysicsActor*)physxActor.userData;
			LP_CORE_INFO("PhysX Actor waking up: Name: {0}", actor->GetEntity()->GetName());
		}
	}

	void ContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxActor& physxActor = *actors[i];
			PhysicsActor* actor = (PhysicsActor*)physxActor.userData;
			LP_CORE_INFO("PhysX Actor going to sleep: Name: {0}", actor->GetEntity()->GetName());
		}
	}

	void ContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		if (!g_pEnv->pLevel->GetIsPlaying())
		{
			return;
		}

		auto removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
		auto removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

		if (removedActorA || removedActorB)
			return;

		PhysicsActor* actorA = (PhysicsActor*)pairHeader.actors[0]->userData;
		PhysicsActor* actorB = (PhysicsActor*)pairHeader.actors[1]->userData;


		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			EntityCollisionStartEvent first(actorB->GetEntity());
			actorA->GetEntity()->OnEvent(first);

			EntityCollisionStartEvent second(actorA->GetEntity());
			actorB->GetEntity()->OnEvent(second);
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			EntityCollisionEndEvent first(actorB->GetEntity());
			actorA->GetEntity()->OnEvent(first);

			EntityCollisionEndEvent second(actorA->GetEntity());
			actorB->GetEntity()->OnEvent(second);
		}
	}

	void ContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		if (g_pEnv->pLevel->GetIsPlaying())
			return;

		for (uint32_t i = 0; i < count; i++)
		{
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			PhysicsActor* triggerActor = (PhysicsActor*)pairs[i].triggerActor->userData;
			PhysicsActor* otherActor = (PhysicsActor*)pairs[i].otherActor->userData;

			if (!triggerActor || !otherActor)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				EntityCollisionStartEvent first(otherActor->GetEntity());
				triggerActor->GetEntity()->OnEvent(first);

				EntityCollisionStartEvent second(triggerActor->GetEntity());
				otherActor->GetEntity()->OnEvent(first);
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				EntityCollisionEndEvent first(otherActor->GetEntity());
				triggerActor->GetEntity()->OnEvent(first);

				EntityCollisionEndEvent second(triggerActor->GetEntity());
				otherActor->GetEntity()->OnEvent(first);
			}
		}
	}

	void ContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
		PX_UNUSED(bodyBuffer);
		PX_UNUSED(poseBuffer);
		PX_UNUSED(count);
	}
}