#include "RenderGraphPanel.h"
#include <ImNodes/ImNodes.h>

#include <imgui/imgui_stdlib.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>

namespace Sandbox3D
{
	using namespace Lamp;

	RenderGraphPanel::RenderGraphPanel(std::string_view name)
		: BaseWindow(name)
	{
		RenderGraphSpecification spec;
		spec.name = "test";
		m_CurrentlyOpenGraph = CreateRef<RenderGraph>(spec);
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
				ImNodes::Link(link->id, link->outputPass->id, link->inputPass->id);
			}
		}

		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();

		ImGui::End();
	}

	void RenderGraphPanel::UpdateNodeWindow()
	{
		ImGui::Begin("Nodes##render");

		if (ImGui::Button("Create pass") && m_CurrentlyOpenGraph)
		{
			Ref<RenderNode> node = CreateRef<RenderNodePass>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;
		}

		if (ImGui::Button("Create buffer") && m_CurrentlyOpenGraph)
		{
			Ref<RenderNode> node = CreateRef<RenderNodeFramebuffer>();
			m_CurrentlyOpenGraph->AddNode(node);
			node->id = m_CurrentlyOpenGraph->GetCurrentId();
			node->currId = node->id;
			m_CurrentlyOpenGraph->GetCurrentId() += 1000;
		}

		ImGui::End();
	}

	void RenderGraphPanel::DrawNode(Ref<Lamp::RenderNode> node)
	{
		node->DrawNode();
	}
}