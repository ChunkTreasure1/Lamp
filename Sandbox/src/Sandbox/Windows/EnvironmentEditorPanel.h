#pragma once

#include "BaseWindow.h"

namespace Sandbox
{
	class EnvironmentEditorPanel : public BaseWindow
	{
	public:
		EnvironmentEditorPanel(std::string_view name);
		void OnEvent(Lamp::Event& e) override;

	private:
		bool OnImGuiUpdate(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);
	};
}