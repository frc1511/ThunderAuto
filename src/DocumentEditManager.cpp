#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Logger.hpp>

void DocumentEditManager::startLongEdit() noexcept {
  if (m_history.isLocked()) {
    ThunderAutoLogger::Warn("History is already locked");
    return;
  }
  m_history.lock();
  m_currentState = m_history.currentState();
  ThunderAutoLogger::Info("Started long edit");
}

void DocumentEditManager::discardLongEdit() noexcept {
  if (!m_history.isLocked()) {
    ThunderAutoLogger::Warn("History is not locked");
    return;
  }
  m_history.unlock();
  m_currentState = std::nullopt;
  ThunderAutoLogger::Info("Discarded long edit");
}

void DocumentEditManager::finishLongEdit() noexcept {
  if (!m_history.isLocked()) {
    ThunderAutoLogger::Warn("History is not locked");
    return;
  }
  m_history.unlock();
  addState(*m_currentState);
  m_currentState = std::nullopt;
  ThunderAutoLogger::Info("Finished long edit");
}

const ThunderAutoProjectState& DocumentEditManager::currentState() const noexcept {
  if (m_history.isLocked()) {
    return *m_currentState;
  }
  return m_history.currentState();
}

void DocumentEditManager::addState(const ThunderAutoProjectState& state, bool unsaved) noexcept {
  if (m_history.isLocked()) {
    m_currentState = state;
  } else {
    m_history.addState(state, unsaved);
  }
  notifyStateUpdateSubscribers();
}

void DocumentEditManager::modifyLastState(const ThunderAutoProjectState& state, bool unsaved) noexcept {
  if (m_history.isLocked()) {
    m_currentState = state;
    return;
  }
  m_history.modifyLastState(state, unsaved);
}

void DocumentEditManager::undo() noexcept {
  m_history.undo();
  notifyStateUpdateSubscribers();
}

void DocumentEditManager::redo() noexcept {
  m_history.redo();
  notifyStateUpdateSubscribers();
}

DocumentEditManager::StateUpdateSubscriberID DocumentEditManager::registerStateUpdateSubscriber(
    StateUpdateCallbackFunc callback) noexcept {
  StateUpdateSubscriberID id = m_nextSubscriberID++;
  m_stateUpdateSubscribers.emplace(id, std::move(callback));
  return id;
}

void DocumentEditManager::unregisterStateUpdateSubscriber(StateUpdateSubscriberID id) noexcept {
  m_stateUpdateSubscribers.erase(id);
}

void DocumentEditManager::notifyStateUpdateSubscribers() noexcept {
  for (auto& [id, callback] : m_stateUpdateSubscribers) {
    callback();
  }
}
