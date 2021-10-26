#pragma once

#include "BaseWindow.h"
#include <Lamp/Objects/Entity/Base/BaseComponent.h>
#include <Lamp/GraphKey/GraphKeyGraph.h>

namespace Sandbox
{
	class GraphKeyPanel : public BaseWindow
	{
	public:
		GraphKeyPanel(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;

		void SetCurrentlyOpenGraph(Ref<Lamp::GraphKeyGraph> graph, uint32_t entity);
		inline Ref<Lamp::GraphKeyGraph>& GetCurrentlyOpenGraph() { return m_CurrentlyOpenGraph; }

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void UpdateNodeWindow();
		void UpdateNodeList();
		void UpdatePropertiesWindow();
		void UpdateGraphList();

		void CreateComponentNodes();
		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

		void DrawNode(Ref<Lamp::Node>& node);
		void DrawInput(Lamp::InputAttribute& attr, Ref<Lamp::Node>& node, bool isProperties = false);
		void DrawOutput(Lamp::OutputAttribute& attr, Ref<Lamp::Node>& node, bool isProperties = false);

		bool IsHovered(const glm::vec2& pos, const glm::vec2& size);
		void AddNode(const std::string& name);

	private:
		std::vector<Ref<Lamp::Node>> m_ComponentNodes;
		std::vector<Ref<Lamp::EntityComponent>> m_BaseComponents;

		Ref<Lamp::Node> m_SelectedNode = nullptr;
		Ref<Lamp::GraphKeyGraph> m_CurrentlyOpenGraph = nullptr;
		uint32_t m_CurrentEntityId = 0;

		bool m_NodeWindowFocused;
		int m_CurrentlyHovered = -1;
	};
}