#pragma once

#include "Event.h"
#include <sstream>

namespace Lamp
{
	class EntityCollisionEvent : public Event
	{
	public:
		EntityCollisionEvent(float overlap, Entity* pEnt, const std::string& tag)
			: m_Overlap(overlap), m_pCollidedWith(pEnt), m_Tag(tag)
		{
		}

		inline const float GetOverlap() const { return m_Overlap; }
		inline Entity* GetEntity() const { return m_pCollidedWith; }
		inline const std::string& GetTag() const { return m_Tag; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityCollisionEvent: " << m_Overlap;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityCollision);
	private:
		float m_Overlap;
		Entity* m_pCollidedWith;
		std::string m_Tag;
	};

	class EntityStoppedCollisionEvent : public Event
	{
	public:
		EntityStoppedCollisionEvent(float overlap, Entity* pEnt, const std::string& tag)
			: m_pCollidedWith(pEnt), m_Tag(tag)
		{
		}

		inline Entity* GetEntity() const { return m_pCollidedWith; }
		inline const std::string& GetTag() const { return m_Tag; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityCollisionEvent: " << m_Tag;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityStoppedCollision);
	private:
		Entity* m_pCollidedWith;
		std::string m_Tag;
	};

	class EntityPositionChangedEvent : public Event
	{
	public:
		EntityPositionChangedEvent()
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityPositionChanged";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityPositionChanged);
	};
}