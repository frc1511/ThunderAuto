#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <string>

class LinkTrajectoryPointPopup : public Popup {
  DocumentEditManager& m_history;

  std::string m_initialLinkName;
  std::string m_selectedLinkName;
  bool m_createNewLink = false;
  char m_newLinkNameBuffer[256] = "";

 public:
  explicit LinkTrajectoryPointPopup(DocumentEditManager& history) : m_history(history) {}

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
