#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <string_view>

class RenameTrajectoryPopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  char m_newTrajectoryNameBuffer[256] = "";
  std::string m_oldTrajectoryName;

 public:
  RenameTrajectoryPopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Rename Trajectory"; }

  void present(bool* running) override;

  void setOldTrajectoryName(std::string_view name) {
    m_oldTrajectoryName = name;

    size_t copySize = std::min(name.size(), sizeof(m_newTrajectoryNameBuffer) - 1);
    std::memcpy(m_newTrajectoryNameBuffer, name.data(), copySize);
    m_newTrajectoryNameBuffer[copySize] = '\0';
  }

  void reset() noexcept { m_newTrajectoryNameBuffer[0] = '\0'; }
};
