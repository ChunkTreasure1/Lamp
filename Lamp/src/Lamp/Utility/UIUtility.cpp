#include "lppch.h"
#include "UIUtility.h"

void UI::ImageText(uint32_t texId, const std::string& text)
{
	ImVec2 size = ImGui::CalcTextSize(text.c_str());
	ImGui::Image((ImTextureID)texId, { size.y, size.y });
	ImGui::SameLine();
	ImGui::Text(text.c_str());
}

bool UI::ImageTreeNodeEx(uint32_t texId, const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
	ImVec2 size = ImGui::CalcTextSize(fmt);
	ImGui::Image((ImTextureID)texId, { size.y, size.y });
	ImGui::SameLine();
	return ImGui::TreeNodeEx(ptr_id, flags, fmt);
}