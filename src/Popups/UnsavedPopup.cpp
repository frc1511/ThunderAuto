#include <ThunderAuto/Popups/UnsavedPopup.hpp>

#include <imgui.h>
#include <imgui_raii.h>

void UnsavedPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(UNSAVED_POPUP_START_WIDTH), GET_UISIZE(UNSAVED_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  m_result = Result::NONE;

  const ImVec2 button_size = ImVec2(ImGui::GetContentRegionAvail().x, 0.f);

  if (ImGui::Button("Yes", button_size)) {
    m_result = Result::SAVE;
  }
  if (ImGui::Button("No", button_size)) {
    m_result = Result::DONT_SAVE;
  }

  ImGui::Separator();

  if (ImGui::Button("Cancel", button_size)) {
    m_result = Result::CANCEL;
  }

  if (m_result != Result::NONE) {
    *running = false;
  }
}
