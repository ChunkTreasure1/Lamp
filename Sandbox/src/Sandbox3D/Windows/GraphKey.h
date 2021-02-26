#pragma once

#include "BaseWindow.h"
#include <Lamp/Objects/Entity/Base/BaseComponent.h>
#include <Lamp/GraphKey/Node.h>

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

		void CreateComponentNodes();
		void RemoveNode(uint32_t id);

		void DrawNode(Ref<Lamp::Node>& node);
		void DrawInput(Lamp::InputAttribute& attr, Ref<Lamp::Node>& node);
		void DrawOutput(Lamp::OutputAttribute& attr, Ref<Lamp::Node>& node);

	private:
		std::vector<Ref<Lamp::Node>> m_ComponentNodes;
		std::vector<Ref<Lamp::EntityComponent>> m_BaseComponents;
	
		std::vector<Ref<Lamp::Node>> m_ExistingNodes;
		std::vector<Ref<Lamp::Link>> m_Links;

		bool m_NodeWindowFocused;
	};
}