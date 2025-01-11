#pragma once

#include <ThunderAuto/angle.h>
#include <ThunderAuto/thunder_auto.h>

//
// Disables ImGui elements for the lifetime of this object.
//
class ImGuiScopedDisabled {
  bool m_disabled = false;

public:
  inline ImGuiScopedDisabled(bool disabled = true)
    : m_disabled(disabled) {

    if (m_disabled) {
      ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
  }

  inline ~ImGuiScopedDisabled() {
    if (m_disabled) {
      ImGui::PopItemFlag();
      ImGui::PopStyleVar();
    }
  }
};

//
// Creates two columns. This object presents the leftmost as text with a given
// name. The rightmost field is presented by the user during the rest of this
// object's lifetime.
//
class ImGuiScopedField {
public:
  inline ImGuiScopedField(const char* name, unsigned int column_width)
    : ImGuiScopedField(name, name, column_width) {}

  inline ImGuiScopedField(const char* id, const char* text,
                          unsigned int column_width)
    : ImGuiScopedField(id, column_width,
                       [text]() { ImGui::Text("%s", text); }) {}

  inline ImGuiScopedField(const char* id, unsigned int column_width,
                          std::function<void()> make_left_column) {
    ImGui::PushID(id);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, column_width);

    make_left_column();

    ImGui::NextColumn();
  }

  inline ~ImGuiScopedField() {
    ImGui::Columns(1);
    ImGui::PopID();
  }
};

inline float pt_distance(ImVec2 a, ImVec2 b) {
  return std::hypot(a.x - b.x, a.y - b.y);
}

inline ImVec2 pt_extend_at_angle(const ImVec2& pt, const Angle& angle,
                                 float distance) {

  const float dx = std::cos(angle.radians()) * distance;
  const float dy = std::sin(angle.radians()) * distance;

  return pt + ImVec2(dx, dy);
}

