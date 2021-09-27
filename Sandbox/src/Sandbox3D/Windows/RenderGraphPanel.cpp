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

		ImGui::End();
	}

	void RenderGraphPanel::UpdateNodeWindow()
	{
		ImGui::Begin("Nodes##render");

		if (ImGui::Button("Create pass") && m_CurrentlyOpenGraph)
		{
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;

			Ref<RenderNode> node = CreateRef<RenderNodePass>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			node->Initialize();
		}

		if (ImGui::Button("Create buffer") && m_CurrentlyOpenGraph)
		{
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;

			Ref<RenderNode> node = CreateRef<RenderNodeFramebuffer>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			node->Initialize();
		}

		if (ImGui::Button("Create texture") && m_CurrentlyOpenGraph)
		{
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;

			Ref<RenderNode> node = CreateRef<RenderNodeTexture>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			node->Initialize();
		}

		if (ImGui::Button("Create Dynamic Uniform") && m_CurrentlyOpenGraph)
		{
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;

			Ref<RenderNodeDynamicUniform> node = CreateRef<RenderNodeDynamicUniform>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->dataName = "Exposure";
			node->pData = RegisterData(&Renderer3D::GetSettings().HDRExposure);
			node->uniformType = Lamp::UniformType::Float;
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			node->Initialize();
		}

		if (ImGui::Button("Create Start") && m_CurrentlyOpenGraph)
		{
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;

			Ref<RenderNodeStart> node = CreateRef<RenderNodeStart>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			node->Initialize();
		}

		if (ImGui::Button("Create End") && m_CurrentlyOpenGraph)
		{
			if (!m_CurrentlyOpenGraph->GetSpecification().endNode)
			{
				m_CurrentlyOpenGraph->GetCurrentId() += 1000;

				Ref<RenderNodeEnd> node = CreateRef<RenderNodeEnd>();
				m_CurrentlyOpenGraph->AddNode(node);
				node->id = m_CurrentlyOpenGraph->GetCurrentId();
				node->currId = node->id;
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
				Open(ResourceCache::GetAsset<RenderGraph>(path));
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
					link->id = m_CurrentlyOpenGraph->GetCurrentId()++;

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

					pStartAttr->links.push_back(link);
					pEndAttr->links.push_back(link);

					m_CurrentlyOpenGraph->GetSpecification().links.push_back(link);
				}
			}
		}
	}
}