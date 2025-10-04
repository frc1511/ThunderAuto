#include <ThunderAuto/Popups/OpenProjectErrorPopup.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void OpenProjectErrorPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(PROJECT_OPEN_ERROR_POPUP_START_WIDTH),
                                  GET_UISIZE(PROJECT_OPEN_ERROR_POPUP_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);

  ImGui::SetNextWindowSizeConstraints(ImVec2(0, GET_UISIZE(PROJECT_OPEN_ERROR_POPUP_START_HEIGHT)),
                                      ImVec2(INFINITY, GET_UISIZE(PROJECT_OPEN_ERROR_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_NoMove);
  if (!scopedPopup || !*running) {
    return;
  }

  ImVec2 regionAvail = ImGui::GetContentRegionAvail();
  ImVec2 textSize = ImVec2(regionAvail.x, ImGui::GetTextLineHeight() * 2.f);

  // We want the error message to be seletable so that it can easily be copied.
  // ImGui does not currently have a widget for this, so InputTextMultiline with ReadOnly flag will work.
  ImGui::InputTextMultiline("##ErrorText", const_cast<char*>(m_error.c_str()), m_error.size() + 1, textSize,
                            ImGuiInputTextFlags_ReadOnly);

  ImGui::Spacing();

  ImVec2 buttonSize = ImVec2(regionAvail.x, 0.f);

  if (ImGui::Button("Close", buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape) ||
      ImGui::IsKeyPressed(ImGuiKey_Enter)) {
    *running = false;
  }
}
