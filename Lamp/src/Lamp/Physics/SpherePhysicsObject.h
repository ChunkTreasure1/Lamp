#pragma once

#include "PhysicsObject.h"

namespace Lamp
{
	class SpherePhysicsObject : public PhysicsObject
	{
	public:
		SpherePhysicsObject(float radius)
			: m_Radius(radius), m_RadiusSqr(radius * radius)
		{}

		//Getting
		inline const float GetRadius() { return m_Radius; }
		inline const float GetRadiusSqr() { return m_RadiusSqr; }

	private:
		float m_Radius;
		float m_RadiusSqr;
	};
}