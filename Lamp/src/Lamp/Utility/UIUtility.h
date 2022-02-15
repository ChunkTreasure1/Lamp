#pragma once

#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/AssetSystem/Asset.h"
#include "ImGuiExtension.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <imgui_stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

namespace Lamp
{
	class Texture2D;
	class Image2D;
}

namespace UI
{
	static uint32_t s_contextId = 0;
	static uint32_t s_stackId = 0;
	
	class ScopedColor
	{
	public:
		ScopedColor(ImGuiCol_ color, const glm::vec4& newColor)
			: m_Color(color)
		{
			auto& colors = ImGui::GetStyle().Colors;
			m_OldColor = colors[color];
			colors[color] = ImVec4{ newColor.x, newColor.y, newColor.z, newColor.w };
		}

		~ScopedColor()
		{
			auto& colors = ImGui::GetStyle().Colors;
			colors[m_Color] = m_OldColor;
		}

	private:
		ImVec4 m_OldColor;
		ImGuiCol_ m_Color;
	};

	class ScopedStyleFloat
	{
	public:
		ScopedStyleFloat(ImGuiStyleVar_ var, float value)
		{
			ImGui::PushStyleVar(var, value);
		}

		~ScopedStyleFloat()
		{
			ImGui::PopStyleVar();
		}
	};

	class ScopedStyleFloat2
	{
	public:
		ScopedStyleFloat2(ImGuiStyleVar_ var, const glm::vec2& value)
		{
			ImGui::PushStyleVar(var, { value.x, value.y });
		}

		~ScopedStyleFloat2()
		{
			ImGui::PopStyleVar();
		}
	};

	ImTextureID GetTextureID(Ref<Lamp::Texture2D> texture);
	ImTextureID GetTextureID(Ref<Lamp::Image2D> texture);
	ImTextureID GetTextureID(Lamp::Texture2D* texture);

	static uint64_t BytesToMBs(uint64_t input)
	{
		return (input / (1024 * 1024));
	}

	static void ImageText(Ref<Lamp::Texture2D> texture, const std::string& text)
	{
		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		ImGui::Image(GetTextureID(texture), { size.y, size.y });
		ImGui::SameLine();
		ImGui::Text(text.c_str());
	}

	static bool ImageSelectable(Ref<Lamp::Texture2D> texture, const std::string& text, bool selected)
	{
		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		ImGui::Image(GetTextureID(texture), { size.y, size.y }, { 0, 1 }, { 1, 0 });
		ImGui::SameLine();
		return ImGui::Selectable(text.c_str(), selected, ImGuiSelectableFlags_SpanAvailWidth);
	}

	static bool TreeNode(const std::string& text, ImGuiTreeNodeFlags flags = 0)
	{
		ScopedStyleFloat2 frame{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
		ScopedStyleFloat2 spacing{ ImGuiStyleVar_ItemSpacing, { 0.f, 0.f } };

		return ImGui::TreeNodeEx(text.c_str(), flags);
	}

	static bool TreeNodeImage(Ref<Lamp::Texture2D> texture, const std::string& text, ImGuiTreeNodeFlags flags)
	{
		ScopedStyleFloat2 frame{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
		ScopedStyleFloat2 spacing{ ImGuiStyleVar_ItemSpacing, { 0.f, 0.f } };

		ImVec2 size = ImGui::CalcTextSize(text.c_str());
		ImGui::Image(GetTextureID(texture), { size.y, size.y }, { 0, 1 }, { 1, 0 });
		ImGui::SameLine();
		return ImGui::TreeNodeEx(text.c_str(), flags);
	}

	static bool TreeNodeFramed(const std::string& text, bool alwaysOpen = false, bool useOther = false, float rounding = 0.f, const glm::vec2& padding = { 0.f, 0.f })
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (alwaysOpen)
		{
			nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
		}

		if (!useOther)
		{
			return ImGui::TreeNodeEx(text.c_str(), nodeFlags);
		}

		UI::ScopedStyleFloat frameRound(ImGuiStyleVar_FrameRounding, rounding);

		return ImGui::TreeNodeEx(text.c_str(), nodeFlags);
	}

	static bool TreeNodeFramed(const std::string& text, float width, bool useOther = false, float rounding = 0.f, const glm::vec2& padding = { 0.f, 0.f })
	{
		const ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (!useOther)
		{
			return ImGui::TreeNodeWidthEx(text.c_str(), width, nodeFlags);
		}

		UI::ScopedStyleFloat frameRound(ImGuiStyleVar_FrameRounding, rounding);

		return ImGui::TreeNodeWidthEx(text.c_str(), width, nodeFlags);
	}

	static void TreeNodePop()
	{
		ImGui::TreePop();
	}

	static bool InputText(const std::string& id, std::string& text, ImGuiInputTextFlags_ flags = ImGuiInputTextFlags_None)
	{
		return ImGui::InputTextString(id.c_str(), &text, flags);
	}

	static bool InputTextOnSameline(std::string& string, const std::string& id)
	{
		ImGui::SameLine();
		return InputText(id, string);
	}

	static void Separator(ImGuiSeparatorFlags customFlags = 0)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return;

		// Those flags should eventually be overridable by the user
		ImGuiSeparatorFlags flags = (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
		flags |= customFlags;
		ImGui::SeparatorEx(flags);
	}

	static void PushId()
	{
		int id = s_contextId++;
		ImGui::PushID(id);
		s_stackId = 0;
	}

	static void PopId()
	{
		ImGui::PopID();
		s_contextId--;
	}

	static bool BeginPopup(const std::string& id = "")
	{
		if (id.empty())
		{
			return ImGui::BeginPopupContextItem();
		}

		return ImGui::BeginPopupContextItem(id.c_str());
	}

	static void EndPopup()
	{
		ImGui::EndPopup();
	}

	static void SameLine(float offsetX = 0.f, float spacing = -1.f)
	{
		ImGui::SameLine(offsetX, spacing);
	}

	static bool BeginProperties(const std::string& name = "", bool pushId = true)
	{
		if (pushId)
		{
			PushId();
		}
		return ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable);
	}

	static void EndProperties(bool popId = true)
	{
		ImGui::EndTable();
		if (popId)
		{
			PopId();
		}
	}

	static void ShiftCursor(float x, float y)
	{
		ImVec2 pos = { ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y };
		ImGui::SetCursorPos(pos);
	}

	//Inputs
	static bool PropertyAxisColor(const std::string& text, glm::vec3& value, float resetValue = 0.f)
	{
		ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, { 4.f, 0.f });

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

	static bool Combo(const std::string& text, int& currentItem, const std::vector<const char*>& items, float width = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();

		if (width == 0.f)
		{
			ImGui::PushItemWidth(ImGui::GetColumnWidth());
		}
		else
		{
			ImGui::PushItemWidth(width);
		}
		std::string id = "##" + std::to_string(s_stackId++);
		if (ImGui::Combo(id.c_str(), &currentItem, items.data(), (int)items.size()))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static void* DragDropTarget(const std::string& type)
	{
		void* data = nullptr;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(type.c_str()))
			{
				data = pPayload->Data;
			}

			ImGui::EndDragDropTarget();
		}

		return data;
	}

	static void* DragDropTarget(std::initializer_list<std::string> types)
	{
		void* data = nullptr;

		for (const auto& type : types)
		{
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(type.c_str()))
				{
					data = pPayload->Data;
				}

				ImGui::EndDragDropTarget();
			}
		}

		return data;
	}

	static bool ImageButton(uint32_t id, const glm::vec2& size = { 64, 64 })
	{
		return ImGui::ImageButton((ImTextureID)id, ImVec2{ size.x, size.y }, { 0, 1 }, { 1, 0 });
	}

	static bool ImageButton(uint32_t id, std::filesystem::path& path, const char* filter = "All (*.*)\0*.*\0", const glm::vec2& size = { 64, 64 })
	{
		bool changed = false;
		if (ImGui::ImageButton((ImTextureID)id, ImVec2{ size.x, size.y }, { 0, 1 }, { 1, 0 }))
		{
			path = Lamp::FileDialogs::OpenFile(filter);
			changed = true;
		}

		return changed;
	}

	static bool Property(const std::string& text, int& value, bool useMinMax = false, int min = 0, int max = 0)
	{
		bool changed = false;

		ScopedStyleFloat2 cellPad{ ImGuiStyleVar_CellPadding, { 4.f, 0.f } };

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());
		if (ImGui::DragInt(id.c_str(), &value, 1.f, min, max))
		{
			if (value > max && useMinMax)
			{
				value = max;
			}

			if (value < min && useMinMax)
			{
				value = min;
			}

			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, bool& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);

		if (ImGui::Checkbox(id.c_str(), &value))
		{
			changed = true;
		}

		return changed;
	}

	static bool Property(const std::string& text, float& value, bool useMinMax = false, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::DragFloat(id.c_str(), &value, 1.f, min, max))
		{
			if (value < min && useMinMax)
			{
				value = min;
			}

			if (value > max && useMinMax)
			{
				value = max;
			}

			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, glm::vec2& value, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

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

	static bool Property(const std::string& text, glm::vec3& value, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

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

	static bool Property(const std::string& text, glm::vec4& value, float min = 0.f, float max = 0.f)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

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

	static bool Property(const std::string& text, const std::string& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (InputText(id, const_cast<std::string&>(value)))
		{
			changed = true;
		}

		ImGui::PopItemWidth();

		return changed;
	}

	static bool Property(const std::string& text, std::string& value)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (InputText(id, value))
		{
			changed = true;
		}

		return changed;
	}

	static bool PropertyColor(const std::string& text, glm::vec4& value)
	{
		ImGui::NextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value)))
		{
			return true;
		}

		return false;
	}

	static bool PropertyColor(const std::string& text, glm::vec3& value)
	{
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string id = "##" + std::to_string(s_stackId++);
		ImGui::PushItemWidth(ImGui::GetColumnWidth());

		if (ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value)))
		{
			return true;
		}

		return false;
	}

	static bool Property(const std::string& text, std::filesystem::path& path)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		std::string sPath = path.string();
		ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::CalcTextSize("Open...").x - 20.f);

		std::string id = "##" + std::to_string(s_stackId++);
		if (InputText(id, sPath))
		{
			path = std::filesystem::path(sPath);
			changed = true;
		}

		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (ImGui::Button("Open...", { ImGui::GetContentRegionAvail().x, 25.f }))
		{
			auto newPath = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
			path = newPath;
			changed = true;
		}

		if (auto ptr = UI::DragDropTarget("CONTENT_BROWSER_ITEM"))
		{
			const wchar_t* inPath = (const wchar_t*)ptr;
			std::filesystem::path newPath = std::filesystem::path("assets") / inPath;

			path = newPath;
			changed = true;
		}

		return changed;
	}

	static bool Property(const std::string& text, Ref<Lamp::Asset>& asset)
	{
		bool changed = false;

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(text.c_str());

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetColumnWidth() - 20.f);
		ImGui::Text("Asset: %s", asset->Path.filename().string().c_str());

		ImGui::PopItemWidth();

		ImGui::SameLine();
		std::string buttonId = "Open##" + std::to_string(s_stackId++);
		if (ImGui::Button(buttonId.c_str(), { ImGui::GetContentRegionAvail().x, 25.f }))
		{
		}
		if (BeginPopup())
		{
			ImGui::Text("Test");

			EndPopup();
		}

		if (auto ptr = UI::DragDropTarget("CONTENT_BROWSER_ITEM"))
		{
			const wchar_t* inPath = (const wchar_t*)ptr;
			std::filesystem::path newPath = std::filesystem::path("assets") / inPath;

			changed = true;
		}

		return changed;
	}
};
