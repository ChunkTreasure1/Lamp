#pragma once

#include "BaseWindow.h"

namespace Sandbox3D
{
	class VisualScripting : public BaseWindow
	{
	public:
		VisualScripting(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		void UpdateNodeWindow();
	};
}