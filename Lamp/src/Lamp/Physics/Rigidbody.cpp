#include "lppch.h"
#include "Rigidbody.h"

#include <glm/gtx/quaternion.hpp>

namespace Lamp
{
	Rigidbody::Rigidbody()
	{
	}

	Rigidbody::~Rigidbody()
	{
	}

	void Rigidbody::SetCollisionShape(btCollisionShape* shape, void* pUser)
	{
		m_pCollisionShape = shape;
		m_pBody->setCollisionShape(shape);

		m_pCollisionShape->setUserPointer(pUser);
	}

	void Rigidbody::SetMass(float mass)
	{
		m_Mass = mass;

		if (m_IsStatic)
		{
			return;
		}

		m_pBody->setMassProps(m_Mass, btVector3(m_Inertia.x, m_Inertia.y, m_Inertia.z));
	}

	void Rigidbody::SetStatic(bool b)
	{
		m_IsStatic = b;
		if (b)
		{
			m_pBody->setMassProps(0.f, btVector3(m_Inertia.x, m_Inertia.y, m_Inertia.z));
		}
		else
		{
			m_pBody->setMassProps(m_Mass, btVector3(m_Inertia.x, m_Inertia.y, m_Inertia.z));
		}
	}

	void Rigidbody::SetDamping(float lDamp, float angDamp)
	{
		m_LinearDamping = lDamp;
		m_AngularDamping = angDamp;

		m_pBody->setDamping(m_LinearDamping, m_AngularDamping);
	}

	void Rigidbody::SetScale(const glm::vec3& scale)
	{
		glm::vec3 s = scale;
		if (s.x < 0.1f) s.x = 0.1f;
		if (s.y < 0.1f) s.y = 0.1f;
		if (s.z < 0.1f) s.z = 0.1f;

		m_pCollisionShape->setLocalScaling(btVector3(s.x, s.y, s.z));
	}

	void Rigidbody::SetPosition(const glm::vec3& pos)
	{
		btTransform& tr = GetTransform();
		tr.setOrigin(btVector3(pos.x, pos.y, pos.z));
		GetMotionState()->setWorldTransform(tr);
	}

	void Rigidbody::SetRotation(const glm::vec3& rot)
	{
		btTransform& tr = GetTransform();
		glm::quat q(rot);

		tr.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
		GetMotionState()->setWorldTransform(tr);
	}
}