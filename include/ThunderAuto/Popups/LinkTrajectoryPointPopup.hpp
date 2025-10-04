#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <string>

class LinkTrajectoryPointPopup : public Popup {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  std::string m_initialLinkName;
  std::string m_selectedLinkName;
  bool m_createNewLink = false;
  char m_newLinkNameBuffer[256] = "";

 public:
  LinkTrajectoryPointPopup(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Link Trajectory Point"; }

  void present(bool* running) override;

  void reset() noexcept {
    m_initialLinkName.clear();
    m_selectedLinkName.clear();
    m_createNewLink = false;
    m_newLinkNameBuffer[0] = '\0';
  }

  void prepareForOpen();
};
