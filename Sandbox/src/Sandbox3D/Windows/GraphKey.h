#pragma once

#include "BaseWindow.h"
#include <Lamp/Objects/Entity/Base/BaseComponent.h>
#include <Lamp/GraphKey/GraphKeyGraph.h>

namespace Sandbox3D
{
	class GraphKey : public BaseWindow
	{
	public:
		GraphKey(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;

		std::vector<Ref<Lamp::Node>>& GetNodes() { return m_ExistingNodes; }

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void UpdateNodeWindow();
		void UpdateNodeList();
		void UpdatePropertiesWindow();
		void UpdateRightClickPopup();

		void CreateComponentNodes();
		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

		void DrawNode(Ref<Lamp::Node>& node);
		void DrawInput(Lamp::InputAttribute& attr, Ref<Lamp::Node>& node, bool isProperties = false);
		void DrawOutput(Lamp::OutputAttribute& attr, Ref<Lamp::Node>& node, bool isProperties = false);

		bool IsHovered(const glm::vec2& pos, const glm::vec2& size);

	private:
		std::vector<Ref<Lamp::Node>> m_ComponentNodes;
		std::vector<Ref<Lamp::EntityComponent>> m_BaseComponents;
	
		std::vector<Ref<Lamp::Node>> m_ExistingNodes;
		std::vector<Ref<Lamp::Link>> m_Links;

		Ref<Lamp::Node> m_SelectedNode = nullptr;
		Ref<Lamp::GraphKeyGraph> m_CurrentlyOpenGraph = nullptr;

		bool m_NodeWindowFocused;
	};
}