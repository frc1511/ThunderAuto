#pragma once

#include <ThunderAuto/project_state.hpp>
#include <ThunderAuto/thunder_auto.hpp>

class DocumentManager;

class HistoryManager {
  std::deque<ProjectState> m_history;
  std::deque<ProjectState>::const_iterator m_current_state;

  bool m_unsaved = false;
  bool m_locked = false;

public:
  void mark_unsaved() { m_unsaved = true; }
  void mark_saved() { m_unsaved = false; }

  bool is_unsaved() const { return m_unsaved; }

  void reset(ProjectState state, bool unsaved = false);

  const ProjectState& current_state() const { return *m_current_state; }

  void add_state(ProjectState state, bool unsaved = true);

  void undo();
  void redo();

private:
  friend class DocumentEditManager;

  // Lock undo/redo actions.
  void lock() { m_locked = true; }
  void unlock() { m_locked = false; }

  bool is_locked() const { return m_locked; }
};

