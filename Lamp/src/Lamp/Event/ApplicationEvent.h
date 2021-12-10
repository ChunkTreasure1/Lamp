#pragma once
#include "Event.h"
#include <sstream>

#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Input/FileSystem.h"

#include "Lamp/Rendering/RenderPass.h"

namespace Lamp
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height)
		{}
			//Getting
			inline const uint32_t GetWidth() const { return m_Width; }
			inline const uint32_t GetHeight() const { return m_Height; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "WindowResizeEvent: " << m_Width << ", " << m_Height << std::endl;
				return ss.str();
			}

			EVENT_CLASS_TYPE(WindowResize);
			EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		EVENT_CLASS_TYPE(AppTick);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent(Timestep ts)
			: m_Timestep(ts)
		{}

		inline const Timestep& GetTimestep() { return m_Timestep; }

		EVENT_CLASS_TYPE(AppUpdate);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		Timestep m_Timestep;
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent(const Ref<CameraBase> camera) 
			: m_Camera(camera)
		{}

		inline const Ref<CameraBase>& GetCamera() { return m_Camera; }

		EVENT_CLASS_TYPE(AppRender);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		Ref<CameraBase> m_Camera;
	};

	class AppLogEvent : public Event
	{
	public:
		AppLogEvent(const std::string& message, const std::string& severity)
			: m_Message(message), m_Severity(severity)
		{}

		inline const std::string& GetMessage() { return m_Message; }
		inline const std::string& GetSeverity() { return m_Severity; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppLogEvent: " << m_Message << ", " << m_Severity << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppLog);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		std::string m_Message;
		std::string m_Severity;
	};

	class ImGuiBeginEvent : public Event
	{
	public:
		ImGuiBeginEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ImGui Begin Frame" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(ImGuiBegin);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class ImGuiUpdateEvent : public Event 
	{
	public:
		ImGuiUpdateEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ImGui Update" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(ImGuiUpdate);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};
}