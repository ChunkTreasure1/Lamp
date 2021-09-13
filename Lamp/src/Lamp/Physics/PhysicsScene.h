#pragma once

#include "PhysicsSettings.h"
#include "PhysicsActor.h"

#define OVERLAP_MAX_COLLIDERS 10

namespace Lamp
{
	struct RaycastHit
	{
		uint64_t HitEntity;
		glm::vec3 Position;
		glm::vec3 Normal;
		float Distance;
	};

	class PhysicsScene
	{
	public:
		PhysicsScene(const PhysicsSettings& settings);
		~PhysicsScene();

		void Simulate(float ts, bool callFixedUpdate = true);

		Ref<PhysicsActor> GetActor(Entity* pEnt);
		const Ref<PhysicsActor>& GetActor(Entity* pEnt) const;

		Ref<PhysicsActor> CreateActor(Entity* pEnt);
		void RemoveActor(Ref<PhysicsActor> actor);

		glm::vec3 GetGravity() const { return PhysXUtils::FromPhysXVector(m_pPhysXScene->getGravity()); }
		void SetGravity(const glm::vec3& gravity) { m_pPhysXScene->setGravity(PhysXUtils::ToPhysXVector(gravity)); }

		bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit);
		bool OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);
		bool OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);
		bool OverlapSphere(const glm::vec3& origin, float radius, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);

		bool IsValid() const { return m_pPhysXScene != nullptr; }

	private:
		void CreateRegions();

		bool Advance(float ts);
		void SubstepStrategy(float ts);

		void Destroy();

		bool OverlapGeometry(const glm::vec3& origin, const physx::PxGeometry& geometry, std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);

	private:
		physx::PxScene* m_pPhysXScene;
		std::vector<Ref<PhysicsActor>> m_Actors;

		float m_SubStepSize;
		float m_Accumulator = 0.f;
		uint32_t m_NumSubSteps = 0;
		const uint32_t m_MaxSubSteps = 8;

	private:
		friend class Physics;
	};
}