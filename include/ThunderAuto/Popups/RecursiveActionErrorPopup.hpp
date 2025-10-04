#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <string>
#include <list>

class RecursiveActionErrorPopup : public Popup {
  std::string m_recursionPath;
  std::string m_actionToAddName;
  std::string m_groupActionName;

 public:
  RecursiveActionErrorPopup() = default;

  void present(bool* running) override;

  const char* name() const noexcept override { return "Recursive Action Error"; }

  void setActionRecursionPath(const std::list<std::string>& recursionPath) noexcept;
  void setGroupAction(const std::string& actionName) noexcept;
};

