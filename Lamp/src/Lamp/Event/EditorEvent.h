#pragma once

#include "Event.h"
#include <sstream>

namespace Lamp
{
	class EditorViewportSizeChangedEvent : public Event
	{
	public:
		EditorViewportSizeChangedEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Viewport size changed: " << std::to_string(m_Width) << std::to_string(m_Height);
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryEditor);
		EVENT_CLASS_TYPE(EditorViewportSizeChanged);

		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }

	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};
}