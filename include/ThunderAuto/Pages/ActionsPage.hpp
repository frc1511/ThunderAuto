#pragma once

#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/Page.hpp>

using namespace thunder::core;

class ActionsPage : public Page {
  DocumentEditManager& m_history;

 public:
  explicit ActionsPage(DocumentEditManager& history) : m_history(history) {}

  const char* name() const noexcept override { return "Actions"; }

  void present(bool* running) override;

  enum class Event {
    NONE = 0,
    RENAME_ACTION,
    NEW_ACTION,
    NEW_ACTION_ADD_TO_GROUP,
    INVALID_OPERATION_RECURSIVE_ACTION,
  };

  Event lastPresentEvent() const noexcept { return m_event; }

  /**
   * Returns the name of the action being renamed by RENAME_ACTION or the name of the action group for
   * NEW_ACTION_ADD_TO_GROUP
   */
  const std::string& eventActionName() const noexcept { return m_eventActionName; }

  /**
   * Returns the problematic path from an action back to itself for INVALID_OPERATION_RECURSIVE_ACTION
   */
  const std::list<std::string>& eventActionRecursionPath() const noexcept {
    return m_eventActionRecursionPath;
  }

 private:
  bool verifyAddedGroupAction(const ThunderAutoProjectState& state,
                              const std::string& groupActionName,
                              const std::string& addedActionName);

 private:
  Event m_event = Event::NONE;
  std::string m_eventActionName;
  std::list<std::string> m_eventActionRecursionPath;
};

