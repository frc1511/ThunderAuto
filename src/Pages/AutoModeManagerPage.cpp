#include <ThunderAuto/Pages/AutoModeManagerPage.hpp>

#include <IconsLucide.h>
#include <imgui_raii.h>

void AutoModeManagerPage::present(bool* running) {
  m_event = Event::NONE;

  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(AUTO_MODE_MANAGER_PAGE_START_WIDTH),
                                  GET_UISIZE(AUTO_MODE_MANAGER_PAGE_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || !*running)
    return;

  ThunderAutoProjectState state = m_history.currentState();

  const bool isInAutoModeState = state.editorState.view == ThunderAutoEditorState::View::AUTO_MODE;
  ThunderAutoModeEditorState& autoModeEditorState = state.editorState.autoModeEditorState;

  std::map<std::string, ThunderAutoMode>& autoModes = state.autoModes;

  std::string autoModeToDeleteName;

  for (auto& [autoModeName, autoMode] : autoModes) {
    const bool isAutoModeSelected = 
        isInAutoModeState && (autoModeName == autoModeEditorState.currentAutoModeName);

    auto scopedPadding =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

    if (ImGui::Selectable(autoModeName.c_str(), isAutoModeSelected) && !isAutoModeSelected) {
      ThunderAutoLogger::Info("Auto Mode '{}' selected", autoModeName);

      state.editorState.view = ThunderAutoEditorState::View::AUTO_MODE;
      autoModeEditorState.currentAutoModeName = autoModeName;
      autoModeEditorState.selectedStepPath = std::nullopt;

      m_history.addState(state);

      m_editorPage.invalidateCachedTrajectory();
      m_editorPage.resetPlayback();
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
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();

    m_history.addState(state);
  }
}