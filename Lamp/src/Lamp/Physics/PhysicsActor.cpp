#include "lppch.h"
#include "PhysicsActor.h"

namespace Lamp
{
	PhysicsActor::PhysicsActor(Entity* pEnt)
		: m_pEntity(pEnt), m_RigidbodyData(pEnt->GetComponent<RigidbodyComponent>()), m_pRigidActor(nullptr)
	{
		CreateRigidActor();
	}
	PhysicsActor::~PhysicsActor()
	{
		m_Colliders.clear();
	}

	void PhysicsActor::SetTranslation(const glm::vec3& trans, bool autoWake)
	{
		physx::PxTransform transform = m_pRigidActor->getGlobalPose();
		transform.p = PhysXUtils::ToPhysXVector(trans);
		m_pRigidActor->setGlobalPose(transform, autoWake);

		if (m_RigidbodyData->GetSpecification().m_BodyType == RigidbodyComponent::Type::Static)
		{
			SynchronizeTransform();
		}
	}

	void PhysicsActor::SetRotation(const glm::vec3& rot, bool autoWake)
	{
		physx::PxTransform transform = m_pRigidActor->getGlobalPose();
		transform.q = PhysXUtils::ToPhysXQuat(glm::quat(rot));
		m_pRigidActor->setGlobalPose(transform, autoWake);

		if (m_RigidbodyData->GetSpecification().m_BodyType == RigidbodyComponent::Type::Static)
		{
			SynchronizeTransform();
		}
	}

	void PhysicsActor::Rotate(const glm::vec3& rot, bool autoWake)
	{
		physx::PxTransform transform = m_pRigidActor->getGlobalPose();
		transform.q *=
			(physx::PxQuat(glm::radians(rot.x), { 1.f, 0.f, 0.f })
				* physx::PxQuat(glm::radians(rot.y), { 0.f, 1.f, 0.f })
				* physx::PxQuat(glm::radians(rot.z), { 0.f, 0.f, 1.f }));

		m_pRigidActor->setGlobalPose(transform, autoWake);

		if (m_RigidbodyData->GetSpecification().m_BodyType == RigidbodyComponent::Type::Static)
		{
			SynchronizeTransform();
		}
	}

	void PhysicsActor::WakeUp()
	{
		if (IsDynamic())
		{
			m_pRigidActor->is<physx::PxRigidDynamic>()->wakeUp();
		}
	}

	void PhysicsActor::PutToSleep()
	{
		if (IsDynamic())
		{
			m_pRigidActor->is<physx::PxRigidDynamic>()->putToSleep();
		}
	}

	float PhysicsActor::GetMass()
	{
		return !IsDynamic() ? m_RigidbodyData->GetSpecification().m_Mass : m_pRigidActor->is<physx::PxRigidDynamic>()->getMass();
	}
	void PhysicsActor::SetMass(float mass)
	{
		if (!IsDynamic())
		{
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass);
		m_RigidbodyData->GetSpecification().m_Mass = mass;
	}

	void PhysicsActor::AddForce(const glm::vec3& force, ForceMode forceMode)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to add force to non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");
		actor->addForce(PhysXUtils::ToPhysXVector(force), (physx::PxForceMode::Enum)forceMode);
	}

	void PhysicsActor::AddTorque(const glm::vec3& torque, ForceMode forceMode)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to add torque to non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");
		actor->addTorque(PhysXUtils::ToPhysXVector(torque), (physx::PxForceMode::Enum)forceMode);
	}

	glm::vec3 PhysicsActor::GetLinearVelocity() const
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to get velocity of non-dynamic PhysicsActor");
			return glm::vec3(0.f);
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		return PhysXUtils::FromPhysXVector(actor->getLinearVelocity());
	}

	void PhysicsActor::SetLinearVelocity(const glm::vec3& vel)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		actor->setLinearVelocity(PhysXUtils::ToPhysXVector(vel));
	}

	glm::vec3 PhysicsActor::GetAngularVelocity() const
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to get velocity of non-dynamic PhysicsActor");
			return glm::vec3(0.f);
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		return PhysXUtils::FromPhysXVector(actor->getAngularVelocity());
	}

	void PhysicsActor::SetAngularVelocity(const glm::vec3& vel)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		actor->setAngularVelocity(PhysXUtils::ToPhysXVector(vel));
	}

	float PhysicsActor::GetMaxLinearVelocity() const
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to get velocity of non-dynamic PhysicsActor");
			return 0.f;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		return actor->getMaxLinearVelocity();
	}

	void PhysicsActor::SetMaxLinearVelocity(float maxVel)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		actor->setMaxLinearVelocity(maxVel);
	}

	float PhysicsActor::GetMaxAngularVelocity() const
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to get velocity of non-dynamic PhysicsActor");
			return 0.f;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		return actor->getMaxAngularVelocity();
	}

	void PhysicsActor::SetMaxAngularVelocity(float maxVel)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to set velocity of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		actor->setMaxLinearVelocity(maxVel);
	}

	void PhysicsActor::SetLinearDrag(float drag) const
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to set drag of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		actor->setLinearDamping(drag);
	}

	void PhysicsActor::SetAngularDrag(float drag) const
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Trying to set drag of non-dynamic PhysicsActor");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");

		actor->setAngularDamping(drag);
	}

	glm::vec3 PhysicsActor::GetKinematicTargetPosition() const
	{
		return glm::vec3();
	}

	glm::vec3 PhysicsActor::GetKinematicTargetRotation() const
	{
		return glm::vec3();
	}

	void PhysicsActor::SetKinematicTarget(const glm::vec3& targetPosition, const glm::vec3& targetRotation) const
	{
	}

	void PhysicsActor::SetSimulationData(uint32_t layerId)
	{
	}

	void PhysicsActor::SetKinematic(bool isKinematic)
	{
	}

	void PhysicsActor::SetGravityDisabled(bool disable)
	{
	}

	void PhysicsActor::SetLockFlag(ActorLockFlag flag, bool value)
	{
	}

	void PhysicsActor::OnFixedUpdate(float fixedDeltaTime)
	{
	}
	void PhysicsActor::AddCollider(Ref<BoxColliderComponent> collider, Entity* pEnt, const glm::vec3& offset)
	{
	}

	void PhysicsActor::CreateRigidActor()
	{
	}

	void PhysicsActor::SynchronizeTransform()
	{
	}
}