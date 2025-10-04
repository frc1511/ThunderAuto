#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <string_view>

class DuplicateTrajectoryPopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  char m_trajectoryNameBuffer[256] = "";
  std::string m_oldTrajectoryName;

 public:
  DuplicateTrajectoryPopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Duplicate Trajectory"; }

  void present(bool* running) override;

  void setOldTrajectoryName(std::string_view name) {
    m_oldTrajectoryName = name;

    size_t copySize = std::min(name.size(), sizeof(m_trajectoryNameBuffer) - 1);
    std::memcpy(m_trajectoryNameBuffer, name.data(), copySize);
    m_trajectoryNameBuffer[copySize] = '\0';
  }

  void reset() noexcept { m_trajectoryNameBuffer[0] = '\0'; }
};
