#pragma once

#include <ThunderAuto/font_library.h>
#include <ThunderAuto/popups/popup.h>

class WelcomePopup : public Popup {
  const char* m_name = "##Welcome!";

  std::list<std::string>& m_recent_projects;
  std::string* m_recent_project = nullptr;

public:
  inline WelcomePopup(std::list<std::string>& recent_projects)
    : m_recent_projects(recent_projects) {}

  void present(bool* running) override;

  constexpr const char* name() override { return m_name; }

  enum class Result {
    NONE,
    NEW_PROJECT,
    OPEN_PROJECT,
    RECENT_PROJECT,
  };

  constexpr Result result() const { return m_result; }
  std::string* recent_project() const {
    return m_recent_project;
  }

private:
  Result m_result = Result::NONE;
};

