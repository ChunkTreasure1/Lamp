#pragma once

#include "EditorWindow.h"

#include <Lamp/Event/ApplicationEvent.h>

namespace Sandbox
{
	class EnvironmentEditorPanel : public EditorWindow
	{
	public:
		EnvironmentEditorPanel(std::string_view name);
		void OnEvent(Lamp::Event& e) override;

	private:
		bool OnImGuiUpdate(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);
	};
}