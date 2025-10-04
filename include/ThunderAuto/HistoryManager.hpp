#pragma once

#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>
#include <deque>

using namespace thunder::core;

class DocumentManager;

class HistoryManager final {
  std::deque<ThunderAutoProjectState> m_history;
  std::deque<ThunderAutoProjectState>::const_iterator m_currentState;

  bool m_unsaved = false;
  bool m_locked = false;

 public:
  void markUnsaved() noexcept { m_unsaved = true; }
  void markSaved() noexcept { m_unsaved = false; }

  bool isUnsaved() const noexcept { return m_unsaved; }

  void reset(ThunderAutoProjectState state, bool unsaved = false) noexcept;

  const ThunderAutoProjectState& currentState() const noexcept { return *m_currentState; }

  void addState(ThunderAutoProjectState state, bool unsaved = true) noexcept;
  void modifyLastState(ThunderAutoProjectState state, bool unsaved = true) noexcept;

  void undo() noexcept;
  void redo() noexcept;

 private:
  friend class DocumentEditManager;

  // Lock undo/redo actions.
  void lock() noexcept { m_locked = true; }
  void unlock() noexcept { m_locked = false; }

  bool isLocked() const noexcept { return m_locked; }
};
