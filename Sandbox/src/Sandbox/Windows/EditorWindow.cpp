#include "EditorWindow.h"

namespace Sandbox
{
	EditorWindow::EditorWindow(std::string_view name)
		: m_name(name), m_isOpen(false), m_deltaTime(0.f)
	{
	}

	const std::string EditorWindow::GetName() const
	{
		std::string name = m_name;
		name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

		return name;
	}
}