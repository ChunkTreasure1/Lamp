#include "RenderGraphPanel.h"

#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeStart.h>
#include <Lamp/Rendering/RenderGraph/RenderGraphUtils.h>

#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Utility/PlatformUtility.h>

#include <Lamp/Input/KeyCodes.h>
#include <Lamp/Utility/UIUtility.h>

#include <ImNodes/ImNodes.h>
#include <imgui/imgui_stdlib.h>

namespace Sandbox
{
	using namespace Lamp;

	RenderGraphPanel::RenderGraphPanel(std::string_view name)
		: EditorWindow(name)
	{
		//Setup icons
		m_loadIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/loadIcon.png");
		m_saveIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/saveIcon.png");
		m_newIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/newIcon.png");

		RenderGraphSpecification spec;
		spec.name = "test";
	}

	void RenderGraphPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(RenderGraphPanel::UpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(RenderGraphPanel::OnUpdate));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(RenderGraphPanel::OnKeyPressed));
	}

	void RenderGraphPanel::Open(Ref<Lamp::RenderGraph> graph)
	{
		m_currentlyOpenGraph = graph;

		for (const auto& node : graph->GetSpecification().nodes)
		{
			ImNodes::SetNodeEditorSpacePos(node->id, ImVec2{ node->position.x, node->position.y });
		}
	}

	void RenderGraphPanel::Start()
	{
		for (auto& node : m_currentlyOpenGraph->GetSpecification().nodes)
		{
			node->Start();
		}
	}

	bool RenderGraphPanel::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_isOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_name.c_str(), &m_isOpen);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_name.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();

		UpdateGraphWindow();
		UpdateNodeWindow();
		UpdateToolbar();

		return false;
	}

	bool RenderGraphPanel::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		return false;
	}

	bool RenderGraphPanel::OnKeyPressed(Lamp::KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case LP_KEY_DELETE:
			{
				if (!m_currentlyOpenGraph)
				{
					break;
				}

				for (uint32_t i = 0; i < m_selectedNodes.size(); i++)
				{
					m_currentlyOpenGraph->RemoveNode((GraphUUID)m_selectedNodes[i]);
				}
				m_selectedNodes.clear();

				for (uint32_t i = 0; i < m_selectedLinks.size(); i++)
				{
					m_currentlyOpenGraph->RemoveLink((GraphUUID)m_selectedLinks[i]);
				}
				m_selectedLinks.clear();

				break;
			}
		}

		return false;
	}

	void RenderGraphPanel::UpdateGraphWindow()
	{
		ImGui::Begin("Graph", &m_isOpen);

		ImNodes::BeginNodeEditor();

		if (m_currentlyOpenGraph)
		{
			for (auto& node : m_currentlyOpenGraph->GetSpecification().nodes)
			{
				DrawNode(node);
			}

			for (auto& link : m_currentlyOpenGraph->GetSpecification().links)
			{
				if (link->pInput->type != RenderAttributeType::Pass)
				{
					ImNodes::PushColorStyle(ImNodesCol_Link, Utils::GetTypeColor(link->pInput->type));
					ImNodes::PushColorStyle(ImNodesCol_LinkHovered, Utils::GetTypeHoverColor(link->pInput->type));
				}

				ImNodes::Link(link->id, link->pOutput->id, link->pInput->id);

				if (link->pInput->type != RenderAttributeType::Pass)
				{
					ImNodes::PopColorStyle();
					ImNodes::PopColorStyle();
				}
			}
		}

		ImNodes::EndNodeEditor();

		CheckLinkCreated();

		m_selectedNodes.clear();
		const int numSelectedNodes = ImNodes::NumSelectedNodes();
		if (numSelectedNodes > 0)
		{
			m_selectedNodes.resize(numSelectedNodes);

			ImNodes::GetSelectedNodes(m_selectedNodes.data());
		}

		m_selectedLinks.clear();
		const int numSelectedLinks = ImNodes::NumSelectedLinks();
		if (numSelectedLinks > 0)
		{
			m_selectedLinks.resize(numSelectedLinks);

			ImNodes::GetSelectedLinks(m_selectedLinks.data());
		}

		ImGui::End();
	}

	void RenderGraphPanel::UpdateNodeWindow()
	{
		ImGui::Begin("Nodes##render");

		if (ImGui::Button("Create Start") && m_currentlyOpenGraph)
		{
			if (!m_currentlyOpenGraph->GetSpecification().startNode)
			{
				Ref<RenderNodeStart> node = CreateRef<RenderNodeStart>();
				m_currentlyOpenGraph->AddNode(node);
				node->Initialize();
				m_currentlyOpenGraph->GetSpecification().startNode = node;
			}
		}

		if (ImGui::Button("Save") && m_currentlyOpenGraph)
		{
			g_pEnv->pAssetManager->SaveAsset(m_currentlyOpenGraph);
		}

		ImGui::End();
	}

	void RenderGraphPanel::UpdateToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.505f, 0.51f, 0.5f));

		ImGui::Begin("##toolbarRenderGraph", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		float size = ImGui::GetWindowHeight() - 4.f;

		if (ImGui::ImageButton(UI::GetTextureID(m_newIcon), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			Ref<RenderGraph> graph = CreateRef<RenderGraph>();
			graph->GetSpecification().name = "New";

			Open(graph);
		}

		ImGui::SameLine();

		if (ImGui::ImageButton(UI::GetTextureID(m_loadIcon), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			std::filesystem::path path = FileDialogs::OpenFile("RenderGraph (*.rendergraph)\0*.rendergraph\0");
			if (!path.empty() && std::filesystem::exists(path))
			{
				Ref<Asset> graph;
				g_pEnv->pAssetManager->LoadAsset(path, graph);
				Open(std::dynamic_pointer_cast<RenderGraph>(graph));
			}
		}

		ImGui::SameLine();

		if (ImGui::ImageButton(UI::GetTextureID(m_saveIcon), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			if (m_currentlyOpenGraph)
			{
				if (m_currentlyOpenGraph->Path.empty())
				{
					std::filesystem::path path = FileDialogs::SaveFile("RenderGraph (*.rendergraph)\0*.rendergraph\0");
					m_currentlyOpenGraph->Path = path;
				}

				g_pEnv->pAssetManager->SaveAsset(m_currentlyOpenGraph);
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void RenderGraphPanel::DrawNode(Ref<Lamp::RenderNode> node)
	{
		node->DrawNode();
	}

	void RenderGraphPanel::CheckLinkCreated()
	{
		if (!m_currentlyOpenGraph)
		{
			return;
		}

		int startAttr, endAttr;
		if (ImNodes::IsLinkCreated(&startAttr, &endAttr))
		{
			RenderNode* pStartNode = nullptr;
			RenderNode* pEndNode = nullptr;

			RenderAttribute* pStartAttr = nullptr;
			RenderAttribute* pEndAttr = nullptr;

			for (auto& node : m_currentlyOpenGraph->GetSpecification().nodes)
			{
				for (uint32_t i = 0; i < node->inputs.size(); i++)
				{
					if (node->inputs[i]->id == startAttr)
					{
						pStartNode = node.get();
						pStartAttr = node->inputs[i].get();

						break;
					}

					if (node->inputs[i]->id == endAttr)
					{
						pEndNode = node.get();
						pEndAttr = node->inputs[i].get();

						break;
					}
				}

				for (uint32_t i = 0; i < node->outputs.size(); i++)
				{
					if (node->outputs[i]->id == startAttr)
					{
						pStartNode = node.get();
						pStartAttr = node->outputs[i].get();

						break;
					}

					if (node->outputs[i]->id == endAttr)
					{
						pEndNode = node.get();
						pEndAttr = node->outputs[i].get();

						break;
					}
				}
			}

			if (pStartAttr && pEndAttr)
			{
				if (pStartAttr->type == pEndAttr->type)
				{
					Ref<RenderLink> link = CreateRef<RenderLink>();

					if (auto p = dynamic_cast<RenderInputAttribute*>(pStartAttr))
					{
						link->pInput = p;
					}
					if (auto p = dynamic_cast<RenderOutputAttribute*>(pStartAttr))
					{
						link->pOutput = p;
					}

					if (auto p = dynamic_cast<RenderInputAttribute*>(pEndAttr))
					{
						link->pInput = p;
					}
					if (auto p = dynamic_cast<RenderOutputAttribute*>(pEndAttr))
					{
						link->pOutput = p;
					}

					pStartNode->links.push_back(link);
					pEndNode->links.push_back(link);

					m_currentlyOpenGraph->GetSpecification().links.push_back(link);
				}
			}
		}
	}

	void RenderGraphPanel::DeleteMarkedLinks()
	{
		for (const auto& node : m_currentlyOpenGraph->GetSpecification().nodes)
		{
			for (int i = 0; i < node->links.size(); i++)
			{
				if (node->links[i]->markedForDelete)
				{
					node->links.erase(node->links.begin() + i);
				}
			}
		}

		for (int i = 0; i < m_currentlyOpenGraph->GetSpecification().links.size(); i++)
		{
			if (m_currentlyOpenGraph->GetSpecification().links[i]->markedForDelete)
			{
				m_currentlyOpenGraph->GetSpecification().links.erase(m_currentlyOpenGraph->GetSpecification().links.begin() + i);
				break;
			}
		}
	}
}