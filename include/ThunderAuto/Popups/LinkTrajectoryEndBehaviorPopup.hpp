#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <string>

class LinkTrajectoryEndBehaviorPopup : public Popup {
  DocumentEditManager& m_history;

  std::string m_trajectoryName;

  std::string m_initialStartLinkName;
  std::string m_initialEndLinkName;

  std::string m_selectedLinkName;
  bool m_createNewLink = false;
  char m_newLinkNameBuffer[256] = "";

  bool m_startBehavior = false;

 public:
  explicit LinkTrajectoryEndBehaviorPopup(DocumentEditManager& history) : m_history(history) {}

  const char* name() const noexcept override { return "Link Trajectory End Behavior"; }

  void present(bool* running) override;

  void reset() noexcept {
    m_trajectoryName.clear();
    m_initialStartLinkName.clear();
    m_initialEndLinkName.clear();
    m_selectedLinkName.clear();
    m_createNewLink = false;
    m_newLinkNameBuffer[0] = '\0';
    m_startBehavior = false;
  }

  void setTrajectoryName(const std::string& trajectoryName, bool startBehavior) noexcept;
  void setupForCurrentTrajectory(bool startBehavior) noexcept;
};
