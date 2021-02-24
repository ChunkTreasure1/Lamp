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

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void UpdateNodeWindow();
		void UpdateNodeList();

		void CreateComponentNodes();
		void CreateRegistryNodes();
		void RemoveNode(uint32_t id);

		void DrawNode(Ref<Lamp::Node>& node);

	private:
		std::vector<Ref<Lamp::Node>> m_ComponentNodes;
		std::vector<Ref<Lamp::EntityComponent>> m_BaseComponents;
	
		std::vector<Ref<Lamp::Node>> m_ExistingNodes;
		std::vector<Lamp::Link> m_Links;

		bool m_NodeWindowFocused;
	};
}