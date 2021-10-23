#include "lppch.h"
#include "UIUtility.h"

#include <imgui_stdlib.h>

bool UI::InputTextOnSameline(std::string& string, const std::string& id)
{
	ImGui::SameLine();
	return ImGui::InputText(id.c_str(), &string);
}

bool UI::Property(const std::string& text, const std::string& value)
{
	if (ImGui::InputText(text.c_str(), &const_cast<std::string&>(value)))
	{
		return true;
	}

	return false;
}

bool UI::Property(const std::string& text, std::string& value)
{
	if (ImGui::InputText(text.c_str(), &value))
	{
		return true;
	}

	return false;
}

bool UI::Property(const std::string& text, std::filesystem::path& path)
{
	bool changed = false;
	if (ImGui::InputText(text.c_str(), &path.string()))
	{
		changed = true;
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			const wchar_t* inPath = (const wchar_t*)payload->Data;
			std::filesystem::path newPath = std::filesystem::path("assets") / inPath;

			path = newPath;
			changed = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Select..."))
	{
		std::string newPath = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
		if (!newPath.empty())
		{
			path = newPath;

			changed = true;
		}
	}

	return changed;
}