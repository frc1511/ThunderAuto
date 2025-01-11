#pragma once

#include <ThunderAuto/history_manager.h>
#include <ThunderAuto/project_state.h>
#include <ThunderAuto/thunder_auto.h>

class DocumentEditManager {
  HistoryManager* m_history;

  std::optional<ProjectState> m_current_state = std::nullopt;

public:
  inline explicit DocumentEditManager(HistoryManager* history)
    : m_history(history) {}

  inline void start_long_edit() {
    if (m_history->is_locked()) {
      puts("Warning: History is already locked");
      return;
    }
    m_history->lock();

    m_current_state = m_history->current_state();
  }

  inline void discard_long_edit() {
    if (!m_history->is_locked()) {
      puts("Warning: History is not locked");
      return;
    }

    m_history->unlock();

    m_current_state = std::nullopt;
  }

  inline void finish_long_edit() {
    if (!m_history->is_locked()) {
      puts("Warning: History is not locked");
      return;
    }

    m_history->unlock();

    m_history->add_state(*m_current_state);
    m_current_state = std::nullopt;
  }

  inline const ProjectState& current_state() const {
    if (m_history->is_locked()) {
      return *m_current_state;
    }

    return m_history->current_state();
  }

  inline void add_state(ProjectState state, bool unsaved = true) {
    if (m_history->is_locked()) {
      m_current_state = state;
      return;
    }

    m_history->add_state(state, unsaved);
  }
};

