#pragma once

#include <btBulletDynamicsCommon.h>

namespace Lamp
{
	class Rigidbody
	{
	public:
		Rigidbody();
		~Rigidbody();

		void SetCollisionShape(btCollisionShape* shape, void* pUser);
		void SetMass(float mass);
		void SetStatic(bool b);
		void SetDamping(float lDamp, float angDamp);
		void SetScale(const glm::vec3& scale);

		void SetPosition(const glm::vec3& pos);
		void SetRotation(const glm::vec3& rot);
		inline void SetRigidBody(btRigidBody* body) { m_pBody = body; }

		inline const glm::vec3& GetVelocity() { btVector3 v = m_pBody->getLinearVelocity(); return glm::vec3(v.getX(), v.getY(), v.getZ()); }
		inline void SetVelocity(const glm::vec3& vel) { m_pBody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z)); }
		inline void Move(const glm::vec3& vel) { glm::vec3 v = vel; v.y = GetVelocity().y; SetVelocity(v); m_pBody->activate(); }

		inline bool GetIsStatic() { return m_IsStatic; }
		inline void AddForce(const glm::vec3& force, const glm::vec3& point) { m_pBody->applyForce(btVector3(force.x, force.y, force.z), btVector3(point.x, point.y, point.z)); }
		inline void AddCentralForce(const glm::vec3& force) { m_pBody->setLinearVelocity(m_pBody->getLinearVelocity() + btVector3(force.x, force.y, force.z)); };

		inline btCollisionShape* GetCollisionShape() { return m_pCollisionShape; }
		inline btRigidBody* GetRigidbody() { return m_pBody; }
		inline btTransform& GetTransform() { return m_pBody->getWorldTransform(); }
		inline btMotionState* GetMotionState() { return m_pBody->getMotionState(); }

	private:
		btCollisionShape* m_pCollisionShape;
		btRigidBody* m_pBody;
		glm::vec3 m_Inertia = glm::vec3(0.f, 0.f, 0.f);
		
		bool m_IsStatic = true;
		float m_Mass = 1.f;
		float m_LinearDamping = 0.f;
		float m_AngularDamping = 0.f;
	};
}