#pragma once
#include "Sandbox3D/Windows/BaseWindow.h"

namespace Sandbox3D
{
	class RenderGraph : public BaseWindow
	{
	public:
		RenderGraph(std::string_view name);
		virtual void OnEvent(Lamp::Event& e) override;

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void UpdateGraphWindow();
	};
}