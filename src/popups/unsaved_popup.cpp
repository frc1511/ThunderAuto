#include <ThunderAuto/popups/unsaved_popup.h>

void UnsavedPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false,
                          ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(125, 115));
  if (!ImGui::BeginPopupModal(m_name, nullptr,
                              ImGuiWindowFlags_AlwaysAutoResize)) {
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

  ImGui::EndPopup();

  if (m_result != Result::NONE) {
    *running = false;
  }
}
