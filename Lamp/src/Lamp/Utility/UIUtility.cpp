#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Utility/PlatformUtility.h"

#include <imgui_stdlib.h>

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
	ImGui::Image((ImTextureID)texId, { size.y, size.y }, { 0, 1 }, { 1, 0 });
	ImGui::SameLine();
	return ImGui::TreeNodeEx(ptr_id, flags, fmt);
}

bool UI::InputTextOnSameline(std::string& string, const std::string& id)
{
	ImGui::SameLine();
	return ImGui::InputText(id.c_str(), &string);
}

bool UI::InputInt(const std::string& text, int& value, int min, int max)
{
	if (ImGui::DragInt(text.c_str(), &value, 1.f, min, max))
	{
		return true;
	}

	return false;
}

bool UI::InputBool(const std::string& text, bool& value)
{
	if (ImGui::Checkbox(text.c_str(), &value))
	{
		return true;
	}

	return false;
}

bool UI::InputFloat(const std::string& text, float& value, float min, float max)
{
	if (ImGui::DragFloat(text.c_str(), &value, 1.f, min, max))
	{
		return true;
	}

	return false;
}

bool UI::InputFloat2(const std::string& text, glm::vec2& value, float min, float max)
{
	if (ImGui::DragFloat2(text.c_str(), glm::value_ptr(value)))
	{
		return true;
	}
	return false;
}

bool UI::InputFloat3(const std::string& text, glm::vec3& value, float min, float max)
{
	if (ImGui::DragFloat3(text.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		return true;
	}

	return false;
}

bool UI::InputFloat4(const std::string& text, glm::vec4& value, float min, float max)
{
	if (ImGui::DragFloat4(text.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		return true;
	}

	return false;
}

bool UI::InputString(const std::string& text, std::string& value)
{
	if (ImGui::InputText(text.c_str(), &value))
	{
		return true;
	}

	return false;
}

bool UI::InputPath(const std::string& text, std::string& value)
{
	bool changed = false;
	if (ImGui::InputText(text.c_str(), &value))
	{
		changed = true;
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			const wchar_t* path = (const wchar_t*)payload->Data;
			std::filesystem::path newPath = std::filesystem::path("assets") / path;

			value = newPath.string();
			changed = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Select..."))
	{
		std::string newPath = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
		if (!newPath.empty())
		{
			value = newPath;

			changed = true;
		}
	}

	return changed;
}

bool UI::InputColor3(const std::string& text, glm::vec3& value)
{
	if (ImGui::ColorEdit3(text.c_str(), glm::value_ptr(value)))
	{
		return true;
	}

	return false;
}

bool UI::InputColor4(const std::string& text, glm::vec4& value)
{
	if (ImGui::ColorEdit4(text.c_str(), glm::value_ptr(value)))
	{
		return true;
	}

	return false;
}

