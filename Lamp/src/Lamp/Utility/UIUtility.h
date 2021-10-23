#pragma once

#include "Lamp/Utility/PlatformUtility.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

namespace UI {
static int s_contextId = 0;
static uint32_t s_stackId = 0;

class ScopedColor {
public:
  ScopedColor(ImGuiCol_ color, const glm::vec4 &newColor) : m_Color(color) {
    auto &colors = ImGui::GetStyle().Colors;
    m_OldColor = colors[color];
    colors[color] = ImVec4{newColor.x, newColor.y, newColor.z, newColor.w};
  }

  ~ScopedColor() {
    auto &colors = ImGui::GetStyle().Colors;
    colors[m_Color] = m_OldColor;
  }

private:
  ImVec4 m_OldColor;
  ImGuiCol_ m_Color;
};

class ScopedStyleFloat {
public:
  ScopedStyleFloat(ImGuiStyleVar_ var, float value) {
    ImGui::PushStyleVar(var, value);
  }

  ~ScopedStyleFloat() { ImGui::PopStyleVar(); }
};

class ScopedStyleFloat2 {
public:
  ScopedStyleFloat2(ImGuiStyleVar_ var, const glm::vec2 &value) {
    ImGui::PushStyleVar(var, {value.x, value.y});
  }

  ~ScopedStyleFloat2() { ImGui::PopStyleVar(); }
};

static void ImageText(uint32_t texId, const std::string &text) {
  ImVec2 size = ImGui::CalcTextSize(text.c_str());
  ImGui::Image((ImTextureID)texId, {size.y, size.y});
  ImGui::SameLine();
  ImGui::Text(text.c_str());
}

static bool ImageTreeNode(uint32_t texId, const void *ptr_id,
                          ImGuiTreeNodeFlags flags, const char *fmt, ...) {
  ScopedStyleFloat2 frame{ImGuiStyleVar_FramePadding, {0.f, 0.f}};
  ScopedStyleFloat2 spacing{ImGuiStyleVar_ItemSpacing, {0.f, 0.f}};

  ImVec2 size = ImGui::CalcTextSize(fmt);
  ImGui::Image((ImTextureID)texId, {size.y, size.y}, {0, 1}, {1, 0});
  ImGui::SameLine();
  return ImGui::TreeNodeEx(ptr_id, flags, fmt);
}

static bool ImageSelectable(uint32_t texId, const std::string &text,
                            bool selected) {
  ImVec2 size = ImGui::CalcTextSize(text.c_str());
  ImGui::Image((ImTextureID)texId, {size.y, size.y}, {0, 1}, {1, 0});
  ImGui::SameLine();
  return ImGui::Selectable(text.c_str(), selected,
                           ImGuiSelectableFlags_SpanAvailWidth);
}

static bool TreeNodeFramed(const std::string &text, bool useOther = false,
                           float rounding = 0.f,
                           const glm::vec2 &padding = {0.f, 0.f}) {
  const ImGuiTreeNodeFlags nodeFlags =
      ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

  if (!useOther) {
    return ImGui::TreeNodeEx(text.c_str(), nodeFlags);
  } else {
    UI::ScopedStyleFloat frameRound(ImGuiStyleVar_FrameRounding, rounding);

    return ImGui::TreeNodeEx(text.c_str(), nodeFlags);
  }
}

static void TreeNodePop() { ImGui::TreePop(); }

bool InputTextOnSameline(std::string &string, const std::string &id);

static void Separator(ImGuiSeparatorFlags customFlags = 0) {
  ImGuiContext &g = *GImGui;
  ImGuiWindow *window = g.CurrentWindow;
  if (window->SkipItems)
    return;

  // Those flags should eventually be overridable by the user
  ImGuiSeparatorFlags flags =
      (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
          ? ImGuiSeparatorFlags_Vertical
          : ImGuiSeparatorFlags_Horizontal;
  flags |= customFlags;
  ImGui::SeparatorEx(flags);
}

static void PushId() {
  ImGui::PushID(s_contextId++);
  s_stackId = 0;
}

static void PopId() {
  ImGui::PopID();
  s_contextId--;
}

static void SameLine(float offsetX = 0.f, float spacing = -1.f) {
  ImGui::SameLine(offsetX, spacing);
}

static bool BeginProperties(const std::string &name = "",
                            bool showHeader = false) {
  ImGuiTableFlags flags;
  if (showHeader) {
    ImGui::TableHeader(name.c_str());
  }
  PushId();
  return ImGui::BeginTable(name.c_str(), 2,
                           ImGuiTableFlags_BordersInnerV |
                               ImGuiTableFlags_SizingStretchProp |
                               ImGuiTableFlags_Resizable);
}

static void EndProperties() {
  ImGui::EndTable();
  PopId();
}

static void ShiftCursor(float x, float y) {
  ImVec2 pos = {ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y};
  ImGui::SetCursorPos(pos);
}

// Inputs
static bool PropertyAxisColor(const std::string &text, glm::vec3 &value,
                              float resetValue = 0.f) {
  ScopedStyleFloat2 cellPad(ImGuiStyleVar_CellPadding, {4.f, 0.f});

  bool changed = false;

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0.f, 0.f});

  float lineHeight =
      GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
  ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};

  {
    ScopedColor color{ImGuiCol_Button, {0.8f, 0.1f, 0.15f, 1.f}};
    ScopedColor colorh{ImGuiCol_ButtonHovered, {0.9f, 0.2f, 0.2f, 1.f}};
    ScopedColor colora{ImGuiCol_ButtonActive, {0.8f, 0.1f, 0.15f, 1.f}};

    std::string butId = "X##" + std::to_string(s_stackId++);
    if (ImGui::Button(butId.c_str(), buttonSize)) {
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
    ScopedColor color{ImGuiCol_Button, {0.2f, 0.7f, 0.2f, 1.f}};
    ScopedColor colorh{ImGuiCol_ButtonHovered, {0.3f, 0.8f, 0.3f, 1.f}};
    ScopedColor colora{ImGuiCol_ButtonActive, {0.2f, 0.7f, 0.2f, 1.f}};

    std::string butId = "Y##" + std::to_string(s_stackId++);
    if (ImGui::Button(butId.c_str(), buttonSize)) {
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
    ScopedColor color{ImGuiCol_Button, {0.1f, 0.25f, 0.8f, 1.f}};
    ScopedColor colorh{ImGuiCol_ButtonHovered, {0.2f, 0.35f, 0.9f, 1.f}};
    ScopedColor colora{ImGuiCol_ButtonActive, {0.1f, 0.25f, 0.8f, 1.f}};

    std::string butId = "Z##" + std::to_string(s_stackId++);
    if (ImGui::Button(butId.c_str(), buttonSize)) {
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

static bool Property(const std::string &text, int &value, int min = 0,
                     int max = 0) {
  bool changed = false;

  ScopedStyleFloat2 cellPad{ImGuiStyleVar_CellPadding, {4.f, 0.f}};

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  std::string id = "##" + std::to_string(s_stackId++);
  ImGui::PushItemWidth(ImGui::GetColumnWidth());
  if (ImGui::DragInt(id.c_str(), &value, 1.f, min, max)) {
    changed = true;
  }

  ImGui::PopItemWidth();

  return changed;
}

static bool Property(const std::string &text, bool &value) {
  bool changed = false;

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  std::string id = "##" + std::to_string(s_stackId++);

  if (ImGui::Checkbox(id.c_str(), &value)) {
    changed = true;
  }

  return changed;
}

static bool Property(const std::string &text, float &value) {
  bool changed = false;

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  std::string id = "##" + std::to_string(s_stackId++);
  ImGui::PushItemWidth(ImGui::GetColumnWidth());

  if (ImGui::DragFloat(id.c_str(), &value, 1.f)) {
    changed = true;
  }

  ImGui::PopItemWidth();

  return changed;
}

static bool Property(const std::string &text, glm::vec2 &value, float min = 0.f,
                     float max = 0.f) {
  bool changed = false;

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  std::string id = "##" + std::to_string(s_stackId++);
  ImGui::PushItemWidth(ImGui::GetColumnWidth());

  if (ImGui::DragFloat2(id.c_str(), glm::value_ptr(value), 1.f, min, max)) {
    changed = true;
  }

  ImGui::PopItemWidth();

  return changed;
}

static bool Property(const std::string &text, glm::vec3 &value, float min = 0.f,
                     float max = 0.f) {
  bool changed = false;

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  std::string id = "##" + std::to_string(s_stackId++);
  ImGui::PushItemWidth(ImGui::GetColumnWidth());

  if (ImGui::DragFloat3(id.c_str(), glm::value_ptr(value), 1.f, min, max)) {
    changed = true;
  }

  ImGui::PopItemWidth();

  return changed;
}

static bool Property(const std::string &text, glm::vec4 &value, float min = 0.f,
                     float max = 0.f) {
  bool changed = false;

  ImGui::TableNextColumn();
  ImGui::Text(text.c_str());

  ImGui::TableNextColumn();
  std::string id = "##" + std::to_string(s_stackId++);
  ImGui::PushItemWidth(ImGui::GetColumnWidth());

  if (ImGui::DragFloat4(id.c_str(), glm::value_ptr(value), 1.f, min, max)) {
    changed = true;
  }

  ImGui::PopItemWidth();

  return changed;
}

bool Property(const std::string &text, std::string &value);

static bool Property(const std::string &text, glm::vec4 &value, bool useAlpha) {
  if (useAlpha) {
    if (ImGui::ColorEdit4(text.c_str(), glm::value_ptr(value))) {
      return true;
    }
  } else {
    float values[3] = {value.x, value.y, value.z};
    if (ImGui::ColorEdit3(text.c_str(), values)) {
      value = glm::vec4({values[0], values[1], values[2], 1.f});
    }
  }

  return false;
}

bool Property(const std::string &text, std::filesystem::path &path);
}; // namespace UI
