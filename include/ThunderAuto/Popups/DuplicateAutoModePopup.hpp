#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <string_view>

class DuplicateAutoModePopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  char m_autoModeNameBuffer[256] = "";
  std::string m_oldAutoModeName;

 public:
  DuplicateAutoModePopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Duplicate Auto Mode"; }

  void present(bool* running) override;

  void setOldAutoModeName(std::string_view name) {
    m_oldAutoModeName = name;

    size_t copySize = std::min(name.size(), sizeof(m_autoModeNameBuffer) - 1);
    std::memcpy(m_autoModeNameBuffer, name.data(), copySize);
    m_autoModeNameBuffer[copySize] = '\0';
  }

  void reset() noexcept { m_autoModeNameBuffer[0] = '\0'; }
};
