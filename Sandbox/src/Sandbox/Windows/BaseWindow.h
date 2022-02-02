#pragma once

#include <string_view>

#include "Sandbox/Sandbox.h"

namespace Sandbox
{
	class BaseWindow
	{
	public:
		using RenderFunc = std::function<void()>;

		BaseWindow(std::string_view name)
			: m_name(name), m_IsOpen(false), m_deltaTime(0.f)
		{}
		virtual ~BaseWindow() = default;

		virtual void OnEvent(Lamp::Event& e) = 0;

		inline void SetIsOpen(bool v) { m_IsOpen = v; }
		inline bool& GetIsOpen() { return m_IsOpen; }
		inline const std::string& GetLabel() { return m_name; }
		inline std::vector<RenderFunc>& GetRenderFuncs() { return m_renderFuncs; }
		inline const float GetDeltaTime() { return m_deltaTime; }

		const std::string GetName()
		{
			std::string name = m_name;
			name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

			return name;
		}

	protected:
		std::string m_name;
		bool m_IsOpen;
		bool m_isFocused;
		float m_deltaTime;

		ActionHandler m_ActionHandler;

		std::vector<RenderFunc> m_renderFuncs;
	};
}