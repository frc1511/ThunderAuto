#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>

class NewTrajectoryPopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  char m_trajectoryNameBuffer[256] = "";

 public:
  NewTrajectoryPopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "New Trajectory"; }

  void present(bool* running) override;

  void reset() noexcept { m_trajectoryNameBuffer[0] = '\0'; }
};
