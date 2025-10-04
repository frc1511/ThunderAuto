#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>

using namespace thunder::core;

class ProjectVersionPopup : public Popup {
  ThunderAutoProjectVersion m_projectVersion;

 public:
  ProjectVersionPopup() = default;

  void present(bool* running) override;
  const char* name() const noexcept override { return "Project Version Does Not Match"; }

  void setProjectVersion(const ThunderAutoProjectVersion& version) noexcept { m_projectVersion = version; }

  enum class Result {
    NONE,
    OK,
    CANCEL,
  };

  Result result() const noexcept { return m_result; }

 private:
  Result m_result = Result::NONE;
};
