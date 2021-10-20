#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Utility/PlatformUtility.h"

#include <imgui_stdlib.h>

static int s_contextId = 0;
static uint32_t s_stackId = 0;

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

void UI::PushId()
{
	ImGui::PushID(s_contextId++);
	s_stackId = 0;
}

void UI::PopId()
{
	ImGui::PopID();
	s_contextId--;
}

bool UI::BeginProperties(const std::string& name, bool showHeader)
{
	ImGuiTableFlags flags;
	if (showHeader)
	{
		ImGui::TableHeader(name.c_str());
	}
	PushId();
	return ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp);
}

void UI::EndProperties()
{
	ImGui::EndTable();
	PopId();
}

void UI::ShiftCursor(float x, float y)
{
	ImVec2 pos = { ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y };
	ImGui::SetCursorPos(pos);
}

bool UI::PropertyAxisColor(const std::string& text, glm::vec3& value, float resetValue)
{
	ScopedStyle cellPad{ ImGuiStyleVar_CellPadding, { 4.f, 0.f } };

	bool changed = false;

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
	ImVec2 buttonSize = { lineHeight + 3.f, lineHeight };

	{
		ScopedColor color{ ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.f } };

		std::string butId = "X##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.x = resetValue;
			changed = true;
		}
	}

	ImGui::SameLine();
	std::string id = "##" + std::to_string(s_stackId++);
	changed = true;
	
	if (ImGui::DragFloat(id.c_str(), &value.x, 0.1f))
		changed = true;
	
	ImGui::PopItemWidth();
	ImGui::SameLine();

	{
		ScopedColor color{ ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.f } };

		std::string butId = "Y##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.y = resetValue;
			changed = true;
		}
	}

	ImGui::SameLine();
	id = "##" + std::to_string(s_stackId++);

	if (ImGui::DragFloat(id.c_str(), &value.y, 0.1f))
		changed = true;

	ImGui::PopItemWidth();
	ImGui::SameLine();

	{
		ScopedColor color{ ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.f } };
		ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.f } };
		ScopedColor colora{ ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.f } };

		std::string butId = "Z##" + std::to_string(s_stackId++);
		if (ImGui::Button(butId.c_str(), buttonSize))
		{
			value.z = resetValue;
			changed = true;
		}
	}

	ImGui::SameLine();
	id = "##" + std::to_string(s_stackId++);
	if (ImGui::DragFloat(id.c_str(), &value.z, 0.1f))
		changed = true;

	ImGui::PopItemWidth();
	ImGui::PopStyleVar();

	return true;
}

bool UI::Property(const std::string& text, int& value, int min, int max)
{
	bool changed = false;

	ScopedStyle cellPad{ ImGuiStyleVar_CellPadding, { 4.f, 0.f } };

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());
	if (ImGui::DragInt(id.c_str(), &value, 1.f, min, max))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, bool& value)
{
	bool changed = false;

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);

	if (ImGui::Checkbox(id.c_str(), &value))
	{
		changed = true;
	}

	return changed;
}

bool UI::Property(const std::string& text, float& value)
{
	bool changed = false;

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat(id.c_str(), &value, 1.f))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec2& value, float min, float max)
{
	bool changed = false;

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec3& value, float min, float max)
{
	bool changed = false;

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
}

bool UI::Property(const std::string& text, glm::vec4& value, float min, float max)
{
	bool changed = false;

	ImGui::TableNextColumn();
	ImGui::Text(text.c_str());

	ImGui::TableNextColumn();
	std::string id = "##" + std::to_string(s_stackId++);
	ImGui::PushItemWidth(ImGui::GetColumnWidth());

	if (ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.f, min, max))
	{
		changed = true;
	}

	ImGui::PopItemWidth();

	return changed;
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