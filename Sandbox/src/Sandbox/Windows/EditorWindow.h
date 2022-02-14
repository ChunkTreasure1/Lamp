#pragma once

#include "Sandbox/Actions/CommandStack.h"

#include <Lamp/Event/Event.h>

#include <string_view>

namespace Sandbox
{
	class EditorWindow
	{
	public:
		EditorWindow(std::string_view name);
		virtual ~EditorWindow() = default;

		virtual void OnEvent(Lamp::Event& e) = 0;

		inline void SetIsOpen(bool v) { m_isOpen = v; }
		inline bool& GetIsOpen() { return m_isOpen; }

		inline const std::string& GetLabel() { return m_name; }
		inline const float GetDeltaTime() { return m_deltaTime; }
		inline const bool IsFocused() const { return m_isFocused; }
		inline CommandStack& GetCommandStack() { return m_commandStack; }

		const std::string GetName() const;

	protected:
		std::string m_name;
		float m_deltaTime;

		bool m_isOpen;
		bool m_isFocused;

		CommandStack m_commandStack;
	};
}