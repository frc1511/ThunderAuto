#pragma once

#include <ThunderAuto/HistoryManager.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>
#include <optional>
#include <functional>
#include <unordered_map>

using namespace thunder::core;

class DocumentEditManager final {
  HistoryManager& m_history;
  std::optional<ThunderAutoProjectState> m_currentState = std::nullopt;

 public:
  explicit DocumentEditManager(HistoryManager& history) noexcept : m_history(history) {}

  void startLongEdit() noexcept;
  void discardLongEdit() noexcept;
  void finishLongEdit() noexcept;

  const ThunderAutoProjectState& currentState() const noexcept;

  void addState(const ThunderAutoProjectState& state, bool unsaved = true) noexcept;
  void modifyLastState(const ThunderAutoProjectState& state, bool unsaved = true) noexcept;

  void undo() noexcept;
  void redo() noexcept;

  using StateUpdateCallbackFunc = std::function<void()>;
  using StateUpdateSubscriberID = size_t;

  [[nodiscard]]
  StateUpdateSubscriberID registerStateUpdateSubscriber(StateUpdateCallbackFunc callback) noexcept;
  void unregisterStateUpdateSubscriber(StateUpdateSubscriberID id) noexcept;

 private:
  void notifyStateUpdateSubscribers() noexcept;

 private:
  std::unordered_map<StateUpdateSubscriberID, StateUpdateCallbackFunc> m_stateUpdateSubscribers;
  StateUpdateSubscriberID m_nextSubscriberID = 1;
};
