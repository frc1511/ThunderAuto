#include <ThunderAuto/Pages/TrajectoryManagerPage.hpp>

#include <ThunderAuto/FontLibrary.hpp>

#include <IconsFontAwesome5.h>
#include <imgui_raii.h>

void TrajectoryManagerPage::present(bool* running) {
  m_event = Event::NONE;

  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(TRAJECTORY_MANAGER_PAGE_START_WIDTH),
                                  GET_UISIZE(TRAJECTORY_MANAGER_PAGE_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || !*running)
    return;

  ThunderAutoProjectState state = m_history.currentState();

  const bool isInTrajectoryMode = state.editorState.view == ThunderAutoEditorState::View::TRAJECTORY;
  ThunderAutoTrajectoryEditorState& trajectoryEditorState = state.editorState.trajectoryEditorState;

  std::map<std::string, ThunderAutoTrajectorySkeleton>& trajectories = state.trajectories;

  std::string trajectoryNameToDelete;

  for (auto& [trajectoryName, trajectorySkeleton] : trajectories) {
    const bool isTrajectorySelected =
        isInTrajectoryMode && (trajectoryName == trajectoryEditorState.currentTrajectoryName);

    auto scopedPadding =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

    if (ImGui::Selectable(trajectoryName.c_str(), isTrajectorySelected) && !isTrajectorySelected) {
      ThunderAutoLogger::Info("Trajectory '{}' selected", trajectoryName);

      state.editorState.view = ThunderAutoEditorState::View::TRAJECTORY;
      trajectoryEditorState.currentTrajectoryName = trajectoryName;
      trajectoryEditorState.trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::NONE;
      trajectoryEditorState.selectionIndex = 0;

      m_history.addState(state);

      m_editorPage.invalidateCachedTrajectory();
      m_editorPage.resetPlayback();
    }

    if (auto popup = ImGui::Scoped::PopupContextItem()) {
      if (ImGui::MenuItem(ICON_FA_PENCIL_ALT "  Rename")) {
        m_eventTrajectory = trajectoryName;
        m_event = Event::RENAME_TRAJECTORY;
      }

      if (ImGui::MenuItem("\xef\x8d\xa3  Reverse Direction")) {
        trajectorySkeleton.reverseDirection();
        m_history.addState(state);
        m_editorPage.invalidateCachedTrajectory();
        m_editorPage.resetPlayback();
      }

      if (ImGui::MenuItem(ICON_FA_COPY "  Duplicate")) {
        m_eventTrajectory = trajectoryName;
        m_event = Event::DUPLICATE_TRAJECTORY;
      }

      if (ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete")) {
        trajectoryNameToDelete = trajectoryName;
      }
    }
  }

  ImGui::Separator();

  if (ImGui::Button("+ New Trajectory", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
    m_event = Event::NEW_TRAJECTORY;
  }

  if (!trajectoryNameToDelete.empty()) {
    state.trajectoryDelete(trajectoryNameToDelete);
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();

    m_history.addState(state);
  }
}
