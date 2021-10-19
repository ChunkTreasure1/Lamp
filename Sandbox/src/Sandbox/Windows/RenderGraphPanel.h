#pragma once
#include "Sandbox/Windows/BaseWindow.h"

#include "Lamp/Rendering/RenderGraph/RenderGraph.h"

namespace Sandbox
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
		bool OnKeyPressed(Lamp::KeyPressedEvent& e);

		void UpdateGraphWindow();
		void UpdateNodeWindow();
		void UpdateToolbar();

		void DrawNode(Ref<Lamp::RenderNode> node);
		void CheckLinkCreated();
		void DeleteMarkedLinks();

	private:
		Ref<Lamp::RenderGraph> m_CurrentlyOpenGraph;
		std::vector<int> m_SelectedNodes;
		std::vector<int> m_SelectedLinks;
		
		//Icons
		Ref<Lamp::Texture2D> m_LoadIcon;
		Ref<Lamp::Texture2D> m_SaveIcon;
		Ref<Lamp::Texture2D> m_NewIcon;
	};
}