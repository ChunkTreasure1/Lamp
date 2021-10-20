#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Utility/PlatformUtility.h"

#include <imgui_stdlib.h>

static uint64_t s_StackId = 0;

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

bool UI::Property(const std::string& text, int& value, int min, int max)
{
	bool changed = false;

	ScopedStyle cellPad{ ImGuiStyleVar_CellPadding, { 4.f, 0.f } };

	ImGui::PushID(s_StackId++);
	if (ImGui::BeginTable("table", 2, ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableNextColumn();
		ImGui::SameLine(30.f);
		ImGui::Text(text.c_str(), ImGui::GetColumnWidth());

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragInt(text.c_str(), &value, 1.f, min, max))
		{
			changed = true;
		}

		ImGui::PopItemWidth();
		ImGui::EndTable();
	}
	ImGui::PopID();

	return changed;
}

bool UI::Property(const std::string& text, bool& value)
{
	if (ImGui::Checkbox(text.c_str(), &value))
	{
		return true;
	}

	return false;
}

bool UI::Property(const std::string& text, float& value)
{
	if (ImGui::DragFloat(text.c_str(), &value, 1.f))
	{
		return true;
	}

	return false;
}

bool UI::Property(const std::string& text, glm::vec2& value, float min, float max)
{
	if (ImGui::DragFloat2(text.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		return true;
	}
	return false;
}

bool UI::Property(const std::string& text, glm::vec3& value, float min, float max)
{
	if (ImGui::DragFloat3(text.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		return true;
	}

	return false;
}

bool UI::Property(const std::string& text, glm::vec4& value, float min, float max)
{
	if (ImGui::DragFloat4(text.c_str(), glm::value_ptr(value), 1.f, min, max))
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

bool UI::Property(const std::string& text, glm::vec4& value, bool useAlpha)
{
	if (useAlpha)
	{
		if (ImGui::ColorEdit4(text.c_str(), glm::value_ptr(value)))
		{
			return true;
		}
	}
	else
	{
		float values[3] = { value.x, value.y, value.z };
		if (ImGui::ColorEdit3(text.c_str(), values))
		{
			value = glm::vec4({ values[0], values[1], values[2], 1.f });
		}
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