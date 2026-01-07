#include <ThunderAuto/Popups/NewTrajectoryPopup.hpp>

#include <ThunderAuto/TrajectoryHelper.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <imgui.h>
#include <imgui_raii.h>

void NewTrajectoryPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(NEW_TRAJECTORY_POPUP_START_WIDTH), GET_UISIZE(NEW_TRAJECTORY_POPUP_START_HEIGHT)));

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

  // TODO: Add option to link start/end point to an existing/new link or to the start/end of an already
  // created trajectory

  if (ImGui::Button("Cancel")) {
    reset();
    *running = false;
  }

  ImGui::SameLine();

  const std::string trajectoryName = m_trajectoryNameBuffer;

  const bool isTrajectoryNameAlreadyTaken = trajectories.contains(trajectoryName);
  const bool isTrajectoryNameEmpty = trajectoryName.empty();

  const bool isCreateDisabled = isTrajectoryNameAlreadyTaken || isTrajectoryNameEmpty;

  auto createDisabled = ImGui::Scoped::Disabled(isCreateDisabled);

  if (ImGui::Button("Create")) {
    ThunderAutoLogger::Info("Creating new trajectory '{}'", trajectoryName);

    // Add trajectory

    state.trajectories.emplace(trajectoryName, kDefaultNewTrajectory);
    state.trajectorySelect(trajectoryName);

    // Apply changes

    m_history.addState(state);

    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();

    reset();
    *running = false;
  }

  // Tooltips

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
    if (isTrajectoryNameAlreadyTaken) {
      ImGui::SetTooltip("A trajectory with this name already exists.");
    } else if (isTrajectoryNameEmpty) {
      ImGui::SetTooltip("Trajectory name can not be empty.");
    }
  }
}
