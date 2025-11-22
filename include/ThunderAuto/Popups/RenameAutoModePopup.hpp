#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <string_view>

class RenameAutoModePopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  char m_newAutoModeNameBuffer[256] = "";
  std::string m_oldAutoModeName;

 public:
  RenameAutoModePopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Rename Auto Mode"; }

  void present(bool* running) override;

  void setOldAutoModeName(std::string_view name) {
    m_oldAutoModeName = name;

    size_t copySize = std::min(name.size(), sizeof(m_newAutoModeNameBuffer) - 1);
    std::memcpy(m_newAutoModeNameBuffer, name.data(), copySize);
    m_newAutoModeNameBuffer[copySize] = '\0';
  }

  void reset() noexcept { m_newAutoModeNameBuffer[0] = '\0'; }
};
