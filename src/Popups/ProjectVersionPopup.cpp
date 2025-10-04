#include <ThunderAuto/Popups/ProjectVersionPopup.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void ProjectVersionPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(PROJECT_VERSION_POPUP_START_WIDTH), -1.f));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_NoMove);
  if (!scopedPopup || !*running) {
    m_result = Result::CANCEL;
    return;
  }

  m_result = Result::NONE;

  bool majorOlder = m_projectVersion.major < THUNDERAUTO_PROJECT_VERSION_MAJOR;
  bool minorNewer = (!majorOlder) && (m_projectVersion.minor > THUNDERAUTO_PROJECT_VERSION_MINOR);

  const char* comparisonString = minorNewer ? "newer" : "older";

  ImGui::Text("Project Version (%d.%d) is %s than current ThunderAuto project version (%d.%d)",
              m_projectVersion.major, m_projectVersion.minor, comparisonString,
              THUNDERAUTO_PROJECT_VERSION_MAJOR, THUNDERAUTO_PROJECT_VERSION_MINOR);

  if (minorNewer) {
    ImGui::TextWrapped(
        "This project's minor version is newer than current ThunderAuto version."
        "Newer minor versions may introduce new features or change the save format, although any changes "
        "should be backwards compatible.");

    {
      auto scopedColor = ImGui::Scoped::StyleColor(ImGuiCol_Text, ImVec4(1, 0.5, 0, 1));
      ImGui::TextWrapped(
          "It is still recommended to update ThunderAuto to the latest version to ensure compatibility, and "
          "to not loose any project data when saving.");
    }
  } else {  // Older
    ImGui::TextWrapped(
        "Upon saving, this project will be updated to the current ThunderAuto project version. This may "
        "break compatibility with older ThunderAuto versions.");
  }

  ImGui::Spacing();
  ImGui::Spacing();

  const ImGuiStyle& style = ImGui::GetStyle();

  ImVec2 regionAvail = ImGui::GetContentRegionAvail();
  ImVec2 buttonSize((regionAvail.x - style.ItemSpacing.x) / 2.f, 0.f);

  if (ImGui::Button("Close", buttonSize)) {
    m_result = Result::CANCEL;
  }

  ImGui::SameLine();

  if (ImGui::Button("Ok", buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape) ||
      ImGui::IsKeyPressed(ImGuiKey_Enter)) {
    m_result = Result::OK;
  }

  if (m_result != Result::NONE) {
    *running = false;
  }
}
