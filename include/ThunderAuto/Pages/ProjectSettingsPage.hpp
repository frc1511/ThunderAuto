#pragma once

#include <ThunderAuto/DocumentManager.hpp>
#include <ThunderAuto/Pages/Page.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>

using namespace thunder::core;

class ProjectSettingsPage : public Page {
  DocumentManager& m_documentManager;

  EditorPage& m_editorPage;

  enum class SettingsSubPage {
    ROBOT_SETTINGS,
    CSV_EXPORT_SETTINGS,
    TRAJECTORY_EDITOR_SETTINGS,
  };
  SettingsSubPage m_subPage = SettingsSubPage::ROBOT_SETTINGS;

 public:
  explicit ProjectSettingsPage(DocumentManager& documentManager, EditorPage& editorPage)
      : m_documentManager(documentManager), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Project Settings"; }

  void present(bool* running) override;

 private:
  void presentRobotSettings();
  void presentCSVExportSettings();
  void presentTrajectoryEditorSettings();
};
