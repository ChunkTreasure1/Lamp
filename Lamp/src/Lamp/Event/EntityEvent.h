#pragma once

#include "Event.h"
#include <sstream>

namespace Lamp
{
	class EntityCollisionEvent : public Event
	{
	public:
		EntityCollisionEvent(float overlap, IEntity* pEnt, const std::string& tag)
			: m_Overlap(overlap), m_pCollidedWith(pEnt), m_Tag(tag)
		{
		}

		inline const float GetOverlap() const { return m_Overlap; }
		inline IEntity* GetEntity() const { return m_pCollidedWith; }
		inline const std::string& GetTag() const { return m_Tag; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityCollisionEvent: " << m_Overlap;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntityEvent);
		EVENT_CLASS_TYPE(EntityCollisionEvent);
	private:
		float m_Overlap;
		IEntity* m_pCollidedWith;
		std::string m_Tag;
	};

	class EntityStoppedCollisionEvent : public Event
	{
	public:
		EntityStoppedCollisionEvent(float overlap, IEntity* pEnt, const std::string& tag)
			: m_pCollidedWith(pEnt), m_Tag(tag)
		{
		}

		inline IEntity* GetEntity() const { return m_pCollidedWith; }
		inline const std::string& GetTag() const { return m_Tag; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityCollisionEvent: " << m_Tag;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntityEvent);
		EVENT_CLASS_TYPE(EntityStoppedCollisionEvent);
	private:
		IEntity* m_pCollidedWith;
		std::string m_Tag;
	};
}