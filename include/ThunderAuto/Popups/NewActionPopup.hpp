#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>

using namespace thunder::core;

class NewActionPopup : public Popup {
  DocumentEditManager& m_history;

  char m_actionNameBuffer[256] = "";
  std::string m_newActionName;

  ThunderAutoAction m_actionInfo;

 public:
  explicit NewActionPopup(DocumentEditManager& history) : m_history(history) {}

  const char* name() const noexcept override { return "New Action"; }

  void present(bool* running) override;

  const std::string& newActionName() const noexcept { return m_newActionName; }

  void reset() noexcept {
    m_actionNameBuffer[0] = '\0';
    m_actionInfo = ThunderAutoAction();
  }

 private:
  void presentNameProperty();
  void presentTypeProperty();
  void presentSequentialActionGroup(const ThunderAutoProjectState& state);
  void presentConcurrentActionGroup(const ThunderAutoProjectState& state);
};

