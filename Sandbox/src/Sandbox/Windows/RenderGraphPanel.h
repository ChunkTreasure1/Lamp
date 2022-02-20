#pragma once

#include "Sandbox/Windows/EditorWindow.h"

#include <Lamp/Event/KeyEvent.h>
#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/Rendering/RenderGraph/RenderGraph.h>

namespace Sandbox
{

	class RenderGraphPanel : public EditorWindow
	{
	public:
		RenderGraphPanel(std::string_view name);
		void OnEvent(Lamp::Event& e) override;

		void Open(Ref<Lamp::RenderGraph> graph);
		void Start();
		inline Ref<Lamp::RenderGraph> GetOpenGraph() { return m_currentlyOpenGraph; }

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
		Ref<Lamp::RenderGraph> m_currentlyOpenGraph;
		std::vector<int> m_selectedNodes;
		std::vector<int> m_selectedLinks;
		
		//Icons
		Ref<Lamp::Texture2D> m_loadIcon;
		Ref<Lamp::Texture2D> m_saveIcon;
		Ref<Lamp::Texture2D> m_newIcon;
	};
}