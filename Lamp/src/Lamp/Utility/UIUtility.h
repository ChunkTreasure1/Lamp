#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <string>

class UI
{
public:
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

	static void ImageText(uint32_t texId, const std::string& text);
	static bool ImageTreeNodeEx(uint32_t texId, const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);
	static bool InputTextOnSameline(std::string& string, const std::string& id);

private:
	UI() = delete;
};
