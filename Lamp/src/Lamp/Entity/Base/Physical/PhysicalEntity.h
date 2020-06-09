#pragma once

#include "Lamp/Physics/Ray.h"

#include "Lamp/Physics/Colliders/Collider.h"
#include "Lamp/Physics/Colliders/BoundingSphere.h"

namespace Lamp
{
	class Brush;
	class Entity;

	class PhysicalEntity
	{
	public:
		PhysicalEntity()
			: m_IsPhysicalized(false), m_LastPosition({0.f, 0.f, 0.f}),
			m_Mass(0.f), m_Velocity(0.f), m_Collider(nullptr)
		{
		}
		~PhysicalEntity() {}

		void Integrate(float delta);

		//Getting
		inline const glm::vec3& GetVelocity() const { return m_Velocity; }
		inline Ref<Collider>& GetCollider() { return m_Collider; }
		inline const glm::vec3& GetVelocity() { return m_Velocity; }
		inline const float GetMass() { return m_Mass; }

		//Setting
		inline void SetVelocity(const glm::vec3& vel) { m_Velocity = vel; }
		inline void SetCollider(Ref<Collider> coll) { m_Collider = coll; }
		inline void SetMass(float mass) { m_Mass = mass; }

	protected:
		Ref<Collider> m_Collider;

		bool m_IsPhysicalized;
		float m_Mass;
		glm::vec3 m_Velocity;
		glm::vec3 m_LastPosition;
	};
}