#include <ThunderAuto/pages/settings_page.hpp>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.hpp>

#define COLUMN_WIDTH 135.0f

void SettingsPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(SETTINGS_PAGE_START_WIDTH),
                                  GET_UISIZE(SETTINGS_PAGE_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(name(), running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }

  ProjectSettings& settings = m_document_manager.settings();

  // Robot Length.
  {
    ImGuiScopedField field = ImGuiScopedField::Builder(ICON_FA_RULER_VERTICAL "  Robot Length").build();

    if (ImGui::DragFloat("##Robot Length", &settings.robot_length, 0.1f, 0.0f,
                         0.0f, "%.2f m")) {
      m_document_manager.history()->mark_unsaved();
    }

    if (settings.robot_length < 0.0f) settings.robot_length = 0.0f;
  }

  // Robot Width.
  {
    ImGuiScopedField field = ImGuiScopedField::Builder(ICON_FA_RULER_HORIZONTAL "  Robot Width").build();

    if (ImGui::DragFloat("##Robot Width", &settings.robot_width, 0.1f, 0.0f,
                         0.0f, "%.2f m")) {
      m_document_manager.history()->mark_unsaved();
    }

    if (settings.robot_width < 0.0f) settings.robot_width = 0.0f;
  }

  ImGui::End();
}

