#include <ThunderAuto/history_manager.h>

#define MAX_HISTORY_SIZE 100

void HistoryManager::reset(ProjectState state, bool unsaved) {
  m_history.clear();
  m_history.push_back(state);
  m_current_state = m_history.cbegin();
  m_unsaved = unsaved;
  m_locked = false;
}

void HistoryManager::add_state(ProjectState state, bool unsaved) {
  assert(!m_locked);

  // Erase all states after the current state (the ones left over from undos).
  m_history.erase(m_current_state + 1, m_history.cend());

  // Add the new state.
  m_history.push_back(state);

  // Keep the history size under the limit.
  if (m_history.size() > MAX_HISTORY_SIZE) {
    m_history.pop_front();
  }

  m_current_state = m_history.cend() - 1;

  if (unsaved) m_unsaved = true;
}

void HistoryManager::undo() {
  if (m_locked || m_current_state == m_history.cbegin()) return;

  puts("Undo");

  // Roll back one state.
  m_current_state--;
  m_unsaved = true;
}

void HistoryManager::redo() {
  if (m_locked || m_current_state == m_history.cend() - 1) return;

  puts("Redo");

  // Roll forward one state.
  m_current_state++;
  m_unsaved = true;
}
