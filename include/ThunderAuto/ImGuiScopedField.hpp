#pragma once

#include <ThunderAuto/UISizes.hpp>
#include <imgui.h>
#include <functional>

namespace ImGui {

/**
 * Creates two columns. This object presents the leftmost as text with a given name. The rightmost field is
 * presented by the user during the rest of this object's lifetime.
 */
class ScopedField final {
 public:
  class Builder final {
   public:
    using LeftColumnBuildFunc = std::function<void()>;

   private:
    const char* m_id = nullptr;
    const char* m_text = nullptr;
    const char* m_tooltip = nullptr;
    float m_leftColumnWidth = -1;
    LeftColumnBuildFunc m_leftColumnBuildFunc;

   public:
    explicit Builder(const char* id) : m_id(id), m_text(id) {}

    ScopedField build() {
      LeftColumnBuildFunc buildLeft = m_leftColumnBuildFunc;
      if (!buildLeft) {
        buildLeft = [&]() {
          ImGui::Text("%s", m_text);
          if (m_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", m_tooltip);
          }
        };
      }

      float leftWidth = m_leftColumnWidth;
      if (leftWidth < 0.f) {
        leftWidth = GET_UISIZE(FIELD_NORMAL_LEFT_COLUMN_WIDTH);
      }

      return ScopedField(m_id, leftWidth, buildLeft);
    }

    Builder& name(const char* name) {
      m_text = name;
      return *this;
    }

    Builder& tooltip(const char* tooltip) {
      m_tooltip = tooltip;
      return *this;
    }

    Builder& leftColumnWidth(float width) {
      assert(width > 0.f);
      m_leftColumnWidth = width;
      return *this;
    }

    Builder& customLeftColumn(std::function<void()> buildFunc) {
      m_leftColumnBuildFunc = buildFunc;
      return *this;
    }
  };

 private:
  ScopedField(const char* id, float columnWidth, std::function<void()> makeLeftColumn) {
    ImGui::PushID(id);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);

    makeLeftColumn();

    ImGui::NextColumn();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  }

 public:
  ~ScopedField() {
    ImGui::Columns(1);
    ImGui::PopID();
  }
};

}  // namespace ImGui
