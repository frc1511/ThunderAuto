#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/platform/platform_manager.hpp>
#include <ThunderAuto/popups/new_field_popup.hpp>
#include <ThunderAuto/popups/popup.hpp>
#include <ThunderAuto/project_settings.hpp>

class NewProjectPopup : public Popup {
  PlatformManager& m_platform_manager;

  const char* m_name = "New Project";

  ProjectSettings m_project;

  std::optional<Field> m_field = std::nullopt;

public:
  inline NewProjectPopup(PlatformManager& platform_manager)
    : m_platform_manager(platform_manager) {}

  void present(bool* running) override;
  constexpr const char* name() override { return m_name; }

  enum class Result {
    NONE,
    NEW_FIELD,
    CREATE,
    CANCEL,
  };

  constexpr Result result() const { return m_result; }

  inline void set_field(Field field) { m_field = field; }

  inline ProjectSettings result_project() const { return m_project; }

private:
  Result m_result = Result::NONE;
};
