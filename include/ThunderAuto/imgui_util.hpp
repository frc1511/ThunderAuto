#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/angle.hpp>
#include <ThunderAuto/ui_sizes.hpp>

//
// Disables ImGui elements for the lifetime of this object.
//
class ImGuiScopedDisabled {
  bool m_disabled = false;

 public:
  inline ImGuiScopedDisabled(bool disabled = true) : m_disabled(disabled) {
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
class ImGuiScopedField final {
  const char* m_tooltip = nullptr;

 public:
  class Builder final {
   public:
    using LeftColumnBuildFunc = std::function<void()>;

   private:
    const char* m_id = nullptr;
    const char* m_text = nullptr;
    const char* m_tooltip = nullptr;
    float m_left_column_width = -1;
    LeftColumnBuildFunc m_left_column_build_func;

   public:
    Builder(const char* id) : m_id(id), m_text(id) {}

    ImGuiScopedField build() {
      LeftColumnBuildFunc build_left = m_left_column_build_func;
      if (!build_left) {
        build_left = [&]() {
          ImGui::Text("%s", m_text);
          if (m_tooltip && ImGui::IsItemHovered()) {
            ImGui::SetTooltip(m_tooltip);
          }
        };
      }

      float left_width = m_left_column_width;
      if (left_width < 0.f) {
        left_width = GET_UISIZE(FIELD_NORMAL_LEFT_COLUMN_WIDTH);
      }

      return ImGuiScopedField(m_id, left_width, build_left);
    }

    Builder& name(const char* name) {
      m_text = name;
      return *this;
    }

    Builder& tooltip(const char* tooltip) {
      m_tooltip = tooltip;
      return *this;
    }

    Builder& left_column_width(float width) {
      assert(width > 0.f);
      m_left_column_width = width;
      return *this;
    }

    Builder& custom_left_column(std::function<void()> build_func) {
      m_left_column_build_func = build_func;
      return *this;
    }
  };

 private:
  ImGuiScopedField(const char* id,
                   float column_width,
                   std::function<void()> make_left_column) {
    ImGui::PushID(id);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, column_width);

    make_left_column();

    ImGui::NextColumn();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  }

 public:
  ~ImGuiScopedField() {
    ImGui::Columns(1);
    ImGui::PopID();
  }
};

inline float pt_distance(ImVec2 a, ImVec2 b) {
  return std::hypot(a.x - b.x, a.y - b.y);
}

inline ImVec2 pt_extend_at_angle(const ImVec2& pt,
                                 const Angle& angle,
                                 float distance) {
  const float dx = std::cos(angle.radians()) * distance;
  const float dy = std::sin(angle.radians()) * distance;

  return pt + ImVec2(dx, dy);
}
