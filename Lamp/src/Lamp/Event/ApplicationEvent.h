#pragma once
#include "Event.h"
#include <sstream>

#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Rendering/Renderer3D.h"
#include "Lamp/Input/FileSystem.h"

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
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

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
		AppRenderEvent(const RenderPassInfo& passInfo) 
			: m_RenderPassInfo(passInfo)
		{}

		inline const RenderPassInfo& GetPassInfo() { return m_RenderPassInfo; }

		EVENT_CLASS_TYPE(AppRender);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);

	private:
		RenderPassInfo m_RenderPassInfo;
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

	class AppItemClickedEvent : public Event
	{
	public:
		AppItemClickedEvent(const File& file)
			: m_File(file)
		{}

		const File& GetFile() { return m_File; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "AppItemClickedEvent: " << m_File.GetPath() << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(AppItemClicked);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	private:
		File m_File;
	};

	class ImGuiBeginEvent : public Event
	{
	public:
		ImGuiBeginEvent()
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ImGui Begin Frame" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(ImGuiBegin);
		EVENT_CLASS_CATEGORY(EventCategoryApplication);
	};
}