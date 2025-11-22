#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>

class NewAutoModePopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  char m_autoModeNameBuffer[256] = "";

 public:
  NewAutoModePopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "New Auto Mode"; }

  void present(bool* running) override;

  void reset() noexcept { m_autoModeNameBuffer[0] = '\0'; }
};
