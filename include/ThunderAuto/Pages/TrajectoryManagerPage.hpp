#pragma once

#include <ThunderAuto/Pages/Page.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <cstddef>

using namespace thunder::core;

class TrajectoryManagerPage : public Page {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

 public:
  TrajectoryManagerPage(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  const char* name() const noexcept override { return "Trajectories"; }

  void present(bool* running) override;

  enum class Event {
    NONE = 0,
    NEW_TRAJECTORY,
    RENAME_TRAJECTORY,
    DUPLICATE_TRAJECTORY,
    LINK_END_BEHAVIOR,
  };

  Event lastPresentEvent() const noexcept { return m_event; }

  const std::string& eventTrajectory() const noexcept { return m_eventTrajectory; }

 private:
  Event m_event = Event::NONE;
  std::string m_eventTrajectory;
};
