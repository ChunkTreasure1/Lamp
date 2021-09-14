#include "lppch.h"
#include "PhysicsShapes.h"

#include "PhysXInternal.h"
#include "PhysicsActor.h"

namespace Lamp
{
	void ColliderShape::SetMaterial(const Ref<PhysicsMaterial>& material)
	{
		Ref<PhysicsMaterial> mat = material;
		if (!mat)
		{
			mat = CreateRef<PhysicsMaterial>(0.6f, 0.6f, 0.f);
		}

		if (m_Material != nullptr)
		{
			m_Material->release();
		}

		m_Material = PhysXInternal::GetPhysXSDK().createMaterial(mat->StaticFriction, mat->DynamicFriction, mat->Bounciness);
	}

	BoxColliderShape::BoxColliderShape(Ref<BoxColliderComponent> component, const PhysicsActor& actor, Entity* entity, const glm::vec3& offset)
		: ColliderShape(ColliderType::Box), m_Component(component)
	{
		SetMaterial(m_Component->GetSpecification().m_Material);

		glm::vec3 colliderSize = entity->GetScale() * m_Component->GetSpecification().m_Size;
		physx::PxBoxGeometry geo = physx::PxBoxGeometry(colliderSize.x / 2.f, colliderSize.y / 2.f, colliderSize.z / 2.f);

		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetPhysXActor(), geo, *m_Material);
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !m_Component->GetSpecification().m_IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, m_Component->GetSpecification().m_IsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset + m_Component->GetSpecification().m_Offset, glm::vec3(0.f)));
	}

	BoxColliderShape::~BoxColliderShape()
	{}

	void BoxColliderShape::SetOffset(const glm::vec3& offset)
	{
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset, glm::vec3(0.f)));
		m_Component->GetSpecification().m_Offset = offset;
	}

	void BoxColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		m_Component->GetSpecification().m_IsTrigger;
	}

	void BoxColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void BoxColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

	SphereColliderShape::SphereColliderShape(Ref<SphereColliderComponent> component, const PhysicsActor& actor, Entity* entity, const glm::vec3& offset)
		: ColliderShape(ColliderType::Sphere), m_Component(component)  
	{
		SetMaterial(m_Component->GetSpecification().m_Material);
		auto& scale = entity->GetScale();
		float largestComp = glm::max(scale.x, glm::max(scale.y, scale.z));

		physx::PxSphereGeometry geo = physx::PxSphereGeometry(largestComp * m_Component->GetSpecification().m_Radius);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetPhysXActor(), geo, *m_Material);
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !m_Component->GetSpecification().m_IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, m_Component->GetSpecification().m_IsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset + m_Component->GetSpecification().m_Offset, glm::vec3(0.f)));
	}

	SphereColliderShape::~SphereColliderShape()
	{
	}

	void SphereColliderShape::SetOffset(const glm::vec3& offset)
	{
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset, glm::vec3(0.f)));
		m_Component->GetSpecification().m_Offset = offset;
	}

	void SphereColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		m_Component->GetSpecification().m_IsTrigger;
	}

	void SphereColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void SphereColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}

	CapsuleColliderShape::CapsuleColliderShape(Ref<CapsuleColliderComponent> component, const PhysicsActor& actor, Entity* entity, const glm::vec3& offset)
		: ColliderShape(ColliderType::Capsule), m_Component(component)
	{
		SetMaterial(m_Component->GetSpecification().m_Material);
		auto& scale = entity->GetScale();
		float radiusScale = glm::max(scale.x, scale.z);

		physx::PxCapsuleGeometry geo = physx::PxCapsuleGeometry(m_Component->GetSpecification().m_Radius * radiusScale, (m_Component->GetSpecification().m_Height / 2.f) * scale.y);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetPhysXActor(), geo, *m_Material);
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !m_Component->GetSpecification().m_IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, m_Component->GetSpecification().m_IsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset + m_Component->GetSpecification().m_Offset, glm::vec3(0.f, 0.f, physx::PxHalfPi)));
	}

	CapsuleColliderShape::~CapsuleColliderShape()
	{
	}

	void CapsuleColliderShape::SetOffset(const glm::vec3& offset)
	{
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset, glm::vec3(0.0f)));
		m_Component->GetSpecification().m_Offset = offset;
	}

	void CapsuleColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		m_Component->GetSpecification().m_IsTrigger = isTrigger;
	}

	void CapsuleColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void CapsuleColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		actor->detachShape(*m_Shape);
	}
}