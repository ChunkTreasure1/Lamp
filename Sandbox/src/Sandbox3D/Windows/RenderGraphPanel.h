#pragma once
#include "Sandbox3D/Windows/BaseWindow.h"

#include "Lamp/Rendering/RenderGraph/RenderGraph.h"

namespace Sandbox3D
{

	class RenderGraphPanel : public BaseWindow
	{
	public:
		RenderGraphPanel(std::string_view name);
		virtual void OnEvent(Lamp::Event& e) override;

		void Open(Ref<Lamp::RenderGraph> graph);
		void Start();
		inline Ref<Lamp::RenderGraph> GetOpenGraph() { return m_CurrentlyOpenGraph; }

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void UpdateGraphWindow();
		void UpdateNodeWindow();

		void DrawNode(Ref<Lamp::RenderNode> node);
		void CheckLinkCreated();

	private:
		Ref<Lamp::RenderGraph> m_CurrentlyOpenGraph;
	};
}