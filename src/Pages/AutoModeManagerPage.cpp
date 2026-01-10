#include <ThunderAuto/Pages/AutoModeManagerPage.hpp>
#include <ThunderAuto/ColorPalette.hpp>
#include <IconsLucide.h>
#include <imgui_raii.h>

void AutoModeManagerPage::present(bool* running) {
  m_event = Event::NONE;

  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(AUTO_MODE_MANAGER_PAGE_START_WIDTH), GET_UISIZE(AUTO_MODE_MANAGER_PAGE_START_HEIGHT)),
      ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || (running && !*running))
    return;

  ThunderAutoProjectState state = m_history.currentState();

  const bool isInAutoModeState = state.editorState.view == ThunderAutoEditorState::View::AUTO_MODE;
  ThunderAutoModeEditorState& autoModeEditorState = state.editorState.autoModeEditorState;

  std::map<std::string, ThunderAutoMode>& autoModes = state.autoModes;

  std::string autoModeToDeleteName;

  for (auto& [autoModeName, autoMode] : autoModes) {
    const bool isAutoModeSelected =
        isInAutoModeState && (autoModeName == autoModeEditorState.currentAutoModeName);

    // TODO: Only compute once for both properties page and auto mode manager page.
    ThunderAutoModeStepTrajectoryBehavior trajectoryBehavior =
        autoMode.getTrajectoryBehavior(state.trajectories);
    if (trajectoryBehavior.errorInfo) {
      ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)ThunderAutoColorPalette::kYellow);
    }

    auto scopedPadding =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

    if (ImGui::Selectable(autoModeName.c_str(), isAutoModeSelected, ImGuiSelectableFlags_AllowOverlap) &&
        !isAutoModeSelected) {
      ThunderAutoLogger::Info("Auto Mode '{}' selected", autoModeName);

      state.editorState.view = ThunderAutoEditorState::View::AUTO_MODE;
      autoModeEditorState.currentAutoModeName = autoModeName;
      autoModeEditorState.selectedStepPath = std::nullopt;

      m_history.addState(state);
    }

    if (trajectoryBehavior.errorInfo) {
      ImGui::PopStyleColor();

      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        auto scopedTooltip = ImGui::Scoped::Tooltip();
        if (trajectoryBehavior.errorInfo.isTrajectoryMissing) {
          ImGui::Text(ICON_LC_TRIANGLE_ALERT
                      " Contains one or more references to\nnon-existent trajectories");
        }
        if (trajectoryBehavior.errorInfo.containsNonContinuousSequence) {
          ImGui::Text(ICON_LC_TRIANGLE_ALERT
                      " Contains one or more sequences of\nnon-continuous trajectory steps");
        }
      }
    }

    if (auto popup = ImGui::Scoped::PopupContextItem()) {
      if (ImGui::MenuItem(ICON_LC_PENCIL "  Rename")) {
        m_eventAutoMode = autoModeName;
        m_event = Event::RENAME_AUTO_MODE;
      }

      if (ImGui::MenuItem(ICON_LC_COPY "  Duplicate")) {
        m_eventAutoMode = autoModeName;
        m_event = Event::DUPLICATE_AUTO_MODE;
      }

      if (ImGui::MenuItem(ICON_LC_TRASH "  Delete")) {
        autoModeToDeleteName = autoModeName;
      }
    }

    if (auto scopedDragSource = ImGui::Scoped::DragDropSource()) {
      ImGui::SetDragDropPayload("Auto Mode", autoModeName.c_str(), autoModeName.size() + 1);
      ImGui::Text("%s", autoModeName.c_str());
    }
  }

  ImGui::Separator();

  if (ImGui::Button("+ New Auto Mode", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
    m_event = Event::NEW_AUTO_MODE;
  }

  if (!autoModeToDeleteName.empty()) {
    state.autoModeDelete(autoModeToDeleteName);
    m_history.addState(state);
  }
}
