#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
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

	class ScopedStyle
	{
	public:
		ScopedStyle(ImGuiStyleVar var, const glm::vec2& value)
		{
			auto& style = ImGui::GetStyle();
			ImGui::PushStyleVar(var, { value.x, value.y });
		}

		~ScopedStyle()
		{
			ImGui::PopStyleVar();
		}
	};

	static void ImageText(uint32_t texId, const std::string& text);
	static bool ImageTreeNodeEx(uint32_t texId, const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);
	static bool InputTextOnSameline(std::string& string, const std::string& id);

	//Inputs
	static bool Property(const std::string& text, int& value, int min = 0, int max = 0);
	static bool Property(const std::string& text, bool& value);
	static bool Property(const std::string& text, float& value);
	static bool Property(const std::string& text, glm::vec2& value, float min = 0.f, float max = 0.f);
	static bool Property(const std::string& text, glm::vec3& value, float min = 0.f, float max = 0.f);
	static bool Property(const std::string& text, glm::vec4& value, float min = 0.f, float max = 0.f);
	static bool Property(const std::string& text, std::string& value);
	static bool Property(const std::string& text, glm::vec4& value, bool useAlpha);
	static bool Property(const std::string& text, std::filesystem::path& path);

private:
	UI() = delete;
};
