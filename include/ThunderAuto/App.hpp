#pragma once

#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/DocumentManager.hpp>
#include <ThunderAuto/FontLibrary.hpp>

#include <ThunderAuto/Popups/NewFieldPopup.hpp>
#include <ThunderAuto/Popups/NewProjectPopup.hpp>
#include <ThunderAuto/Popups/UnsavedPopup.hpp>
#include <ThunderAuto/Popups/WelcomePopup.hpp>
#include <ThunderAuto/Popups/OpenProjectErrorPopup.hpp>
#include <ThunderAuto/Popups/SaveProjectErrorPopup.hpp>
#include <ThunderAuto/Popups/ProjectVersionPopup.hpp>
#include <ThunderAuto/Popups/CSVExportPopup.hpp>
#include <ThunderAuto/Popups/NewTrajectoryPopup.hpp>
#include <ThunderAuto/Popups/RenameTrajectoryPopup.hpp>
#include <ThunderAuto/Popups/DuplicateTrajectoryPopup.hpp>
#include <ThunderAuto/Popups/NewAutoModePopup.hpp>
#include <ThunderAuto/Popups/RenameAutoModePopup.hpp>
#include <ThunderAuto/Popups/DuplicateAutoModePopup.hpp>
#include <ThunderAuto/Popups/LinkTrajectoryPointPopup.hpp>
#include <ThunderAuto/Popups/NewActionPopup.hpp>
#include <ThunderAuto/Popups/RenameActionPopup.hpp>
#include <ThunderAuto/Popups/RecursiveActionErrorPopup.hpp>

#include <ThunderAuto/Pages/ActionsPage.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <ThunderAuto/Pages/TrajectoryManagerPage.hpp>
#include <ThunderAuto/Pages/AutoModeManagerPage.hpp>
#include <ThunderAuto/Pages/PropertiesPage.hpp>
#include <ThunderAuto/Pages/ProjectSettingsPage.hpp>

#include <ThunderLibCore/RecentItemList.hpp>

#include <ThunderLibCore/Auto/ThunderAutoOutputTrajectory.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>

#include <string>
#include <filesystem>

using namespace thunder::core;

struct GLFWwindow;

class App {
  bool m_running = true;

  // The state of the application (used to manage popups and stuff).
  enum class EventState {
    NONE = 0,
    PROJECT = NONE,
    WELCOME,
    NEW_PROJECT,
    NEW_FIELD,
    OPEN_PROJECT,
    CLOSE_PROJECT,
    CLOSE_EVERYTHING,
    OPEN_PROJECT_ERROR,
  };

  EventState m_eventState = EventState::WELCOME;
  EventState m_nextEventState = EventState::NONE;

  enum class ProjectEvent {
    NONE,
    UNSAVED,
    SAVE_ERROR,
    VERSION_DIFFERENT,
    CSV_EXPORT,

    NEW_TRAJECTORY,
    RENAME_TRAJECTORY,
    DUPLICATE_TRAJECTORY,

    NEW_AUTO_MODE,
    RENAME_AUTO_MODE,
    DUPLICATE_AUTO_MODE,

    LINK_TRAJECTORY_POINT,

    NEW_ACTION,
    NEW_ACTION_ADD_TO_GROUP,
    RENAME_ACTION,
    RECURSIVE_ACTION_ERROR,
  };

  ProjectEvent m_projectEvent = ProjectEvent::NONE;

  DocumentManager m_documentManager;
  DocumentEditManager m_documentEditManager{m_documentManager.history()};

  RecentItemList<std::filesystem::path, 15> m_recentProjects;

  bool m_wasUnsaved = false;
  std::string m_titlebarFilename;

  // Popup Modals

  NewFieldPopup m_newFieldPopup;
  NewProjectPopup m_newProjectPopup;
  UnsavedPopup m_unsavedPopup;
  WelcomePopup m_welcomePopup{m_recentProjects};
  OpenProjectErrorPopup m_openProjectErrorPopup;
  SaveProjectErrorPopup m_saveProjectErrorPopup;
  ProjectVersionPopup m_projectVersionPopup;
  CSVExportPopup m_csvExportPopup;

  NewTrajectoryPopup m_newTrajectoryPopup{m_documentEditManager, m_editorPage};
  RenameTrajectoryPopup m_renameTrajectoryPopup{m_documentEditManager, m_editorPage};
  DuplicateTrajectoryPopup m_duplicateTrajectoryPopup{m_documentEditManager, m_editorPage};
  NewAutoModePopup m_newAutoModePopup{m_documentEditManager, m_editorPage};
  RenameAutoModePopup m_renameAutoModePopup{m_documentEditManager, m_editorPage};
  DuplicateAutoModePopup m_duplicateAutoModePopup{m_documentEditManager, m_editorPage};
  LinkTrajectoryPointPopup m_linkTrajectoryPointPopup{m_documentEditManager, m_editorPage};
  NewActionPopup m_newActionPopup{m_documentEditManager};
  RenameActionPopup m_renameActionPopup{m_documentEditManager};
  RecursiveActionErrorPopup m_recursiveActionErrorPopup;

  // Pages

  EditorPage m_editorPage{m_documentEditManager};
  TrajectoryManagerPage m_trajectoryManagerPage{m_documentEditManager, m_editorPage};
  AutoModeManagerPage m_autoModeManagerPage{m_documentEditManager, m_editorPage};
  PropertiesPage m_propertiesPage{m_documentEditManager, m_editorPage};
  ActionsPage m_actionsPage{m_documentEditManager};
  ProjectSettingsPage m_projectSettingsPage{m_documentManager, m_editorPage};

  bool m_showEditor = true;
  bool m_showTrajectoryManager = true;
  bool m_showAutoModeManager = true;
  bool m_showProperties = true;
  bool m_showActions = true;
  bool m_showProjectSettings = false;
#ifdef THUNDERAUTO_DEBUG
  bool m_showImGuiDemoWindow = false;
#endif

  // Graphics stuff

  int m_menuBarWidth = 0;
  int m_menuBarHeight = 0;

 public:
  App() = default;

  // Initialization

  void setupDockspace(ImGuiID dockspaceID);

  // Loop stuff

  constexpr bool isRunning() const { return m_running; }

  void focusWasChanged(bool focused);

  void processInput();
  void present();

  // Project stuff

  void openFromPath(const std::filesystem::path& path);
  void close();

  // Data handling

  void dataClear();
  bool dataShouldOpen(const char* name);
  void dataReadLine(const char* line);
  void dataApply();
  void dataWrite(const char* typeName, ImGuiTextBuffer* buf);

 private:
  void presentProjectPages();
  void presentProjectEventPopups();

  void presentMenuBar();
  void presentMenuBarTitle();

  void presentFileMenu();
  void presentEditMenu();
  void presentViewMenu();
  void presentTrajectoryMenu();
  void presentAutoModeMenu();
  void presentToolsMenu();

  bool tryChangeState(EventState eventState);

  void presentWelcomePopup();
  void presentNewProjectPopup();
  void presentNewFieldPopup();
  void openProject();

  void presentUnsavedPopup();
  void presentOpenProjectErrorPopup();
  void presentSaveProjectErrorPopup();
  void presentProjectVersionDifferentPopup();
  void presentCSVExportedPopup();
  void presentNewTrajectoryPopup();
  void presentRenameTrajectoryPopup();
  void presentDuplicateTrajectoryPopup();
  void presentNewAutoModePopup();
  void presentRenameAutoModePopup();
  void presentDuplicateAutoModePopup();
  void presentLinkTrajectoryPointPopup();
  void presentNewActionPopup();
  void presentRenameActionPopup();
  void presentRecursiveActionErrorPopup();

  void save();
  void saveAs();
  void csvExportAllTrajectories();
  void csvExportCurrentTrajectory();

  void undo();
  void redo();

  void updateTitlebarTitle();

 public:
  // Pixel sizes used for the custom titlebar on Windows.
  int menuBarWidth() const { return m_menuBarWidth; }
  int menuBarHeight() const { return m_menuBarHeight; }
};
