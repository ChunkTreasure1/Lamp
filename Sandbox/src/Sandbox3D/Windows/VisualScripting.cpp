#include "VisualScripting.h"

#include <imnodes.h>

namespace Sandbox3D
{
	using namespace Lamp;

	VisualScripting::VisualScripting(std::string_view name)
		: BaseWindow(name)
	{

	}

	void VisualScripting::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(VisualScripting::UpdateImGui));
	}

	bool VisualScripting::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
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

		UpdateNodeWindow();

		return false;
	}

	void VisualScripting::UpdateNodeWindow()
	{
		ImGui::Begin("Main", &m_IsOpen);

		imnodes::BeginNodeEditor();

		imnodes::BeginNode(0);
		ImGui::Dummy(ImVec2(80.0f, 45.0f));
		imnodes::EndNode();

		imnodes::EndNodeEditor();

		ImGui::End();
	}
}