#pragma once

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "PhysXUtils.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/RigidbodyComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/BoxColliderComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/SphereColliderComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/CapsuleColliderComponent.h"

#include "PhysicsShapes.h"

namespace Lamp
{
	class PhysicsActor
	{
	public:
		PhysicsActor(Entity* pEnt);
		~PhysicsActor();

		glm::vec3 GetTranslation() const { return PhysXUtils::FromPhysXVector(m_pRigidActor->getGlobalPose().p); }
		void SetTranslation(const glm::vec3& trans, bool autoWake = true);

		glm::vec3 GetRotation() const { return glm::eulerAngles(PhysXUtils::FromPhysXQuat(m_pRigidActor->getGlobalPose().q)); }
		void SetRotation(const glm::vec3& rot, bool autoWake = true);
		void Rotate(const glm::vec3& rot, bool autoWake = true);

		void WakeUp();
		void PutToSleep();

		float GetMass();
		void SetMass(float mass);

		void AddForce(const glm::vec3& force, ForceMode forceMode);
		void AddTorque(const glm::vec3& torque, ForceMode forceMode);

		glm::vec3 GetLinearVelocity() const;
		void SetLinearVelocity(const glm::vec3& vel);

		glm::vec3 GetAngularVelocity() const;
		void SetAngularVelocity(const glm::vec3& vel);

		float GetMaxLinearVelocity() const;
		void SetMaxLinearVelocity(float maxVel);		
		
		float GetMaxAngularVelocity() const;
		void SetMaxAngularVelocity(float maxVel);

		void SetLinearDrag(float drag) const;
		void SetAngularDrag(float drag) const;

		glm::vec3 GetKinematicTargetPosition() const;
		glm::vec3 GetKinematicTargetRotation() const;
		void SetKinematicTarget(const glm::vec3& targetPosition, const glm::vec3& targetRotation) const;

		void SetSimulationData(uint32_t layerId);

		bool IsDynamic() const { return m_RigidbodyData->GetSpecification().m_BodyType == RigidbodyComponent::Type::Dynamic; }

		bool IsKinematic() const { return IsDynamic() && m_RigidbodyData->GetSpecification().IsKinematic; }
		void SetKinematic(bool isKinematic);

		bool IsGravityDisabled() const { return m_pRigidActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY); }
		void SetGravityDisabled(bool disable);

		bool IsLockFlagSet(ActorLockFlag flag) const { return (uint32_t)flag & m_LockFlags; }
		void SetLockFlag(ActorLockFlag flag, bool value);
		uint32_t GetLockFlags() const { return m_LockFlags; }

		void OnFixedUpdate(float fixedDeltaTime);
		
		Entity* GetEntity() const { return m_pEntity; }
		const glm::vec3& GetPosition() { return m_pEntity->GetPosition(); }
		const glm::vec3& GetRotation() { return m_pEntity->GetRotation(); }
		const glm::vec3& GetScale() { return m_pEntity->GetScale(); }

		void AddCollider(Ref<BoxColliderComponent> collider, Entity* pEnt, const glm::vec3& offset = glm::vec3(0.f));
		void AddCollider(Ref<SphereColliderComponent> collider, Entity* pEnt, const glm::vec3& offset = glm::vec3(0.f));
		void AddCollider(Ref<CapsuleColliderComponent> collider, Entity* pEnt, const glm::vec3& offset = glm::vec3(0.f));

		physx::PxRigidActor& GetPhysXActor() const { return *m_pRigidActor; }

	private:
		void CreateRigidActor();
		void SynchronizeTransform();

	private:
		Entity* m_pEntity;
		Ref<RigidbodyComponent> m_RigidbodyData;

		uint32_t m_LockFlags = 0;

		physx::PxRigidActor* m_pRigidActor;
		std::vector<Ref<ColliderShape>> m_Colliders;

	private:
		friend class PhysicsScene;
	};
}