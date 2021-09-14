#pragma once

#include "PhysXUtils.h"
#include "PhysicsMaterial.h"

#include "Lamp/Objects/Entity/BaseComponents/Physics/BoxColliderComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/CapsuleColliderComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/SphereColliderComponent.h"

namespace Lamp
{
	enum class ColliderType
	{
		Box, Sphere, Capsule, ConvexMesh, TriangleMesh
	};

	class ColliderShape
	{
	protected:
		ColliderShape(ColliderType type)
			: m_Type(type), m_Material(nullptr) {}

	public:
		virtual ~ColliderShape() {}

		void Release()
		{
			m_Material->release();
		}

		void SetMaterial(const Ref<PhysicsMaterial>& material);

		virtual const glm::vec3& GetOffset() const = 0;
		virtual void SetOffset(const glm::vec3& offset) = 0;

		virtual bool IsTrigger() const = 0;
		virtual void SetTrigger(bool isTrigger) = 0;

		virtual void SetFilterData(const physx::PxFilterData& filterData) = 0;

		virtual void DetachFromActor(physx::PxRigidActor* actor) = 0;

		physx::PxMaterial& GetMaterial() const { return *m_Material; }

		bool IsValid() const { return m_Material != nullptr; }

	protected:
		ColliderType m_Type;

		physx::PxMaterial* m_Material;
	};

	class PhysicsActor;

	class BoxColliderShape : public ColliderShape
	{
	public:
		BoxColliderShape(Ref<BoxColliderComponent> component, const PhysicsActor& actor, Entity* entity, const glm::vec3& offset = glm::vec3(0.0f));
		~BoxColliderShape();

		const glm::vec3& GetOffset() const override { return m_Component->GetSpecification().m_Offset; }
		void SetOffset(const glm::vec3& offset) override;

		virtual bool IsTrigger() const override { return m_Component->GetSpecification().m_IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		Ref<BoxColliderComponent> m_Component;
		physx::PxShape* m_Shape;
	};

	class SphereColliderShape : public ColliderShape
	{
	public:
		SphereColliderShape(Ref<SphereColliderComponent> component, const PhysicsActor& actor, Entity* entity, const glm::vec3& offset = glm::vec3(0.0f));
		~SphereColliderShape();

		const glm::vec3& GetOffset() const override { return m_Component->GetSpecification().m_Offset; }
		void SetOffset(const glm::vec3& offset) override;

		virtual bool IsTrigger() const override { return m_Component->GetSpecification().m_IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		Ref<SphereColliderComponent> m_Component;
		physx::PxShape* m_Shape;
	};

	class CapsuleColliderShape : public ColliderShape
	{
	public:
		CapsuleColliderShape(Ref<CapsuleColliderComponent> component, const PhysicsActor& actor, Entity* entity, const glm::vec3& offset = glm::vec3(0.0f));
		~CapsuleColliderShape();

		const glm::vec3& GetOffset() const override { return m_Component->GetSpecification().m_Offset; }
		void SetOffset(const glm::vec3& offset) override;

		virtual bool IsTrigger() const override { return m_Component->GetSpecification().m_IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	private:
		Ref<CapsuleColliderComponent> m_Component;
		physx::PxShape* m_Shape;
	};

	//class ConvexMeshShape : public ColliderShape
	//{
	//public:
	//	ConvexMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity, const glm::vec3& offset = glm::vec3(0.0f));
	//	~ConvexMeshShape();

	//	const glm::vec3& GetOffset() const override { return glm::vec3(0.0f); }
	//	void SetOffset(const glm::vec3& offset) override {}

	//	virtual bool IsTrigger() const override { return m_Component.IsTrigger; }
	//	virtual void SetTrigger(bool isTrigger) override;

	//	virtual void SetFilterData(const physx::PxFilterData& filterData) override;

	//	virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	//private:
	//	MeshColliderComponent& m_Component;
	//	std::vector<physx::PxShape*> m_Shapes;
	//};

	//class TriangleMeshShape : public ColliderShape
	//{
	//public:
	//	TriangleMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity, const glm::vec3& offset = glm::vec3(0.0f));
	//	~TriangleMeshShape();

	//	const glm::vec3& GetOffset() const override { return glm::vec3(0.0f); }
	//	void SetOffset(const glm::vec3& offset) override {}

	//	virtual bool IsTrigger() const override { return m_Component.IsTrigger; }
	//	virtual void SetTrigger(bool isTrigger) override;

	//	virtual void SetFilterData(const physx::PxFilterData& filterData) override;

	//	virtual void DetachFromActor(physx::PxRigidActor* actor) override;

	//private:
	//	MeshColliderComponent& m_Component;
	//	std::vector<physx::PxShape*> m_Shapes;
	//};
}