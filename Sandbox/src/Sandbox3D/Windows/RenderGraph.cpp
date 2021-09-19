#include "RenderGraph.h"
#include <ImNodes/ImNodes.h>

namespace Sandbox3D
{
	using namespace Lamp;

	RenderGraph::RenderGraph(std::string_view name)
		: BaseWindow(name)
	{
	}

	void RenderGraph::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(RenderGraph::UpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(RenderGraph::OnUpdate));
	}

	bool RenderGraph::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
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

		return false;
	}

	bool RenderGraph::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		return false;
	}

	void RenderGraph::UpdateGraphWindow()
	{
		ImGui::Begin("Graph", &m_IsOpen);

		ImNodes::BeginNodeEditor();

		ImNodes::EndNodeEditor();

		ImGui::End();
	}
}