#pragma once

#include "Event.h"
#include <sstream>

namespace Lamp
{
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
}