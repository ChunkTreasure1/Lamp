#pragma once

#include "Event.h"
#include <sstream>

namespace Lamp
{
	class Entity;

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

	class EntityPropertyChangedEvent : public Event
	{
	public:
		EntityPropertyChangedEvent()
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityPropertyChanged";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityPropertyChanged);
	};

	class EntityComponentAddedEvent : public Event
	{
	public:
		EntityComponentAddedEvent()
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityComponentAdded";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityComponentAdded);
	};

	class EntityPhysicsUpdateEvent : public Event 
	{
	public:
		EntityPhysicsUpdateEvent(float ts)
			: m_Timestep(ts)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityPhysicsUpdate";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityPhysicsUpdate);

		inline float GetTimestep() { return m_Timestep; }

	private:
		float m_Timestep;
	};

	class EntityCollisionStartEvent : public Event
	{
	public:
		EntityCollisionStartEvent(Entity* pOther)
			: m_pOther(pOther)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityCollisionStart";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityCollisionStart);

		inline Entity* GetOther() { return m_pOther; }

	private:
		Entity* m_pOther;
	};

	class EntityCollisionEndEvent : public Event
	{
	public:
		EntityCollisionEndEvent(Entity* pOther)
			: m_pOther(pOther)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityCollisionEnd";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityCollisionEnd);

		inline Entity* GetOther() { return m_pOther; }

	private:
		Entity* m_pOther;
	};

	class EntityTriggerBeginEvent : public Event
	{
	public:
		EntityTriggerBeginEvent(Entity* pOther)
			: m_pOther(pOther)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityTriggerBegin";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityTriggerBegin);

		inline Entity* GetOther() { return m_pOther; }

	private:
		Entity* m_pOther;
	};

	class EntityTriggerEndEvent : public Event
	{
	public:
		EntityTriggerEndEvent(Entity* pOther)
			: m_pOther(pOther)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EntityTriggerBegin";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
		EVENT_CLASS_TYPE(EntityTriggerEnd);

		inline Entity* GetOther() { return m_pOther; }

	private:
		Entity* m_pOther;
	};
}