#include "RenderGraphPanel.h"

#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodePass.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeFramebuffer.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeTexture.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeDynamicUniform.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeStart.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeEnd.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Utility/PlatformUtility.h>

#include <ImNodes/ImNodes.h>
#include <imgui/imgui_stdlib.h>

namespace Sandbox3D
{
	using namespace Lamp;

	RenderGraphPanel::RenderGraphPanel(std::string_view name)
		: BaseWindow(name)
	{
		//Setup icons
		m_LoadIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/loadIcon.png");
		m_SaveIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/saveIcon.png");
		m_NewIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/newIcon.png");

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
		m_CurrentlyOpenGraph = graph;

		for (const auto& node : graph->GetSpecification().nodes)
		{
			ImNodes::SetNodeEditorSpacePos(node->id, ImVec2{ node->position.x, node->position.y });
		}
	}

	void RenderGraphPanel::Start()
	{
		for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
		{
			node->Start();
		}
	}

	bool RenderGraphPanel::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_IsOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_Name.c_str(), &m_IsOpen);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_Name.c_str());
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
				if (!m_CurrentlyOpenGraph)
				{
					break;
				}

				for (uint32_t i = 0; i < m_SelectedNodes.size(); i++)
				{
					m_CurrentlyOpenGraph->RemoveNode((GraphUUID)m_SelectedNodes[i]);
				}
				m_SelectedNodes.clear();

				for (uint32_t i = 0; i < m_SelectedLinks.size(); i++)
				{
					m_CurrentlyOpenGraph->RemoveLink((GraphUUID)m_SelectedLinks[i]);
				}
				m_SelectedLinks.clear();

				break;
			}
		}

		return false;
	}

	void RenderGraphPanel::UpdateGraphWindow()
	{
		ImGui::Begin("Graph", &m_IsOpen);

		ImNodes::BeginNodeEditor();

		if (m_CurrentlyOpenGraph)
		{
			for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
			{
				DrawNode(node);
			}

			for (auto& link : m_CurrentlyOpenGraph->GetSpecification().links)
			{
				ImNodes::Link(link->id, link->pOutput->id, link->pInput->id);
			}
		}

		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();

		CheckLinkCreated();

		m_SelectedNodes.clear();
		const int numSelectedNodes = ImNodes::NumSelectedNodes();
		if (numSelectedNodes > 0)
		{
			m_SelectedNodes.resize(numSelectedNodes);

			ImNodes::GetSelectedNodes(m_SelectedNodes.data());
		}

		m_SelectedLinks.clear();
		const int numSelectedLinks = ImNodes::NumSelectedLinks();
		if (numSelectedLinks > 0)
		{
			m_SelectedLinks.resize(numSelectedLinks);

			ImNodes::GetSelectedLinks(m_SelectedLinks.data());
		}

		ImGui::End();
	}

	void RenderGraphPanel::UpdateNodeWindow()
	{
		ImGui::Begin("Nodes##render");

		if (ImGui::Button("Create pass") && m_CurrentlyOpenGraph)
		{
			Ref<RenderNode> node = CreateRef<RenderNodePass>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->Initialize();
		}

		if (ImGui::Button("Create buffer") && m_CurrentlyOpenGraph)
		{
			Ref<RenderNode> node = CreateRef<RenderNodeFramebuffer>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->Initialize();
		}

		if (ImGui::Button("Create texture") && m_CurrentlyOpenGraph)
		{
			Ref<RenderNode> node = CreateRef<RenderNodeTexture>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->Initialize();
		}

		if (ImGui::Button("Create Dynamic Uniform") && m_CurrentlyOpenGraph)
		{
			Ref<RenderNodeDynamicUniform> node = CreateRef<RenderNodeDynamicUniform>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->dataName = "Exposure";
			node->pData = RegisterData(&Renderer3D::GetSettings().HDRExposure);
			node->uniformType = Lamp::UniformType::Float;
			node->Initialize();
		}

		if (ImGui::Button("Create Start") && m_CurrentlyOpenGraph)
		{
			if (!m_CurrentlyOpenGraph->GetSpecification().startNode)
			{
				Ref<RenderNodeStart> node = CreateRef<RenderNodeStart>();
				m_CurrentlyOpenGraph->AddNode(node);
				node->Initialize();
				m_CurrentlyOpenGraph->GetSpecification().startNode = node;
			}
		}

		if (ImGui::Button("Create End") && m_CurrentlyOpenGraph)
		{
			if (!m_CurrentlyOpenGraph->GetSpecification().endNode)
			{
				Ref<RenderNodeEnd> node = CreateRef<RenderNodeEnd>();
				m_CurrentlyOpenGraph->AddNode(node);
				node->Initialize();
				m_CurrentlyOpenGraph->GetSpecification().endNode = node;
			}
		}

		if (ImGui::Button("Save") && m_CurrentlyOpenGraph)
		{
			g_pEnv->pAssetManager->SaveAsset(m_CurrentlyOpenGraph);
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

		if (ImGui::ImageButton((ImTextureID)m_NewIcon->GetID(), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			Ref<RenderGraph> graph = CreateRef<RenderGraph>();
			graph->GetSpecification().name = "New";

			Open(graph);
		}

		ImGui::SameLine();

		if (ImGui::ImageButton((ImTextureID)m_LoadIcon->GetID(), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			std::string path = FileDialogs::OpenFile("RenderGraph (*.rendergraph)\0*.rendergraph\0");
			if (!path.empty() && std::filesystem::exists(path))
			{
				Ref<Asset> graph;
				g_pEnv->pAssetManager->LoadAsset(path, graph);
				Open(std::dynamic_pointer_cast<RenderGraph>(graph));
			}
		}

		ImGui::SameLine();

		if (ImGui::ImageButton((ImTextureID)m_SaveIcon->GetID(), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			if (m_CurrentlyOpenGraph)
			{
				if (m_CurrentlyOpenGraph->Path.empty())
				{
					std::string path = FileDialogs::SaveFile("RenderGraph (*.rendergraph)\0*.rendergraph\0");
					m_CurrentlyOpenGraph->Path = path;
				}

				g_pEnv->pAssetManager->SaveAsset(m_CurrentlyOpenGraph);
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
		if (!m_CurrentlyOpenGraph)
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

			for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
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

					m_CurrentlyOpenGraph->GetSpecification().links.push_back(link);
				}
			}
		}
	}

	void RenderGraphPanel::DeleteMarkedLinks()
	{
		for (const auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
		{
			for (int i = 0; i < node->links.size(); i++)
			{
				if (node->links[i]->markedForDelete)
				{
					node->links.erase(node->links.begin() + i);
				}
			}
		}

		for (int i = 0; i < m_CurrentlyOpenGraph->GetSpecification().links.size(); i++)
		{
			if (m_CurrentlyOpenGraph->GetSpecification().links[i]->markedForDelete)
			{
				m_CurrentlyOpenGraph->GetSpecification().links.erase(m_CurrentlyOpenGraph->GetSpecification().links.begin() + i);
				break;
			}
		}
	}
}