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
			: m_name(name), m_IsOpen(false)
		{}

		virtual void OnEvent(Lamp::Event& e) = 0;

		inline void SetIsOpen(bool v) { m_IsOpen = v; }
		inline bool& GetIsOpen() { return m_IsOpen; }
		inline const std::string& GetLabel() { return m_name; }
		inline std::vector<RenderFunc>& GetRenderFuncs() { return m_renderFuncs; }

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

		ActionHandler m_ActionHandler;

		std::vector<RenderFunc> m_renderFuncs;
	};
}