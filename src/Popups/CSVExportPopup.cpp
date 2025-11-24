#include <ThunderAuto/Popups/CSVExportPopup.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void CSVExportPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(CSV_EXPORT_POPUP_START_WIDTH), GET_UISIZE(CSV_EXPORT_POPUP_START_HEIGHT)),
      ImGuiCond_FirstUseEver);

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_NoMove);
  if (!scopedPopup || !*running) {
    return;
  }

  ImGui::Text("%s", m_exportMessage.c_str());

  ImGui::Spacing();

  if (ImGui::Button("Ok") || ImGui::IsKeyPressed(ImGuiKey_Escape) ||
      ImGui::IsKeyPressed(ImGuiKey_Enter)) {
    *running = false;
  }
}
