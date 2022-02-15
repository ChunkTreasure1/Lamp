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
		WindowClose = 1, WindowResize = 2, WindowFocus = 3, WindowLostFocus = 4, WindowMoved = 5,
		AppTick = 6, AppUpdate = 7, AppRender = 8, AppLog = 9, AppItemClicked = 10,
		KeyPressed = 11, KeyReleased = 12, KeyTyped = 13,
		MouseButtonPressed = 14, MouseButtonReleased = 15, MouseMoved = 16, MouseScrolled = 17,
		EntityCollision = 18, EntityStoppedCollision = 19, ObjectPositionChanged = 20, ImGuiBegin = 21, ObjectPropertyChanged = 22,
		EntityComponentAdded = 23, ImGuiUpdate = 24, EditorViewportSizeChanged = 25, EntityPhysicsUpdate = 26, EntityCollisionStart = 27,
		EntityCollisionEnd = 28, EntityTriggerBegin = 29, EntityTriggerEnd = 30, ObjectRotationChanged = 31, ObjectScaleChanged = 32, EditorObjectSelected = 33,
		LevelLoadStarted = 34, LevelLoadFinished = 35,
	};


	enum EventCategory
	{
		EventCategoryNone = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4),
		EventCategoryEntity = BIT(5),
		EventCategoryEditor = BIT(6)
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