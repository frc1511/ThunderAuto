#include <ThunderAuto/App.hpp>

#include <ThunderAuto/Platform/Platform.hpp>
#include <ThunderAuto/Graphics/Graphics.hpp>
#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/Error.hpp>
#include <ThunderAuto/Input.hpp>
#include <IconsLucide.h>
#include <imgui.h>
#include <imgui_raii.h>
#include <imgui_internal.h>

void App::setupDockspace(ImGuiID dockspaceID) {
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  bool dockspaceCreated = ImGui::DockBuilderGetNode(dockspaceID) != nullptr;

  ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

  if (!dockspaceCreated || m_resetDockspace) {
    m_resetDockspace = false;

    ThunderAutoLogger::Info("Creating dockspace layout");

    ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

    ImGuiID dockspaceIDRightUp =
        ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.3f, nullptr, &dockspaceID);

    ImGuiID dockspaceIDRightDown =
        ImGui::DockBuilderSplitNode(dockspaceIDRightUp, ImGuiDir_Down, 0.22f, nullptr, &dockspaceIDRightUp);

    ImGuiID dockspaceIDLeftUp =
        ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Left, 0.2f, nullptr, &dockspaceID);

    ImGuiID dockspaceIDLeftDown =
        ImGui::DockBuilderSplitNode(dockspaceIDLeftUp, ImGuiDir_Down, 0.5f, nullptr, &dockspaceIDLeftUp);

    ImGui::DockBuilderDockWindow(m_editorPage.name(), dockspaceID);
    ImGui::DockBuilderDockWindow(m_propertiesPage.name(), dockspaceIDRightUp);
    ImGui::DockBuilderDockWindow(m_actionsPage.name(), dockspaceIDRightDown);
    ImGui::DockBuilderDockWindow(m_trajectoryManagerPage.name(), dockspaceIDLeftUp);
    ImGui::DockBuilderDockWindow(m_autoModeManagerPage.name(), dockspaceIDLeftDown);
  }
}

void App::focusWasChanged(bool focused) {
  if (!focused) {
    // Auto export CSV Export
    bool autoCSVExport = m_documentManager.settings().autoCSVExport;
    if (autoCSVExport) {
      csvExportAllTrajectories();
    }
  }
}

void App::present() {
  presentMenuBar();

#ifdef THUNDERAUTO_DEBUG
  if (m_showImGuiDemoWindow) {
    ImGui::ShowDemoWindow(&m_showImGuiDemoWindow);
  }
#endif

  const ThunderAutoProjectSettings& settings = m_documentManager.settings();

  switch (m_eventState) {
    using enum EventState;
    case PROJECT:
      if (m_documentManager.isOpen()) {
        presentProjectEventPopups();
        presentProjectPages();
        break;
      }
      m_eventState = WELCOME;
      [[fallthrough]];
    case WELCOME:
      presentWelcomePopup();
      break;
    case NEW_PROJECT:
      presentNewProjectPopup();
      break;
    case NEW_FIELD:
      presentNewFieldPopup();
      break;
    case OPEN_PROJECT:
      openProject();
      break;
    case CLOSE_PROJECT:
      if (settings.autoCSVExport) {
        csvExportAllTrajectories();
      }
      m_documentManager.close();
      updateTitlebarTitle();
      m_eventState = WELCOME;
      break;
    case CLOSE_EVERYTHING:
      m_running = false;
      break;
    case OPEN_PROJECT_ERROR:
      presentOpenProjectErrorPopup();
      break;
    default:
      break;
  }

  bool isUnsaved = m_documentManager.isUnsaved();

  if (isUnsaved && m_documentManager.settings().autoSave) {
    save();
    isUnsaved = false;
  }

  if (isUnsaved != m_wasUnsaved) {
    updateTitlebarTitle();
  }

  m_wasUnsaved = isUnsaved;
}

void App::close() {
  if (tryChangeState(EventState::CLOSE_EVERYTHING)) {
    const ThunderAutoProjectSettings& settings = m_documentManager.settings();
    if (settings.autoCSVExport) {
      csvExportAllTrajectories();
    }

    m_documentManager.close();
  }
}

void App::dataClear() {
  m_recentProjects.clear();
}

bool App::dataShouldOpen(const char* name) {
  return strcmp(name, "RecentProjects") == 0;
}

void App::dataReadLine(const char* line) {
  if (!line || line[0] == '\0') {
    ThunderAutoLogger::Warn("Empty line in app save ini file, ignoring");
    return;
  }

  // Recent files should be stored oldest to newest, so just add them normally.

  std::filesystem::path path(line);
  if (!path.is_absolute()) {
    ThunderAutoLogger::Warn("Path '{}' in app save ini file is not absolute, ignoring", path.string());
    return;
  }
  if (!std::filesystem::exists(path)) {
    ThunderAutoLogger::Warn("Path '{}' in app save ini file does not exist, ignoring", path.string());
    return;
  }

  m_recentProjects.add(path);
}

void App::dataApply() {}

void App::dataWrite(const char* typeName, ImGuiTextBuffer* buf) {
  buf->appendf("[%s][%s]\n", typeName, "RecentProjects");

  // Write them in reverse order, so that way they are read in the correct order
  for (auto it = m_recentProjects.crbegin(); it != m_recentProjects.crend(); ++it) {
    const std::filesystem::path& projectPath = *it;
    if (projectPath.empty())
      continue;

    // Write the path to the buffer
    buf->appendf("%s\n", projectPath.string().c_str());
  }
}

void App::presentProjectPages() {
  if (m_showActions) {
    m_actionsPage.present(&m_showActions);

    if (m_showActions) {
      switch (m_actionsPage.lastPresentEvent()) {
        using enum ActionsPage::Event;
        case NONE:
          break;
        case RENAME_ACTION:
          m_projectEvent = ProjectEvent::RENAME_ACTION;
          m_renameActionPopup.setOldActionName(m_actionsPage.eventActionName());
          break;
        case NEW_ACTION:
          m_projectEvent = ProjectEvent::NEW_ACTION;
          break;
        case NEW_ACTION_ADD_TO_GROUP:
          m_projectEvent = ProjectEvent::NEW_ACTION_ADD_TO_GROUP;
          break;
        case INVALID_OPERATION_RECURSIVE_ACTION:
          m_projectEvent = ProjectEvent::RECURSIVE_ACTION_ERROR;
          m_recursiveActionErrorPopup.setActionRecursionPath(m_actionsPage.eventActionRecursionPath());
          m_recursiveActionErrorPopup.setGroupAction(m_actionsPage.eventActionName());
          break;
        default:
          ThunderAutoUnreachable("Unknown actions page event");
          break;
      }
    }
  }

  m_propertiesPage.present(nullptr);
  switch (m_propertiesPage.lastPresentEvent()) {
    using enum PropertiesPage::Event;
    case NONE:
      break;
    case TRAJECTORY_POINT_LINK:
      m_projectEvent = ProjectEvent::LINK_TRAJECTORY_POINT;
      m_linkTrajectoryPointPopup.prepareForOpen();
      break;
    case AUTO_MODE_ADD_STEP:
      m_projectEvent = ProjectEvent::ADD_AUTO_MODE_STEP;
      break;
    case TRAJECTORY_START_BEHAVIOR_LINK:
      m_projectEvent = ProjectEvent::LINK_TRAJECTORY_END_BEHAVIOR;
      m_linkTrajectoryEndBehaviorPopup.setupForCurrentTrajectory(true);
      break;
    case TRAJECTORY_END_BEHAVIOR_LINK:
      m_projectEvent = ProjectEvent::LINK_TRAJECTORY_END_BEHAVIOR;
      m_linkTrajectoryEndBehaviorPopup.setupForCurrentTrajectory(false);
      break;
    default:
      ThunderAutoUnreachable("Unknown properties page event");
  }

  m_trajectoryManagerPage.present(nullptr);
  switch (m_trajectoryManagerPage.lastPresentEvent()) {
    using enum TrajectoryManagerPage::Event;
    case NONE:
      break;
    case NEW_TRAJECTORY:
      m_projectEvent = ProjectEvent::NEW_TRAJECTORY;
      break;
    case RENAME_TRAJECTORY:
      m_projectEvent = ProjectEvent::RENAME_TRAJECTORY;
      m_renameTrajectoryPopup.setOldTrajectoryName(m_trajectoryManagerPage.eventTrajectory());
      break;
    case DUPLICATE_TRAJECTORY:
      m_projectEvent = ProjectEvent::DUPLICATE_TRAJECTORY;
      m_duplicateTrajectoryPopup.setOldTrajectoryName(m_trajectoryManagerPage.eventTrajectory());
      break;
    case LINK_END_BEHAVIOR:
      m_projectEvent = ProjectEvent::LINK_TRAJECTORY_END_BEHAVIOR;
      m_linkTrajectoryEndBehaviorPopup.setTrajectoryName(m_trajectoryManagerPage.eventTrajectory(), true);
      break;
    default:
      ThunderAutoUnreachable("Unknown trajectory manager event");
  }

  m_autoModeManagerPage.present(nullptr);
  switch (m_autoModeManagerPage.lastPresentEvent()) {
    using enum AutoModeManagerPage::Event;
    case NONE:
      break;
    case NEW_AUTO_MODE:
      m_projectEvent = ProjectEvent::NEW_AUTO_MODE;
      break;
    case RENAME_AUTO_MODE:
      m_projectEvent = ProjectEvent::RENAME_AUTO_MODE;
      m_renameAutoModePopup.setOldAutoModeName(m_autoModeManagerPage.eventAutoMode());
      break;
    case DUPLICATE_AUTO_MODE:
      m_projectEvent = ProjectEvent::DUPLICATE_AUTO_MODE;
      m_duplicateAutoModePopup.setOldAutoModeName(m_autoModeManagerPage.eventAutoMode());
      break;
    default:
      ThunderAutoUnreachable("Unknown auto mode manager event");
  }

  m_editorPage.present(nullptr);

  if (m_showProjectSettings) {
    m_projectSettingsPage.present(&m_showProjectSettings);
  }

  if (m_showRemoteUpdate) {
    m_remoteUpdatePage.present(&m_showRemoteUpdate);
  }
}

void App::presentProjectEventPopups() {
  if (m_projectEvent != ProjectEvent::NONE && m_documentManager.history().isLocked()) {
    m_documentEditManager.discardLongEdit();
  }

  switch (m_projectEvent) {
    using enum ProjectEvent;
    case NONE:
      break;
    case UNSAVED:
      presentUnsavedPopup();
      break;
    case SAVE_ERROR:
      presentSaveProjectErrorPopup();
      break;
    case VERSION_DIFFERENT:
      presentProjectVersionDifferentPopup();
      break;
    case CSV_EXPORT:
      presentCSVExportedPopup();
      break;
    case NEW_TRAJECTORY:
      presentNewTrajectoryPopup();
      break;
    case RENAME_TRAJECTORY:
      presentRenameTrajectoryPopup();
      break;
    case DUPLICATE_TRAJECTORY:
      presentDuplicateTrajectoryPopup();
      break;
    case LINK_TRAJECTORY_END_BEHAVIOR:
      presentLinkTrajectoryEndBehaviorPopup();
      break;
    case NEW_AUTO_MODE:
      presentNewAutoModePopup();
      break;
    case RENAME_AUTO_MODE:
      presentRenameAutoModePopup();
      break;
    case DUPLICATE_AUTO_MODE:
      presentDuplicateAutoModePopup();
      break;
    case LINK_TRAJECTORY_POINT:
      presentLinkTrajectoryPointPopup();
      break;
    case ADD_AUTO_MODE_STEP:
      presentAddAutoModeStepPopup();
      break;
    case NEW_ACTION:
    case NEW_ACTION_ADD_TO_GROUP:
      presentNewActionPopup();
      break;
    case RECURSIVE_ACTION_ERROR:
      presentRecursiveActionErrorPopup();
      break;
    case RENAME_ACTION:
      presentRenameActionPopup();
      break;
    default:
      ThunderAutoUnreachable("Unknown project event");
  }
}

void App::presentMenuBar() {
  auto scopedFramePadding =
      ImGui::Scoped::StyleVarY(ImGuiStyleVar_FramePadding, GET_UISIZE(TITLEBAR_FRAME_PADDING_Y));

  if (ImGui::BeginMainMenuBar()) {
    {
      auto scopedDisabled = ImGui::Scoped::Disabled(!getPlatformGraphics().isMainWindowFocused());

      presentFileMenu();

      if (m_documentManager.isOpen()) {
        presentEditMenu();
        presentViewMenu();

        const ThunderAutoProjectState& state = m_documentEditManager.currentState();

        switch (state.editorState.view) {
          using enum ThunderAutoEditorState::View;
          case TRAJECTORY: {
            const ThunderAutoTrajectoryEditorState& trajectoryEditorState =
                state.editorState.trajectoryEditorState;
            const bool hasCurrentTrajectory = !trajectoryEditorState.currentTrajectoryName.empty();

            if (hasCurrentTrajectory) {
              presentTrajectoryMenu();
            }
            break;
          }
          case AUTO_MODE: {
            const ThunderAutoModeEditorState& autoModeEditorState = state.editorState.autoModeEditorState;
            const bool hasCurrentAutoMode = !autoModeEditorState.currentAutoModeName.empty();

            if (hasCurrentAutoMode) {
              presentAutoModeMenu();
            }
            break;
          }
          case NONE:
            break;
          default:
            ThunderAutoUnreachable("Unknown editor view");
        }

        presentToolsMenu();

#ifdef THUNDERAUTO_DEBUG
        if (ImGui::MenuItem("ImGui Demo Window")) {
          m_showImGuiDemoWindow = !m_showImGuiDemoWindow;
        }
#endif
      }

      m_menuBarWidth = ImGui::GetCursorPosX();
    }

    m_menuBarHeight = ImGui::GetWindowSize().y;

#if THUNDERAUTO_DIRECTX11  // Title bar is custom for DirectX11
    presentMenuBarTitle();
#endif

    ImGui::EndMainMenuBar();
  }
}

void App::presentMenuBarTitle() {
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems)
    return;

  auto scopedDisabled = ImGui::Scoped::Disabled(!getPlatformGraphics().isMainWindowFocused());
  // auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont);

  const double spacerWidth = ImGui::GetContentRegionAvail().x - 3 * GET_UISIZE(TITLEBAR_BUTTON_WIDTH);

  double titleWidth = 0.0;
  double filenameWidth = 0.0;
  double appNameWidth = 0.0;

  const ImVec2 spacerMin(win->DC.CursorPos.x, win->DC.CursorPos.y + win->DC.CurrLineTextBaseOffset);

  std::string beginTitle;
  std::string endTitle;

  const char* filename = m_titlebarFilename.c_str();
  const ImVec2 filenameSize = ImGui::CalcTextSize(filename);

  static const char* elipsis = "...";
  const ImVec2 elipsisSize = ImGui::CalcTextSize(elipsis);

  static const char* dash = " - ";
  const ImVec2 dashSize = ImGui::CalcTextSize(dash);

  static const char* appName = DEFAULT_WINDOW_TITLE;
  const ImVec2 appNameSize = ImGui::CalcTextSize(appName);

  if (appNameSize.x >= spacerWidth) {  // Nothing will fit.
  } else if (elipsisSize.x + dashSize.x + appNameSize.x >= spacerWidth ||
             !*filename) {  // Just app name will fit
    titleWidth = appNameSize.x;
    appNameWidth = appNameSize.x;

    endTitle = appName;
  } else if (filenameSize.x + dashSize.x + appNameSize.x >=
             spacerWidth) {  // Some of the filename and app name will fit
    titleWidth = spacerWidth;
    appNameWidth = elipsisSize.x + dashSize.x + appNameSize.x;
    filenameWidth = spacerWidth - appNameWidth;

    beginTitle = filename;
    endTitle = std::string(elipsis) + dash + appName;

  } else {  // Filename and app name will fit
    titleWidth = filenameSize.x + dashSize.x + appNameSize.x;
    filenameWidth = filenameSize.x;
    appNameWidth = dashSize.x + appNameSize.x;

    beginTitle = filename;
    endTitle = std::string(dash) + appName;
  }

  const float spacerEdgeWidth = (spacerWidth - titleWidth) / 2.f;
  const float textHeight = ImGui::GetTextLineHeight();

  const ImVec2 beginMin(spacerMin.x + spacerEdgeWidth, spacerMin.y);
  const ImVec2 beginMax(beginMin.x + filenameWidth, spacerMin.y + textHeight);

  const ImVec2 beginSize = beginMax - beginMin;

  const ImVec2 endMin(beginMin.x + filenameWidth, spacerMin.y);
  const ImVec2 endMax(endMin.x + appNameWidth, spacerMin.y + textHeight);

  const ImVec2 endSize = endMax - endMin;

  {
    auto scopedSpacerID = ImGui::Scoped::ID("Menu Spacer");

    const ImRect spacer(spacerMin, spacerMin + ImVec2(spacerWidth, textHeight));
    ImGui::ItemSize(spacer);
    ImGui::ItemAdd(spacer, 0);

    const char* beginTitleStr = beginTitle.c_str();
    const char* endTitleStr = endTitle.c_str();

    ImGui::RenderTextClipped(beginMin, beginMax, beginTitleStr, beginTitleStr + strlen(beginTitleStr),
                             &beginSize);
    ImGui::RenderTextClipped(endMin, endMax, endTitleStr, endTitleStr + strlen(endTitleStr), &endSize);
  }
}

#ifdef THUNDERAUTO_MACOS
#define CTRL_STR "Cmd+"
#define CTRL_SHIFT_STR "Cmd+Shift+"
#else
#define CTRL_STR "Ctrl+"
#define CTRL_SHIFT_STR "Ctrl+Shift+"
#endif

void App::presentFileMenu() {
  bool itemNew = false, itemOpen = false, itemSave = false, itemSaveAs = false, itemClose = false;

  ThunderAutoProjectSettings& settings = m_documentManager.settings();

  bool showMenu;
  {
    auto scopedSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(TITLEBAR_ITEM_SPACING_Y));

    showMenu = ImGui::BeginMenu("File");
  }

  if (showMenu) {
    ImGui::MenuItem(ICON_LC_FILE "  New", CTRL_STR "N", &itemNew);
    ImGui::MenuItem(ICON_LC_FOLDER_OPEN "  Open", CTRL_STR "O", &itemOpen);

    {
      auto scopedDisabled = ImGui::Scoped::Disabled(!m_documentManager.isOpen());

      ImGui::MenuItem(ICON_LC_SAVE "  Save", CTRL_STR "S", &itemSave);
      ImGui::MenuItem(ICON_LC_SAVE "  Save As", CTRL_SHIFT_STR "S", &itemSaveAs);

      ImGui::Separator();

      if (ImGui::MenuItem(ICON_LC_FILE_SPREADSHEET "  Export All Trajectories to CSV")) {
        csvExportAllTrajectories();
      }

      ImGui::Separator();

      if (ImGui::MenuItem(ICON_LC_SAVE "  Auto Save", nullptr, &settings.autoSave)) {
        if (settings.autoSave) {
          save();
        }
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("Automatically save project\nwhen changes are made");
      }

      if (ImGui::MenuItem(ICON_LC_FILE_SPREADSHEET "  Auto CSV Export", nullptr, &settings.autoCSVExport)) {
        if (settings.autoCSVExport) {
          m_documentManager.history().markUnsaved();
          csvExportAllTrajectories();
        }
      }
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("Automatically export all paths\nwhen focus is lost");
      }

      ImGui::Separator();

      ImGui::MenuItem(ICON_LC_X "  Close", CTRL_STR "W", &itemClose);
    }

    ImGui::EndMenu();
  }

  if (itemNew)
    tryChangeState(EventState::NEW_PROJECT);
  if (itemOpen)
    tryChangeState(EventState::OPEN_PROJECT);
  if (itemSave)
    save();
  if (itemSaveAs)
    saveAs();
  if (itemClose)
    tryChangeState(EventState::CLOSE_PROJECT);
}

void App::presentEditMenu() {
  bool showMenu;
  {
    auto scopedItemSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(TITLEBAR_ITEM_SPACING_Y));

    showMenu = ImGui::BeginMenu("Edit");
  }

  if (showMenu) {
    if (ImGui::MenuItem(ICON_LC_UNDO "  Undo", CTRL_STR "Z")) {
      undo();
    }
    if (ImGui::MenuItem(ICON_LC_REDO "  Redo", CTRL_SHIFT_STR "Z")) {
      redo();
    }

    ImGui::Separator();

    // if (ImGui::MenuItem(ICON_LC_DELETE "  Delete", "Delete")) {
    // TODO
    // }

    ImGui::EndMenu();
  }
}

void App::presentViewMenu() {
  bool show_menu;
  {
    auto scopedItemSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(TITLEBAR_ITEM_SPACING_Y));

    show_menu = ImGui::BeginMenu("View");
  }

  if (show_menu) {
    if (ImGui::MenuItem(ICON_LC_ROTATE_CCW " Reset Editor View", CTRL_STR "0")) {
      m_editorPage.resetView();
    }
    if (ImGui::MenuItem(ICON_LC_PANELS_TOP_LEFT " Reset Docking Layout")) {
      // Re-initialize dockspace on next frame
      m_resetDockspace = true;
      // Default opened/closed pages
      m_showActions = true;
      m_showProjectSettings = false;
      m_showRemoteUpdate = false;
      // Reset editor view as well
      m_editorPage.resetView();
    }
    ImGui::EndMenu();
  }
}

void App::presentTrajectoryMenu() {
  bool itemExport = false, itemRename = false, itemReverse = false, itemDuplicate = false, itemDelete = false;

  ThunderAutoProjectState state = m_documentEditManager.currentState();

  bool showMenu;
  {
    auto scopedItemSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(TITLEBAR_ITEM_SPACING_Y));

    showMenu = ImGui::BeginMenu("Trajectory");
  }

  if (showMenu) {
    ImGui::MenuItem(ICON_LC_FILE_SPREADSHEET "  Export to CSV", nullptr, &itemExport);
    ImGui::MenuItem(ICON_LC_PENCIL "  Rename", nullptr, &itemRename);
    ImGui::MenuItem(ICON_LC_ARROW_RIGHT_LEFT "  Reverse Direction", nullptr, &itemReverse);
    ImGui::MenuItem(ICON_LC_COPY "  Duplicate", nullptr, &itemDuplicate);
    ImGui::MenuItem(ICON_LC_TRASH "  Delete", nullptr, &itemDelete);
    ImGui::EndMenu();
  }

  if (itemExport) {
    csvExportCurrentTrajectory();
  }

  const std::string currentTrajectoryName = state.editorState.trajectoryEditorState.currentTrajectoryName;

  if (itemRename) {
    m_renameTrajectoryPopup.setOldTrajectoryName(currentTrajectoryName);
    m_projectEvent = ProjectEvent::RENAME_TRAJECTORY;
  }

  if (itemReverse) {
    state.trajectoryReverseDirection(currentTrajectoryName);
    m_documentEditManager.addState(state);
  }

  if (itemDuplicate) {
    m_duplicateTrajectoryPopup.setOldTrajectoryName(currentTrajectoryName);
    m_projectEvent = ProjectEvent::DUPLICATE_TRAJECTORY;
  }

  if (itemDelete) {
    state.trajectoryDelete(currentTrajectoryName);
    m_documentEditManager.addState(state);
  }
}

void App::presentAutoModeMenu() {
  bool itemRename = false, itemDuplicate = false, itemDelete = false;

  ThunderAutoProjectState state = m_documentEditManager.currentState();

  bool showMenu;
  {
    auto scopedItemSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(TITLEBAR_ITEM_SPACING_Y));

    showMenu = ImGui::BeginMenu("Auto Mode");
  }

  if (showMenu) {
    ImGui::MenuItem(ICON_LC_PENCIL "  Rename", nullptr, &itemRename);
    ImGui::MenuItem(ICON_LC_COPY "  Duplicate", nullptr, &itemDuplicate);
    ImGui::MenuItem(ICON_LC_TRASH "  Delete", nullptr, &itemDelete);
    ImGui::EndMenu();
  }

  const std::string currentAutoModeName = state.editorState.autoModeEditorState.currentAutoModeName;

  if (itemRename) {
    m_renameAutoModePopup.setOldAutoModeName(currentAutoModeName);
    m_projectEvent = ProjectEvent::RENAME_AUTO_MODE;
  }

  if (itemDuplicate) {
    m_duplicateAutoModePopup.setOldAutoModeName(currentAutoModeName);
    m_projectEvent = ProjectEvent::DUPLICATE_AUTO_MODE;
  }

  if (itemDelete) {
    state.autoModeDelete(currentAutoModeName);
    m_documentEditManager.addState(state);
  }
}

void App::presentToolsMenu() {
  bool showMenu;
  {
    auto scopedItemSpacing =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(TITLEBAR_ITEM_SPACING_Y));

    showMenu = ImGui::BeginMenu("Tools");
  }

  if (showMenu) {
    bool dummy = true;
    ImGui::MenuItem(ICON_LC_LIST "  Trajectories", nullptr, &dummy);
    ImGui::MenuItem(ICON_LC_LIST "  Auto Modes", nullptr, &dummy);
    ImGui::MenuItem(ICON_LC_SPLINE_POINTER "  Editor", nullptr, &dummy);
    ImGui::MenuItem(ICON_LC_SETTINGS_2 "  Properties", nullptr, &dummy);
    ImGui::MenuItem(ICON_LC_PAPERCLIP "  Actions", nullptr, &m_showActions);
    ImGui::MenuItem(ICON_LC_SETTINGS "  Project Settings", nullptr, &m_showProjectSettings);
    ImGui::MenuItem(ICON_LC_ROUTER "  Remote Update", nullptr, &m_showRemoteUpdate);

    ImGui::EndMenu();
  }
}

bool App::tryChangeState(EventState desiredState) {
  if (m_documentManager.isUnsaved()) {
    m_projectEvent = ProjectEvent::UNSAVED;
    m_nextEventState = desiredState;
    return false;
  }

  if (m_eventState == desiredState) {
    return true;
  }

  m_eventState = desiredState;

  switch (m_eventState) {
    using enum EventState;
    case NEW_PROJECT:
      m_newProjectPopup.reset();
      m_newFieldPopup.reset();
      break;
    default:
      break;
  }

  return true;
}

void App::presentWelcomePopup() {
  bool showingPopup = true;
  m_welcomePopup.present(&showingPopup);

  if (showingPopup)
    return;

  WelcomePopup::Result result = m_welcomePopup.result();

  switch (result) {
    using enum WelcomePopup::Result;
    case NEW_PROJECT:
      m_eventState = EventState::NEW_PROJECT;
      m_newProjectPopup.reset();
      m_newFieldPopup.reset();
      break;
    case OPEN_PROJECT:
      m_eventState = EventState::OPEN_PROJECT;
      break;
    case RECENT_PROJECT: {
      auto recentPathIt = m_welcomePopup.recentProject();
      ThunderAutoAssert(recentPathIt != m_recentProjects.end());
      std::filesystem::path recentPath = *recentPathIt;
      openFromPath(recentPath);
      break;
    }
    default:
      ThunderAutoUnreachable("Unknown welcome popup result");
  }
}

void App::presentNewProjectPopup() {
  ImGui::OpenPopup(m_newProjectPopup.name());

  bool showingPopup = true;

  m_newProjectPopup.present(&showingPopup);

  if (showingPopup)
    return;

  NewProjectPopup::Result result = m_newProjectPopup.result();

  m_eventState = EventState::PROJECT;

  switch (result) {
    using enum NewProjectPopup::Result;
    case NEW_FIELD:
      m_eventState = EventState::NEW_FIELD;
      break;
    case CREATE: {
      m_documentManager.newProject(m_newProjectPopup.resultProject());
      const ThunderAutoProjectSettings& settings = m_documentManager.settings();
      m_editorPage.setupField(settings);
      m_propertiesPage.setup(settings);
      updateTitlebarTitle();
      m_recentProjects.add(m_documentManager.path());
      break;
    }
    case CANCEL:
      break;
    default:
      ThunderAutoUnreachable("Unknown new project popup result");
  }
}

void App::presentNewFieldPopup() {
  ImGui::OpenPopup(m_newFieldPopup.name());

  bool showingPopup = true;

  m_newFieldPopup.present(&showingPopup);

  if (showingPopup)
    return;

  NewFieldPopup::Result result = m_newFieldPopup.result();

  m_eventState = EventState::NEW_PROJECT;

  switch (result) {
    using enum NewFieldPopup::Result;
    case CREATE:
      m_newProjectPopup.setField(m_newFieldPopup.field());
      break;
    case CANCEL:
      break;
    default:
      ThunderAutoUnreachable("Unknown new field popup result");
  }
}

void App::openProject() {
  std::filesystem::path path = getPlatform().openFileDialog(FileType::FILE, {kThunderAutoFileFilter});

  if (path.empty()) {  // Open cancelled.
    m_eventState = EventState::NONE;
    return;
  }

  openFromPath(path);
}

void App::openFromPath(const std::filesystem::path& path) {
  m_recentProjects.remove(path);

  std::string projectOpenError;
  ThunderAutoProjectVersion projectVersion;

  try {
    projectVersion = m_documentManager.openProject(path);
  } catch (const ThunderError& e) {
    projectOpenError = e.message();
  } catch (const std::exception& e) {
    projectOpenError = e.what();
  } catch (...) {
    projectOpenError = "Unknown error ocurred";
  }

  if (!projectOpenError.empty()) {
    m_openProjectErrorPopup.setError(projectOpenError);
    m_eventState = EventState::OPEN_PROJECT_ERROR;
    return;
  }

  m_eventState = EventState::PROJECT;

  const ThunderAutoProjectSettings& settings = m_documentManager.settings();
  m_editorPage.setupField(settings);
  m_propertiesPage.setup(settings);

  m_recentProjects.add(path);

  if (projectVersion.major != THUNDERAUTO_PROJECT_VERSION_MAJOR ||
      projectVersion.minor != THUNDERAUTO_PROJECT_VERSION_MINOR) {
    m_projectVersionPopup.setProjectVersion(projectVersion);
    m_projectEvent = ProjectEvent::VERSION_DIFFERENT;
  }

  updateTitlebarTitle();
}

void App::presentUnsavedPopup() {
  ImGui::OpenPopup(m_unsavedPopup.name());

  bool showingPopup = true;

  m_unsavedPopup.present(&showingPopup);

  if (showingPopup)
    return;

  UnsavedPopup::Result result = m_unsavedPopup.result();

  m_projectEvent = ProjectEvent::NONE;

  switch (result) {
    using enum UnsavedPopup::Result;
    case SAVE:
      save();
      m_documentManager.close();
      m_eventState = m_nextEventState;
      m_nextEventState = EventState::NONE;
      break;
    case DONT_SAVE:
      m_documentManager.close();
      m_eventState = m_nextEventState;
      m_nextEventState = EventState::NONE;
      break;
    case CANCEL:
      m_eventState = m_nextEventState = EventState::NONE;
      break;
    default:
      ThunderAutoUnreachable("Unknown unsaved popup result");
  }
}

void App::presentOpenProjectErrorPopup() {
  ImGui::OpenPopup(m_openProjectErrorPopup.name());

  bool showingPopup = true;

  m_openProjectErrorPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_eventState = EventState::NONE;
}

void App::presentSaveProjectErrorPopup() {
  ImGui::OpenPopup(m_saveProjectErrorPopup.name());

  bool showingPopup = true;

  m_saveProjectErrorPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentProjectVersionDifferentPopup() {
  ImGui::OpenPopup(m_projectVersionPopup.name());

  bool showingPopup = true;

  m_projectVersionPopup.present(&showingPopup);

  if (showingPopup)
    return;

  ProjectVersionPopup::Result result = m_projectVersionPopup.result();

  m_projectEvent = ProjectEvent::NONE;

  switch (result) {
    using enum ProjectVersionPopup::Result;
    case OK:
      // Proceed normally.
      break;
    case CANCEL:
      m_documentManager.close();
      m_eventState = EventState::NONE;
      break;
    default:
      ThunderAutoUnreachable("Unknown project version popup result");
  }
}

void App::presentCSVExportedPopup() {
  ImGui::OpenPopup(m_csvExportPopup.name());

  bool showingPopup = true;

  m_csvExportPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentNewTrajectoryPopup() {
  ImGui::OpenPopup(m_newTrajectoryPopup.name());

  bool showingPopup = true;

  m_newTrajectoryPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentRenameTrajectoryPopup() {
  ImGui::OpenPopup(m_renameTrajectoryPopup.name());

  bool showingPopup = true;

  m_renameTrajectoryPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentDuplicateTrajectoryPopup() {
  ImGui::OpenPopup(m_duplicateTrajectoryPopup.name());

  bool showingPopup = true;

  m_duplicateTrajectoryPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentLinkTrajectoryEndBehaviorPopup() {
  ImGui::OpenPopup(m_linkTrajectoryEndBehaviorPopup.name());

  bool showingPopup = true;

  m_linkTrajectoryEndBehaviorPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentNewAutoModePopup() {
  ImGui::OpenPopup(m_newAutoModePopup.name());

  bool showingPopup = true;

  m_newAutoModePopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentRenameAutoModePopup() {
  ImGui::OpenPopup(m_renameAutoModePopup.name());

  bool showingPopup = true;

  m_renameAutoModePopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentDuplicateAutoModePopup() {
  ImGui::OpenPopup(m_duplicateAutoModePopup.name());

  bool showingPopup = true;

  m_duplicateAutoModePopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentLinkTrajectoryPointPopup() {
  ImGui::OpenPopup(m_linkTrajectoryPointPopup.name());

  bool showingPopup = true;

  m_linkTrajectoryPointPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentAddAutoModeStepPopup() {
  ImGui::OpenPopup(m_addAutoModeStepPopup.name());

  bool showingPopup = true;

  m_addAutoModeStepPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentNewActionPopup() {
  ImGui::OpenPopup(m_newActionPopup.name());

  bool showingPopup = true;

  m_newActionPopup.present(&showingPopup);

  if (showingPopup)
    return;

  ThunderAutoProjectState projectState = m_documentEditManager.currentState();

  switch (m_projectEvent) {
    using enum ProjectEvent;
    case NEW_ACTION:
      break;
    case NEW_ACTION_ADD_TO_GROUP: {
      std::string actionGroupName = m_actionsPage.eventActionName();
      ThunderAutoAction& actionGroup = projectState.getAction(actionGroupName);
      actionGroup.addGroupAction(m_newActionPopup.newActionName());
      m_documentEditManager.addState(projectState);
    } break;
    default:
      ThunderAutoUnreachable("Invalid new action project event state");
  }

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentRecursiveActionErrorPopup() {
  ImGui::OpenPopup(m_recursiveActionErrorPopup.name());

  bool showingPopup = true;

  m_recursiveActionErrorPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::presentRenameActionPopup() {
  ImGui::OpenPopup(m_renameActionPopup.name());

  bool showingPopup = true;

  m_renameActionPopup.present(&showingPopup);

  if (showingPopup)
    return;

  m_projectEvent = ProjectEvent::NONE;
}

void App::save() {
  std::string projectSaveError;

  try {
    m_documentManager.save();
  } catch (const ThunderError& e) {
    projectSaveError = e.message();
  } catch (const std::exception& e) {
    projectSaveError = e.what();
  } catch (...) {
    projectSaveError = "Unknown error ocurred";
  }

  if (!projectSaveError.empty()) {
    m_saveProjectErrorPopup.setError(projectSaveError);
    m_projectEvent = ProjectEvent::SAVE_ERROR;
  }
}

void App::saveAs() {
  std::filesystem::path path = getPlatform().saveFileDialog({kThunderAutoFileFilter});
  if (path.empty())
    return;

  m_documentManager.setProjectPath(path);
  updateTitlebarTitle();

  save();

  m_recentProjects.add(path);
}

void App::csvExportAllTrajectories() {
  const ThunderAutoProjectState& projectState = m_documentEditManager.currentState();
  const ThunderAutoProjectSettings& projectSettings = m_documentManager.settings();

  std::filesystem::path exportDir = m_documentManager.settings().directory;

  std::string csvExportStatus;
  for (const auto& [name, skeleton] : projectState.trajectories) {
    std::filesystem::path exportPath = exportDir / (name + ".csv");

    try {
      BuildAndCSVExportThunderAutoOutputTrajectory(skeleton, kHighResOutputTrajectorySettings,
                                                   projectState.actionsOrder, exportPath,
                                                   projectSettings.csvExportProps);

    } catch (const ThunderError& e) {
      csvExportStatus = fmt::format("Failed to export trajectory '{}' to '{}': {}", exportPath.string(),
                                    exportDir.string(), e.message());
      ThunderAutoLogger::Error("{}", csvExportStatus);
      break;

    } catch (const std::exception& e) {
      csvExportStatus = fmt::format("Failed to export trajectory '{}' to '{}': {}", exportPath.string(),
                                    exportDir.string(), e.what());
      ThunderAutoLogger::Error("{}", csvExportStatus);
      break;

    } catch (...) {
      csvExportStatus = fmt::format("Failed to export trajectory '{}' to '{}': Unknown error",
                                    exportPath.string(), exportDir.string());
      ThunderAutoLogger::Error("{}", csvExportStatus);
      break;
    }
  }

  if (csvExportStatus.empty()) {
    csvExportStatus = fmt::format("Successfully exported all trajectories to {}", exportDir.string());
  }

  m_projectEvent = ProjectEvent::CSV_EXPORT;
  m_csvExportPopup.setExportMessage(csvExportStatus);
}

void App::csvExportCurrentTrajectory() {
  const ThunderAutoProjectState& projectState = m_documentEditManager.currentState();
  const ThunderAutoProjectSettings& projectSettings = m_documentManager.settings();

  const ThunderAutoEditorState& editorState = projectState.editorState;
  ThunderAutoAssert(editorState.view == ThunderAutoEditorState::View::TRAJECTORY,
                    "Cannot export current trajectory when not in trajectory view");

  std::string trajectoryName = editorState.trajectoryEditorState.currentTrajectoryName;
  ThunderAutoAssert(!trajectoryName.empty(), "Current trajectory name is empty");

  ThunderAutoAssert(projectState.trajectories.contains(trajectoryName),
                    "Current trajectory name does not match any trajectory in the project state");
  const ThunderAutoTrajectorySkeleton& trajectory = projectState.trajectories.at(trajectoryName);

  std::filesystem::path exportDir = m_documentManager.settings().directory;
  std::filesystem::path exportPath = exportDir / (trajectoryName + ".csv");

  std::string csvExportStatus;
  try {
    BuildAndCSVExportThunderAutoOutputTrajectory(trajectory, kHighResOutputTrajectorySettings,
                                                 projectState.actionsOrder, exportPath,
                                                 projectSettings.csvExportProps);

  } catch (const ThunderError& e) {
    csvExportStatus = fmt::format("Failed to export trajectory '{}' to '{}': {}", exportPath.string(),
                                  exportDir.string(), e.message());
    ThunderAutoLogger::Error("{}", csvExportStatus);

  } catch (const std::exception& e) {
    csvExportStatus = fmt::format("Failed to export trajectory '{}' to '{}': {}", exportPath.string(),
                                  exportDir.string(), e.what());
    ThunderAutoLogger::Error("{}", csvExportStatus);

  } catch (...) {
    csvExportStatus = fmt::format("Failed to export trajectory '{}' to '{}': Unknown error",
                                  exportPath.string(), exportDir.string());
    ThunderAutoLogger::Error("{}", csvExportStatus);
  }

  if (csvExportStatus.empty()) {
    csvExportStatus =
        fmt::format("Successfully exported trajectory '{}' to {}", trajectoryName, exportPath.string());
  }

  m_projectEvent = ProjectEvent::CSV_EXPORT;
  m_csvExportPopup.setExportMessage(csvExportStatus);
}

void App::undo() {
  if (m_eventState != EventState::PROJECT || m_projectEvent != ProjectEvent::NONE) {
    return;
  }

  m_documentEditManager.undo();
}

void App::redo() {
  if (m_eventState != EventState::PROJECT || m_projectEvent != ProjectEvent::NONE) {
    return;
  }

  m_documentEditManager.redo();
}

void App::updateTitlebarTitle() {
  if (!m_documentManager.isOpen()) {
    m_titlebarFilename = "";
    getPlatformGraphics().setMainWindowTitle("");
    return;
  }

  std::string title;
  if (m_documentManager.isUnsaved() && !m_documentManager.settings().autoSave) {
    title += "* ";
  }
  title += m_documentManager.name();

  m_titlebarFilename = title;
  getPlatformGraphics().setMainWindowTitle(m_titlebarFilename.c_str());
}

void App::processInput() {
  if (IsCtrlDown() && IsKeyPressed(ImGuiKey_N)) {  // Ctrl+N
    tryChangeState(EventState::NEW_PROJECT);

  } else if (IsCtrlDown() && IsKeyPressed(ImGuiKey_O)) {  // Ctrl+O
    tryChangeState(EventState::OPEN_PROJECT);

  } else if (m_documentManager.isOpen()) {
    if (IsCtrlDown() && IsKeyPressed(ImGuiKey_S)) {  // Ctrl+S
      save();

    } else if (IsCtrlShiftDown() && IsKeyPressed(ImGuiKey_S)) {  // Ctrl+Shift+S
      saveAs();

    } else if (IsCtrlDown() && IsKeyPressed(ImGuiKey_W)) {  // Ctrl+W
      tryChangeState(EventState::CLOSE_PROJECT);

    } else if (IsCtrlDown() && IsKeyPressedOrRepeat(ImGuiKey_Z)) {  // Ctrl+Z
      undo();

    } else if ((IsCtrlDown() && IsKeyPressedOrRepeat(ImGuiKey_Y)) ||       // Ctrl+Y
               (IsCtrlShiftDown() && IsKeyPressedOrRepeat(ImGuiKey_Z))) {  // Ctrl+Shift+Z
      redo();

    } else if (IsCtrlDown() &&
               (IsKeyPressed(ImGuiKey_E) || IsKeyPressed(ImGuiKey_Apostrophe))) {  // Ctrl+E or Ctrl+'
      csvExportAllTrajectories();
    }
  }
}
