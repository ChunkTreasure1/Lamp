#pragma once

#include "Lamp/Physics/Ray.h"

namespace Lamp
{
	class IEntity;
	class Brush;

	class PhysicalEntity
	{
	public:
		PhysicalEntity()
			: m_pEntity(nullptr), m_pBrush(nullptr)
		{}
		virtual ~PhysicalEntity() {}

		virtual bool Intersect(const Ray& ray) const = 0;

		//Getting
		inline IEntity* GetEntity() { return m_pEntity; }
		inline Brush* GetBrush() { return m_pBrush; }

		//Setting
		inline void SetEntity(IEntity* entity) { m_pEntity = entity; }
		inline void SetBrush(Brush* brush) { m_pBrush = brush; }

	protected:
		IEntity* m_pEntity;
		Brush* m_pBrush;
	};
}