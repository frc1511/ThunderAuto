#include <ThunderAuto/Pages/TrajectoryManagerPage.hpp>

#include <IconsLucide.h>
#include <imgui_raii.h>

void TrajectoryManagerPage::present(bool* running) {
  m_event = Event::NONE;

  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(TRAJECTORY_MANAGER_PAGE_START_WIDTH),
                                  GET_UISIZE(TRAJECTORY_MANAGER_PAGE_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || (running && !*running))
    return;

  ThunderAutoProjectState state = m_history.currentState();

  const bool isInTrajectoryMode = state.editorState.view == ThunderAutoEditorState::View::TRAJECTORY;
  ThunderAutoTrajectoryEditorState& trajectoryEditorState = state.editorState.trajectoryEditorState;

  std::map<std::string, ThunderAutoTrajectorySkeleton>& trajectories = state.trajectories;

  std::string trajectoryToDeleteName;

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
    }

    if (auto popup = ImGui::Scoped::PopupContextItem()) {
      if (ImGui::MenuItem(ICON_LC_PENCIL "  Rename")) {
        m_eventTrajectory = trajectoryName;
        m_event = Event::RENAME_TRAJECTORY;
      }

      if (ImGui::MenuItem(ICON_LC_LINK "  Link Start/End Behavior")) {
        m_eventTrajectory = trajectoryName;
        m_event = Event::LINK_END_BEHAVIOR;
      }

      {
        auto scopedIndent = ImGui::Scoped::Indent();

      if (trajectorySkeleton.hasStartBehaviorLink()) {
        if (ImGui::MenuItem(ICON_LC_UNLINK "  Unlink Start Behavior")) {
          trajectorySkeleton.clearStartBehaviorLink();
          m_history.addState(state);
        }
      }

      if (trajectorySkeleton.hasEndBehaviorLink()) {
        if (ImGui::MenuItem(ICON_LC_UNLINK "  Unlink End Behavior")) {
          trajectorySkeleton.clearEndBehaviorLink();
          m_history.addState(state);
        }
      }

      }

      if (ImGui::MenuItem(ICON_LC_ARROW_RIGHT_LEFT "  Reverse Direction")) {
        trajectorySkeleton.reverseDirection();
        m_history.addState(state);
      }

      if (ImGui::MenuItem(ICON_LC_COPY "  Duplicate")) {
        m_eventTrajectory = trajectoryName;
        m_event = Event::DUPLICATE_TRAJECTORY;
      }

      if (ImGui::MenuItem(ICON_LC_TRASH "  Delete")) {
        trajectoryToDeleteName = trajectoryName;
      }
    }

    if (auto scopedDragSource = ImGui::Scoped::DragDropSource()) {
      ImGui::SetDragDropPayload("Trajectory", trajectoryName.c_str(), trajectoryName.size() + 1);
      ImGui::Text("%s", trajectoryName.c_str());
    }
  }

  ImGui::Separator();

  if (ImGui::Button("+ New Trajectory", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
    m_event = Event::NEW_TRAJECTORY;
  }

  if (!trajectoryToDeleteName.empty()) {
    state.trajectoryDelete(trajectoryToDeleteName);
    m_history.addState(state);
  }
}
