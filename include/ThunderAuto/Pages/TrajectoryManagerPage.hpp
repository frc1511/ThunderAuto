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

  const char* name() const noexcept override { return "Trajectory Manager"; }

  void present(bool* running) override;

  void duplicatePath(ThunderAutoProjectState& state, size_t index);
  void deletePath(ThunderAutoProjectState& state, size_t index);
  void reversePath(ThunderAutoProjectState& state, size_t index);

  enum class Event {
    NONE = 0,
    NEW_TRAJECTORY,
    RENAME_TRAJECTORY,
    DUPLICATE_TRAJECTORY,
  };

  Event lastPresentEvent() const noexcept { return m_event; }

  const std::string& eventTrajectory() const noexcept { return m_eventTrajectory; }

 private:
  bool presentContextMenu(ThunderAutoProjectState& state, size_t index);
  void presentNewPathButton(ThunderAutoProjectState& state);

  static bool selectableInput(const char* label, bool selected, char* buf, size_t bufSize, bool& inputActive);

 private:
  Event m_event = Event::NONE;
  std::string m_eventTrajectory;
};
