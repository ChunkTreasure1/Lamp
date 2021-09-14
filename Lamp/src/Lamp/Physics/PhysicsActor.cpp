#include "lppch.h"
#include "PhysicsActor.h"

#include "PhysXInternal.h"
#include "Physics.h"

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
		if (!IsKinematic())
		{
			LP_CORE_WARN("Trying to set kinematic target for a non-kinematic actor.");
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");
		physx::PxTransform target;
		actor->getKinematicTarget(target);
		return PhysXUtils::FromPhysXVector(target.p);
	}

	glm::vec3 PhysicsActor::GetKinematicTargetRotation() const
	{
		if (!IsKinematic())
		{
			LP_CORE_WARN("Trying to set kinematic target for a non-kinematic actor.");
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");
		physx::PxTransform target;
		actor->getKinematicTarget(target);
		return glm::eulerAngles(PhysXUtils::FromPhysXQuat(target.q));
	}

	void PhysicsActor::SetKinematicTarget(const glm::vec3& targetPosition, const glm::vec3& targetRotation) const
	{
		if (!IsKinematic())
		{
			LP_CORE_WARN("Trying to set kinematic target for a non-kinematic actor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_pRigidActor->is<physx::PxRigidDynamic>();
		LP_CORE_ASSERT(actor, "Actor is null!");
		actor->setKinematicTarget(PhysXUtils::ToPhysXTransform(targetPosition, targetRotation));
	}

	void PhysicsActor::SetSimulationData(uint32_t layerId)
	{
		//const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(layerId);
		//
		//if (layerInfo.CollidesWith == 0)
		//	return;
		//
		//physx::PxFilterData filterData;
		//filterData.word0 = layerInfo.BitValue;
		//filterData.word1 = layerInfo.CollidesWith;
		//filterData.word2 = (uint32_t)m_RigidBodyData.CollisionDetection;
		//
		//for (auto& collider : m_Colliders)
		//	collider->SetFilterData(filterData);
	}

	void PhysicsActor::SetKinematic(bool isKinematic)
	{
		if (!IsDynamic())
		{
			LP_CORE_WARN("Static PhysicsActor can't be kinematic.");
			return;
		}

		m_RigidbodyData->GetSpecification().IsKinematic = isKinematic;
		m_pRigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
	}

	void PhysicsActor::SetGravityDisabled(bool disable)
	{
		m_pRigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, disable);
	}

	void PhysicsActor::SetLockFlag(ActorLockFlag flag, bool value)
	{
		if (!IsDynamic())
			return;

		if (value)
			m_LockFlags |= (uint32_t)flag;
		else
			m_LockFlags &= ~(uint32_t)flag;

		m_pRigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags((physx::PxRigidDynamicLockFlags)m_LockFlags);
	}

	void PhysicsActor::OnFixedUpdate(float fixedDeltaTime)
	{
		EntityPhysicsUpdateEvent e(fixedDeltaTime);
		m_pEntity->OnEvent(e);
	}

	void PhysicsActor::AddCollider(Ref<BoxColliderComponent> collider, Entity* pEnt, const glm::vec3& offset)
	{
		m_Colliders.push_back(CreateRef<BoxColliderShape>(collider, *this, pEnt, offset));
	}

	void PhysicsActor::AddCollider(Ref<SphereColliderComponent> collider, Entity* pEnt, const glm::vec3& offset)
	{
		m_Colliders.push_back(CreateRef<SphereColliderShape>(collider, *this, pEnt, offset));
	}

	void PhysicsActor::AddCollider(Ref<CapsuleColliderComponent> collider, Entity* pEnt, const glm::vec3& offset)
	{
		m_Colliders.push_back(CreateRef<CapsuleColliderShape>(collider, *this, pEnt, offset));
	}

	void PhysicsActor::CreateRigidActor()
	{
		auto& sdk = PhysXInternal::GetPhysXSDK();
		
		glm::mat4 transform = m_pEntity->GetModelMatrix();

		if (m_RigidbodyData->GetSpecification().m_BodyType == RigidbodyComponent::Type::Static)
		{
			m_pRigidActor = sdk.createRigidStatic(PhysXUtils::ToPhysXTransform(transform));
		}
		else
		{
			const PhysicsSettings& settings = Physics::GetSettings();
			m_pRigidActor = sdk.createRigidDynamic(PhysXUtils::ToPhysXTransform(transform));

			SetLinearDrag(m_RigidbodyData->GetSpecification().m_LinearDrag);
			SetAngularDrag(m_RigidbodyData->GetSpecification().m_AngularDrag);

			SetKinematic(m_RigidbodyData->GetSpecification().IsKinematic);

			SetGravityDisabled(m_RigidbodyData->GetSpecification().m_DisableGravity);

			m_pRigidActor->is<physx::PxRigidDynamic>()->setSolverIterationCounts(settings.SolverIterations, settings.SolverVelocityIterations);
			m_pRigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, m_RigidbodyData->GetSpecification().m_CollisionDetection == RigidbodyComponent::CollisionDetectionType::Continuous);
			m_pRigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, m_RigidbodyData->GetSpecification().m_CollisionDetection == RigidbodyComponent::CollisionDetectionType::ContinuousSpeculative);
		}

		if (m_pEntity->HasComponent<BoxColliderComponent>()) AddCollider(m_pEntity->GetComponent<BoxColliderComponent>(), m_pEntity);
		if (m_pEntity->HasComponent<SphereColliderComponent>()) AddCollider(m_pEntity->GetComponent<SphereColliderComponent>(), m_pEntity);
		if (m_pEntity->HasComponent<CapsuleColliderComponent>()) AddCollider(m_pEntity->GetComponent<CapsuleColliderComponent>(), m_pEntity);
	
		SetMass(m_RigidbodyData->GetSpecification().m_Mass);
		m_pRigidActor->userData = this;
	}

	void PhysicsActor::SynchronizeTransform()
	{
		physx::PxTransform actorPose = m_pRigidActor->getGlobalPose();
		m_pEntity->SetPosition(PhysXUtils::FromPhysXVector(actorPose.p));
		m_pEntity->SetRotation(glm::eulerAngles(PhysXUtils::FromPhysXQuat(actorPose.q)));
	}
}