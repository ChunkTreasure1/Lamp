#pragma once
#include "lppch.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	// Events in Lamp are currently blocking, meaning when an event occurs it
	// immediately gets dispatched and must be dealt with right then an there.
	// For the future, a better strategy might be to buffer events in an event
	// bus and process them during the "event" part of the update stage.


	enum EventType : uint64_t
	{
		None = 0,
		WindowClose = 0b00000001, WindowResize = 0b00000010, WindowFocus = 0b00000011, WindowLostFocus = 0b00000100, WindowMoved = 0b00000101,
		AppTick = 0b00000110, AppUpdate = 0b00000111, AppRender = 0b00001000, AppLog = 0b00001001, AppItemClicked = 0b00001010,
		KeyPressed = 0b00001100, KeyReleased = 0b00001101, KeyTyped = 0b00001110,
		MouseButtonPressed = 0b00001111, MouseButtonReleased = 0b00010000, MouseMoved = 0b00010001, MouseScrolled = 0b00010010,
		EntityCollision = 0b00010011, EntityStoppedCollision = 0b00010100, EntityPositionChanged = 0b00010101, ImGuiBegin = 0b0010110, EntityPropertyChanged = 0b0011000,
		EntityComponentAdded = 0b0011001,

		All = WindowClose | WindowResize | WindowFocus | WindowLostFocus | WindowMoved |
		AppTick | AppUpdate | AppRender | AppLog | AppItemClicked |
		KeyPressed | KeyReleased | KeyTyped |
		MouseButtonPressed | MouseButtonReleased | MouseMoved | MouseScrolled |
		EntityCollision | EntityStoppedCollision | EntityPositionChanged | ImGuiBegin | EntityPropertyChanged | EntityComponentAdded
	};


	enum EventCategory
	{
		EventCategoryNone = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4),
		EventCategoryEntity = BIT(5)

	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event
	{
	public:
		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; }
	};

	class EventDispatcher
	{

	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{}

		// F will be deduced by the compiler
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator <<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}