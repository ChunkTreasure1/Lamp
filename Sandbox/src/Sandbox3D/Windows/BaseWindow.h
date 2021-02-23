#pragma once

#include <string_view>

#include "Sandbox3D/Sandbox3D.h"

namespace Sandbox3D
{
	class BaseWindow
	{
	public:
		using RenderFunc = std::function<void()>;

		BaseWindow(std::string_view name)
			: m_Name(name), m_Commands(100), m_IsOpen(false)
		{}

		virtual void OnEvent(Lamp::Event& e) = 0;

		inline bool& GetIsOpen() { return m_IsOpen; }
		inline const std::string& GetLabel() { return m_Name; }
		inline std::vector<RenderFunc>& GetRenderFuncs() { return m_RenderFuncs; }

		const std::string GetName()
		{
			std::string name = m_Name;
			name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

			return name;
		}

	protected:
		std::string m_Name;
		bool m_IsOpen;
		CommandStack<Command> m_Commands;

		std::vector<RenderFunc> m_RenderFuncs;
	};
}