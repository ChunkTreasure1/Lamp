#include "lppch.h"
#include "PhysicsScene.h"

#include "ContactListener.h"
#include "PhysXInternal.h"
#include "Physics.h"

namespace Lamp
{
	static ContactListener s_ContactListener;

	PhysicsScene::PhysicsScene(const PhysicsSettings& settings)
		: m_SubStepSize(settings.FixedTimestep)
	{
		physx::PxSceneDesc sceneDesc(PhysXInternal::GetPhysXSDK().getTolerancesScale());
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
		sceneDesc.broadPhaseType = PhysXInternal::LampToPhysXBroadphaseType(settings.BroadphaseAlgorithm);
		sceneDesc.cpuDispatcher = PhysXInternal::GetCPUDispatcher();
		sceneDesc.filterShader = (physx::PxSimulationFilterShader)PhysXInternal::FilterShader;
		sceneDesc.simulationEventCallback = &s_ContactListener;
		sceneDesc.frictionType = PhysXInternal::LampToPhysXFrictionType(settings.FrictionModel);
	
		LP_CORE_ASSERT(sceneDesc.isValid(), "Physics scene not valid!");

		m_pPhysXScene = PhysXInternal::GetPhysXSDK().createScene(sceneDesc);
		LP_CORE_ASSERT(m_pPhysXScene, "PhysX scene not valid!");

		CreateRegions();
	}

	PhysicsScene::~PhysicsScene()
	{
	}

	void PhysicsScene::Simulate(float ts, bool callFixedUpdate)
	{
		if (callFixedUpdate)
		{
			for (auto& actor : m_Actors)
			{
				actor->OnFixedUpdate(m_SubStepSize);
			}
		}

		bool advanced = Advance(ts);
		if (advanced)
		{
			uint32_t activeActorCount;
			physx::PxActor** activeActors = m_pPhysXScene->getActiveActors(activeActorCount);
			for (uint32_t i = 0; i < activeActorCount; i++)
			{
				PhysicsActor* actor = (PhysicsActor*)(activeActors[i]->userData);
				actor->SynchronizeTransform();
			}
		}
	}

	Ref<PhysicsActor> PhysicsScene::GetActor(Entity* pEnt)
	{
		for (auto& actor : m_Actors)
		{
			if (actor->GetEntity() == pEnt)
			{
				return actor;
			}
		}

		return nullptr;
	}

	const Ref<PhysicsActor>& PhysicsScene::GetActor(Entity* pEnt) const
	{
		for (const auto& actor : m_Actors)
		{
			if (actor->GetEntity() == pEnt)
			{
				return actor;
			}
		}

		return nullptr;
	}

	Ref<PhysicsActor> PhysicsScene::CreateActor(Entity* pEnt)
	{
		Ref<PhysicsActor> actor = CreateRef<PhysicsActor>(pEnt);

		m_Actors.push_back(actor);
		m_pPhysXScene->addActor(*actor->m_pRigidActor);

		return actor;
	}

	void PhysicsScene::RemoveActor(Ref<PhysicsActor> actor)
	{
		if (!actor)
		{
			return;
		}

		for (auto& collider : actor->m_Colliders)
		{
			collider->DetachFromActor(actor->m_pRigidActor);
			collider->Release();
		}

		m_pPhysXScene->removeActor(*actor->m_pRigidActor);
		actor->m_pRigidActor->release();
		actor->m_pRigidActor = nullptr;

		for (auto it = m_Actors.begin(); it != m_Actors.end(); it++)
		{
			if ((*it)->GetEntity() == actor->GetEntity())
			{
				m_Actors.erase(it);
				break;
			}
		}
	}

	bool PhysicsScene::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit)
	{
		physx::PxRaycastBuffer hitInfo;
		bool result = m_pPhysXScene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo);

		if (result)
		{
			PhysicsActor* actor = (PhysicsActor*)hitInfo.block.actor->userData;
			outHit->HitEntity = actor->GetEntity()->GetID();
			outHit->Position = PhysXUtils::FromPhysXVector(hitInfo.block.position);
			outHit->Normal = PhysXUtils::FromPhysXVector(hitInfo.block.normal);
			outHit->Distance = hitInfo.block.distance;
		}

		return result;
	}

	bool PhysicsScene::OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxBoxGeometry(halfSize.x, halfSize.y, halfSize.z), buffer, count);
	}

	bool PhysicsScene::OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxCapsuleGeometry(radius, halfHeight), buffer, count);
	}

	bool PhysicsScene::OverlapSphere(const glm::vec3& origin, float radius, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxSphereGeometry(radius), buffer, count);
	}

	void PhysicsScene::CreateRegions()
	{
		const PhysicsSettings& settings = Physics::GetSettings();

		if (settings.BroadphaseAlgorithm == BroadphaseType::AutomaticBoxPrune)
		{
			return;
		}

		physx::PxBounds3* regionBounds = new physx::PxBounds3[settings.WorldBoundsSubdivisions * settings.WorldBoundsSubdivisions];
		physx::PxBounds3 globalBounds(PhysXUtils::ToPhysXVector(settings.WorldBoundsMin), PhysXUtils::ToPhysXVector(settings.WorldBoundsMax));
		uint32_t regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, settings.WorldBoundsSubdivisions);

		for (uint32_t i = 0; i < regionCount; i++)
		{
			physx::PxBroadPhaseRegion region;
			region.bounds = regionBounds[i];
			m_pPhysXScene->addBroadPhaseRegion(region);
		}
	}

	bool PhysicsScene::Advance(float ts)
	{
		SubstepStrategy(ts);

		for (uint32_t i = 0; i < m_NumSubSteps; i++)
		{
			m_pPhysXScene->simulate(m_SubStepSize);
			m_pPhysXScene->fetchResults(true);
		}

		return m_NumSubSteps != 0;
	}

	void PhysicsScene::SubstepStrategy(float ts)
	{
		if (m_Accumulator > m_SubStepSize)
			m_Accumulator = 0.0f;

		m_Accumulator += ts;
		if (m_Accumulator < m_SubStepSize)
		{
			m_NumSubSteps = 0;
			return;
		}

		m_NumSubSteps = glm::min(static_cast<uint32_t>(m_Accumulator / m_SubStepSize), m_MaxSubSteps);
		m_Accumulator -= (float)m_NumSubSteps * m_SubStepSize;
	}

	void PhysicsScene::Destroy()
	{
		LP_CORE_ASSERT(m_pPhysXScene, "Trying to destroy invalid physics scene!");

		for (auto& actor : m_Actors)
			RemoveActor(actor);

		m_Actors.clear();
		m_pPhysXScene->release();
		m_pPhysXScene = nullptr;
	}

	bool PhysicsScene::OverlapGeometry(const glm::vec3& origin, const physx::PxGeometry& geometry, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		physx::PxOverlapBuffer buf(buffer.data(), OVERLAP_MAX_COLLIDERS);
		physx::PxTransform pose = PhysXUtils::ToPhysXTransform(glm::translate(glm::mat4(1.0f), origin));

		bool result = m_pPhysXScene->overlap(geometry, pose, buf);
		if (result)
		{
			memcpy(buffer.data(), buf.touches, buf.nbTouches * sizeof(physx::PxOverlapHit));
			count = buf.nbTouches;
		}

		return result;
	}
}