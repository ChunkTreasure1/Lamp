#pragma once

#include "PhysicalEntity.h"

namespace Lamp
{
	class SpherePhysicalEntity : public PhysicalEntity
	{
	public:
		SpherePhysicalEntity(float radius) 
			: m_Radius(radius), m_RadiusSqr(radius * radius)
		{}
		virtual ~SpherePhysicalEntity() override {}
	
		virtual bool Intersect(const Ray& ray) const;

		//Getting
		inline const float GetRadius() { return m_Radius; }
		inline const float GetRadiusSqr() { return m_RadiusSqr; }

	private:
		float m_Radius;
		float m_RadiusSqr;
	};
}