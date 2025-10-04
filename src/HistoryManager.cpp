#include <ThunderAuto/HistoryManager.hpp>

#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/Error.hpp>

static constexpr size_t kMaxHistorySize = 100;

void HistoryManager::reset(ThunderAutoProjectState state, bool unsaved) noexcept {
  m_history.clear();
  m_history.push_back(state);
  m_currentState = m_history.cbegin();
  m_unsaved = unsaved;
  m_locked = false;
}

void HistoryManager::addState(ThunderAutoProjectState state, bool unsaved) noexcept {
  ThunderAutoAssert(!m_locked, "HistoryManager is locked, cannot add state");

  // Erase all states after the current state (the ones left over from undos).
  m_history.erase(m_currentState + 1, m_history.cend());

  // Add the new state.
  m_history.push_back(state);

  // Keep the history size under the limit.
  if (m_history.size() > kMaxHistorySize) {
    m_history.pop_front();
  }

  m_currentState = m_history.cend() - 1;

  if (unsaved) {
    m_unsaved = true;
  }
}

void HistoryManager::modifyLastState(ThunderAutoProjectState state, bool unsaved) noexcept {
  ThunderAutoAssert(!m_locked, "HistoryManager is locked, cannot add state");
  if (m_history.empty()) {
    return;
  }

  m_history.back() = state;

  if (unsaved) {
    m_unsaved = true;
  }
}

void HistoryManager::undo() noexcept {
  if (m_locked || m_currentState == m_history.cbegin())
    return;

  ThunderAutoLogger::Info("Undo");

  // Roll back one state.
  m_currentState--;
  m_unsaved = true;
}

void HistoryManager::redo() noexcept {
  if (m_locked || m_currentState == m_history.cend() - 1)
    return;

  ThunderAutoLogger::Info("Redo");

  // Roll forward one state.
  m_currentState++;
  m_unsaved = true;
}
