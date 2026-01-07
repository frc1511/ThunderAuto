#include <ThunderAuto/Popups/DuplicateTrajectoryPopup.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void DuplicateTrajectoryPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(DUPLICATE_TRAJECTORY_POPUP_START_WIDTH),
                                  GET_UISIZE(DUPLICATE_TRAJECTORY_POPUP_START_HEIGHT)));

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  if (!scopedPopup || !*running) {
    return;
  }

  ThunderAutoProjectState state = m_history.currentState();
  std::map<std::string, ThunderAutoTrajectorySkeleton>& trajectories = state.trajectories;

  {
    auto scopedField = ImGui::ScopedField::Builder("Trajectory Name").build();

    ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
      // [a-zA-Z0-9_ ]
      if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
      }
      return 0;
    };

    ImGui::InputText("##Trajectory Name", m_trajectoryNameBuffer, sizeof(m_trajectoryNameBuffer),
                     ImGuiInputTextFlags_CallbackCharFilter, callback);
  }

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const std::string newTrajectoryName = m_trajectoryNameBuffer;

  const bool isNewTrajectoryNameAlreadyTaken = trajectories.contains(newTrajectoryName);
  const bool isNewTrajectoryNameEmpty = newTrajectoryName.empty();

  const bool isRenameDisabled = isNewTrajectoryNameAlreadyTaken || isNewTrajectoryNameEmpty;

  auto renameDisabled = ImGui::Scoped::Disabled(isRenameDisabled);

  if (ImGui::Button("Duplicate")) {
    ThunderAutoLogger::Info("Duplicate trajectory '{}' to '{}'", m_oldTrajectoryName, newTrajectoryName);

    state.trajectoryDuplicate(m_oldTrajectoryName, newTrajectoryName);
    m_history.addState(state);

    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();

    reset();
    *running = false;
  }

  // Tooltips

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
    if (isNewTrajectoryNameAlreadyTaken) {
      ImGui::SetTooltip("A trajectory with this name already exists.");
    } else if (isNewTrajectoryNameEmpty) {
      ImGui::SetTooltip("Trajectory name can not be empty.");
    }
  }
}
